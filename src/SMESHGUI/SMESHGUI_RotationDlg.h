//  SMESH SMESHGUI : GUI for SMESH component
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
//
//  File   : SMESHGUI_RotationDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header: 

#ifndef DIALOGBOX_ROTATION_H
#define DIALOGBOX_ROTATION_H

#include "SALOME_Selection.h"
#include "SMESH_LogicalFilter.hxx"

// QT Includes
#include <qdialog.h>

class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QCheckBox;
class SMESHGUI_SpinBox;
class SMESHGUI;
class SMESH_Actor;
class SMESHGUI_SpinBox;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)


//=================================================================================
// class    : SMESHGUI_RotationDlg
// purpose  :
//=================================================================================
class SMESHGUI_RotationDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_RotationDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_RotationDlg();

private:

    void Init( bool ResetControls = true ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                          /* mouse enter the QWidget */
    void hideEvent ( QHideEvent * );                        /* ESC key */
    bool IsAxisOk();
    
    SMESHGUI*                     mySMESHGUI ;              /* Current SMESHGUI object */
    SALOME_Selection*             mySelection ;             /* User shape selection */
    int                           myNbOkElements ;          /* to check when elements are defined */
    QString                       myElementsId;
    
    QWidget*                      myEditCurrentArgument;    /* Current  LineEdit */
    int myConstructorId;
    bool                          myBusy;
    SMESH::SMESH_Mesh_var         myMesh;
    SMESH_Actor*                  myActor;
    Handle(SMESH_LogicalFilter)   myMeshOrSubMeshOrGroupFilter;
    
    QButtonGroup* GroupConstructors;
    QRadioButton* RadioButton1;
    QGroupBox* GroupButtons;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;
    QGroupBox* GroupArguments;
    QGroupBox* GroupAxis;
    QLabel* TextLabelElements;
    QPushButton* SelectElementsButton;
    QLineEdit* LineEditElements;
    QCheckBox* CheckBoxMesh;

    QLabel* TextLabelPoint;
    QPushButton* SelectPointButton;
    QLabel* TextLabelX;
    SMESHGUI_SpinBox* SpinBox_X;
    QLabel* TextLabelY;
    SMESHGUI_SpinBox* SpinBox_Y;
    QLabel* TextLabelZ;
    SMESHGUI_SpinBox* SpinBox_Z;
    QLabel* TextLabelVector;
    QPushButton* SelectVectorButton;
    QLabel* TextLabelDX;
    SMESHGUI_SpinBox* SpinBox_DX;
    QLabel* TextLabelDY;
    SMESHGUI_SpinBox* SpinBox_DY;
    QLabel* TextLabelDZ;
    SMESHGUI_SpinBox* SpinBox_DZ;

    QLabel* TextLabelAngle;
    SMESHGUI_SpinBox* SpinBox_Angle;
    QCheckBox* CheckBoxCopy;
   
    private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    void onTextChange(const QString&);
    void onSelectMesh(bool toSelectMesh);
    void onVectorChanged();

protected:
    QGridLayout* SMESHGUI_RotationDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupArgumentsLayout;
};

#endif // DIALOGBOX_ROTATION_H