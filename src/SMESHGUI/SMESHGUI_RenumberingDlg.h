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
//  File   : SMESHGUI_RenumberingDlg.h
//  Author : Michael ZORIN
//  Module : SMESH
//  $Header:

#ifndef DIALOGBOX_RENUMBERING_H
#define DIALOGBOX_RENUMBERING_H

#include "SALOME_Selection.h"
#include "SMESH_TypeFilter.hxx"

// QT Includes
#include <qdialog.h>

// Open CASCADE Includes

class QGridLayout; 
class QButtonGroup;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class SMESHGUI;

// IDL Headers
#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(SMESH_Mesh)

//=================================================================================
// class    : SMESHGUI_RenumberingDlg
// purpose  : If the unit == 0 nodes will be renumbered, if the unit == 1 the elements will.
//=================================================================================
class SMESHGUI_RenumberingDlg : public QDialog
{ 
    Q_OBJECT

public:
    SMESHGUI_RenumberingDlg( QWidget* parent = 0, const char* name = 0, SALOME_Selection* Sel = 0, const int unit = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SMESHGUI_RenumberingDlg();

private:

    void Init( SALOME_Selection* Sel ) ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;                          /* mouse enter the QWidget */
    void hideEvent ( QHideEvent * );                        /* ESC key */
    
    SMESHGUI*                     mySMESHGUI ;              /* Current SMESHGUI object */
    SALOME_Selection*             mySelection ;             /* User shape selection */
    int                           myConstructorId ;         /* Current constructor id = radio button id */
    QLineEdit*                    myEditCurrentArgument;    /* Current  LineEdit */

    int myUnit;    
    SMESH::SMESH_Mesh_var         myMesh;
    Handle(SMESH_TypeFilter)      myMeshFilter;
        
    QButtonGroup* GroupConstructors;
    QRadioButton* Constructor1;
    QGroupBox* GroupButtons;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QPushButton* buttonApply;
    QGroupBox* GroupMesh;
    QLabel* TextLabelMesh;
    QPushButton* SelectButton;
    QLineEdit* LineEditMesh;

private slots:

    void ConstructorsClicked(int constructorId);
    void ClickOnOk();
    void ClickOnCancel();
    void ClickOnApply();
    void SetEditCurrentArgument() ;
    void SelectionIntoArgument() ;
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
    
protected:
    QGridLayout* SMESHGUI_RenumberingDlgLayout;
    QGridLayout* GroupConstructorsLayout;
    QGridLayout* GroupButtonsLayout;
    QGridLayout* GroupMeshLayout;
};

#endif // DIALOGBOX_RENUMBERING_H