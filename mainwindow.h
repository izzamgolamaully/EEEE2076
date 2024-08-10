#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>


#define USE_GUI_RENDERER

#include "ModelPartList.h"
#include "VRRenderThread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void updateRender();
    void updateRenderFromTree(const QModelIndex& index);
    void loadStlFile(const QString& fileName);
    void addActorsToVR();
    void addActorsToVR_recursive(const QModelIndex& index);
    void VRActorsFromTree(const QModelIndex& index);
    void resetCamera();  // Reset camera to fit actors within view

public slots:
    void editSelectedItem();
    void addNewItem();
    void on_actionOpen_File_triggered();
    void onStartVR();
    void onStopVR();

signals:
    void statusUpdateMessage(const QString& message, int timeout);

private:
    Ui::MainWindow* ui;
    ModelPartList* partList;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkOpenVRRenderWindow> vrRenderWindow;
    vtkSmartPointer<vtkOpenVRRenderer> vrRenderer;
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor> vrInteractor;
    VRRenderThread* renderThread;
};

#endif // MAINWINDOW_H
