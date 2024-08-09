/**     @file ModelPart.cpp
  *
  *     EEEE2076 - Software Engineering & VR Project
  *
  *     Template for model parts that will be added as treeview items
  *
  *     P Evans 2022
  */

#include "ModelPart.h"


/* Commented out for now, will be uncommented later when you have
 * installed the VTK library
 */
#include <vtkSmartPointer.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>



ModelPart::ModelPart(const QList<QVariant>& data, ModelPart* parent )
    : m_itemData(data), m_parentItem(parent) {
    /* You probably want to give the item a default colour */
    file = nullptr;
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    actor = vtkSmartPointer<vtkActor>::New();
    actor->VisibilityOn();      // Do the same with this, ModelPart should be default visible.
}

/**
 * @brief Destructor for the ModelPart class.
 *
 * Deletes all child items of the current model part.
 */
ModelPart::~ModelPart() {
    qDeleteAll(m_childItems);
}


void ModelPart::appendChild( ModelPart* item ) {
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}


ModelPart* ModelPart::child( int row ) {
    /* Return pointer to child item in row below this item.
     */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const {
    /* Count number of child items
     */
    return m_childItems.count();
}


int ModelPart::columnCount() const {
    /* Count number of columns (properties) that this item has.
     */
    return m_itemData.count();
}

QVariant ModelPart::data(int column) const {
    /* Return the data associated with a column of this item 
     *  Note on the QVariant type - it is a generic placeholder type
     *  that can take on the type of most Qt classes. It allows each 
     *  column or property to store data of an arbitrary type.
     */
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}


void ModelPart::set(int column, const QVariant &value) {
    /* Set the data associated with a column of this item 
     */
    if (column < 0 || column >= m_itemData.size())
        return;

    m_itemData.replace(column, value);
}


ModelPart* ModelPart::parentItem() {
    return m_parentItem;
}


int ModelPart::row() const {
    /* Return the row index of this item, relative to it's parent.
     */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart*>(this));
    return 0;
}

/**
 * @brief Sets the color of the model part.
 *
 * @param Clr The color to set for the model part.
 */
void ModelPart::setColour(QColor Clr) {
    /* This is a placeholder function that will be used in the next worksheet */
    Colour = Clr;
}

/**
 * @brief Sets the visibility of the model part.
 *
 * @param isvisible The visibility status to set for the model part.
 */
void ModelPart::setVisible(bool isVisible) {
    /* This is a placeholder function that will be used in the next worksheet */
    isVisible = isVisible;
    /* As the name suggests ... */
}

/**
 * @brief Loads an STL file for the model part.
 *
 * @param fileName The name of the STL file to load.
 */
void ModelPart::loadSTL( QString fileName ) {
    /* This is a placeholder function that will be used in the next worksheet */
    
    /* 1. Use the vtkSTLReader class to load the STL file 
     *     https://vtk.org/doc/nightly/html/classvtkSTLReader.html
     */
    file = vtkSmartPointer<vtkSTLReader>::New();
    file->SetFileName(fileName.toStdString().c_str());
    file->Update();

    /* 2. Initialise the part's vtkMapper */
    if (file->GetOutput() == nullptr) {
        qDebug() << "Failed to load STL file: " << fileName;
        return;
    }
    
    /* 3. Initialise the part's vtkActor and link to the mapper */
    mapper->SetInputConnection(file->GetOutputPort());
    actor->SetMapper(mapper);
}
/**
 * @brief Sets the name of the model part.
 *
 * @param name The name to set for the model part.
 */
void ModelPart::setName(QString name) {
    Name = name;
}

vtkSmartPointer<vtkActor> ModelPart::getActor() {
    /* This is a placeholder function that will be used in the next worksheet */
    
    return actor;
    /* Needs to return a smart pointer to the vtkActor to allow
     * part to be rendered.
     */
}

/**
 * @brief Gets the color of the model part.
 *
 * @return The color of the model part.
 */
QColor ModelPart::get_Color(void) const {
    return Colour;
}

/**
 * @brief Gets the name of the model part.
 *
 * @return The name of the model part.
 */
const QString ModelPart::get_Name(void) {
    return Name;
}

/**
 * @brief Gets the visibility status of the model part.
 *
 * @return The visibility status of the model part.
 */
bool ModelPart::get_Visibility(void) const {
    return isVisible;
}

vtkActor* ModelPart::getNewActor() {
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    pd->DeepCopy(mapper->GetInputDataObject(0, 0));
     
     /* 1. Create new mapper */
    vtkSmartPointer<vtkMapper>vrMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    if (file == nullptr) {
        qDebug() << "ERROR: nothing in file reader";
        return nullptr;
    }
    vrMapper->SetInputDataObject(pd);
    vtkActor* vrActor = vtkActor::New();
    vrActor->SetMapper(vrMapper);
    vrActor->SetProperty(actor->GetProperty());
    /* The new vtkActor pointer must be returned here */
    return vrActor;
    
}

