#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "optiondialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QStatusBar>
#include <QDirIterator>

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkSTLReader.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindowInteractor.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Ensure the render window is correctly associated with the widget
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->widget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Set background color to make the model more visible
    vtkNew<vtkNamedColors> colors;
    renderer->SetBackground(colors->GetColor3d("white").GetData());
    renderWindow->AddRenderer(renderer);

    renderThread = new VRRenderThread();

    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);

    /* Link TreeView to Model */
    this->partList = new ModelPartList("PartsList");
    ui->treeView->setModel(this->partList);

    /* Create a root item in the tree. */
    ModelPart* rootItem = this->partList->getRootItem();

    /* Add 3 top level items */
    for (int i = 0; i < 1; i++) {
        QString name = QString("TopLevel %1").arg(i);
        QString visible("true");

        ModelPart* childItem = new ModelPart({ name, visible });
        childItem->setName(name);

        rootItem->appendChild(childItem);
    }

    // Connect VR start and stop actions
    connect(ui->actionstart_VR, &QAction::triggered, this, &MainWindow::onStartVR);
    connect(ui->actionstop_VR, &QAction::triggered, this, &MainWindow::onStopVR);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_File_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open Files"),
        "C:\\",
        tr("STL Files (*.stl)")
    );

    foreach(QString path, fileNames) {
        QFileInfo fileInfo(path);

        if (fileInfo.isFile()) {
            loadStlFile(path);
        }
    }
    updateRender();
    resetCamera();  // Reset camera to fit the loaded model
}

void MainWindow::loadStlFile(const QString& fileName)
{
    // Check if file exists
    if (!QFile::exists(fileName)) {
        QMessageBox::warning(this, "File Error", "The file does not exist.");
        return;
    }

    // Create a new STL reader
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(fileName.toStdString().c_str());

    // Update the reader to read the file
    reader->Update();

    // Get the polydata from the reader
    vtkSmartPointer<vtkPolyData> polyData = reader->GetOutput();

    // Debug: Check if the polydata has points
    if (polyData->GetNumberOfPoints() == 0) {
        qDebug() << "STL file has no points. Check if the file is valid.";
        QMessageBox::warning(this, "STL Error", "The STL file contains no geometry.");
        return;
    }

    // Create a mapper
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    // Create an actor
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // Optionally set the actor's color
    vtkSmartPointer<vtkNamedColors> colors = vtkSmartPointer<vtkNamedColors>::New();
    actor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());

    // Add the actor to the renderer
    this->renderer->AddActor(actor);

    // Reset the camera to ensure the model is visible
    resetCamera();
}


void MainWindow::resetCamera()
{
    // Reset the camera to fit the actors within the view
    this->renderer->ResetCamera();
    this->renderWindow->Render();
}

void MainWindow::VRActorsFromTree(const QModelIndex& index)
{
    if (index.isValid()) {
        /* Get item at this stage of the tree */
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());

        vtkActor* actor = selectedPart->getActor();
        if (actor != nullptr) {
            // Add the actor to the VR renderer (could be different from the standard renderer)
            this->vrRenderer->AddActor(actor);
        }
    }

    if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
        return;
    }

    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++) {
        VRActorsFromTree(partList->index(i, 0, index));
    }
}

void MainWindow::addNewItem() {
    // Get index of currently selected item
    QModelIndex index = ui->treeView->currentIndex();

    // Add a child
    QModelIndex childIndex = partList->appendChild(index, { QVariant("New"), QVariant("true") });
}

void MainWindow::editSelectedItem() {
    /* Get selected item */
    QModelIndex index = ui->treeView->currentIndex();

    /* Check that something was actually selected before right click ... */
    if (!index.isValid()) {
        return;
    }

    /* Get pointer to selected part */
    ModelPart* part = static_cast<ModelPart*>(index.internalPointer());

    /* Run a dialog to change colour ... */
}

void MainWindow::updateRender()
{
    // Ensure the renderer and render window are properly set up
    if (!this->renderer || !this->renderWindow) {
        qDebug() << "Renderer or Render Window is not set up correctly.";
        return;
    }

    // Remove all items from VTK Renderer
    renderer->RemoveAllViewProps();

    // Update the rendering from the tree model
    updateRenderFromTree(partList->index(0, 0, QModelIndex()));

    // Debug: Check how many actors are in the renderer
    qDebug() << "Number of actors in renderer: " << this->renderer->GetActors()->GetNumberOfItems();

    // Render the scene
    renderer->Render();
    renderWindow->Render();

    // Reset the camera
    resetCamera();
}


void MainWindow::updateRenderFromTree(const QModelIndex& index) {
    if (index.isValid()) {
        /* Get item at this stage of the tree */
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
        selectedPart->set(1, "true");

        // Check if the ModelPart is visible
        if (!selectedPart->get_Visibility()) {
            selectedPart->set(1, "false");
            return;
        }

        vtkActor* actor = selectedPart->getActor();

        // Check if the actor is not null
        if (actor == nullptr) {
            qDebug() << "Failed to get actor from model part";
            return;
        }

        // Get the color from the ModelPart and set it to the actor
        QColor color = selectedPart->get_Color();
        actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

        renderer->AddActor(actor);
    }

    if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
        return;
    }

    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++) {
        updateRenderFromTree(partList->index(i, 0, index));
    }
}

void MainWindow::addActorsToVR() {
    addActorsToVR_recursive(partList->index(0, 0, QModelIndex()));
    renderThread->start();
}

void MainWindow::addActorsToVR_recursive(const QModelIndex& index) {
    if (index.isValid()) {
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());

        vtkActor* actor = selectedPart->getActor();
        if (actor != nullptr) {
            vrRenderer->AddActor(actor);
        }
    }

    if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren)) {
        return;
    }

    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++) {
        addActorsToVR_recursive(partList->index(i, 0, index));
    }
}

void MainWindow::onStartVR() {
    // Initialize VR Renderer, Render Window, and Interactor
    vrRenderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
    vrRenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    vrRenderWindow->AddRenderer(vrRenderer);
    vrInteractor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
    vrInteractor->SetRenderWindow(vrRenderWindow);

    // Add actors from the tree to the VR scene
    addActorsToVR_recursive(partList->index(0, 0, QModelIndex()));

    // Render the VR scene
    vrRenderWindow->Render();
    vrInteractor->Start();

    // Optionally stop VR when the window closes
    vrInteractor->TerminateApp();
}

void MainWindow::onStopVR() {
    if (vrRenderWindow && vrInteractor) {
        vrInteractor->TerminateApp();
    }
}

