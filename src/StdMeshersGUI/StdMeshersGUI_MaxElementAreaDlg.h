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
//  File   : StdMeshersGUI_MaxElementAreaDlg.h
//           Moved here from SMESHGUI_MaxElementAreaDlg.h
//  Author : Nicolas REJNERI
//  Module : SMESH
//  $Header$

#ifndef DIALOGBOX_MAX_ELEMENT_AREA_H
#define DIALOGBOX_MAX_ELEMENT_AREA_H

// QT Includes
#include <qdialog.h>

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class SMESHGUI;
class SMESHGUI_SpinBox;

//=================================================================================
// class    : StdMeshersGUI_MaxElementAreaDlg
// purpose  :
//=================================================================================
class StdMeshersGUI_MaxElementAreaDlg : public QDialog
{ 
    Q_OBJECT

public:
    StdMeshersGUI_MaxElementAreaDlg (const QString& hypType,
				     QWidget* parent = 0,
				     const char* name = 0,
				     bool modal = FALSE,
				     WFlags fl = 0 );
    ~StdMeshersGUI_MaxElementAreaDlg ();

private:

    void Init() ;
    void closeEvent( QCloseEvent* e ) ;
    void enterEvent ( QEvent * ) ;

    SMESHGUI*  mySMESHGUI ;
    QString    myHypType ;

    QLabel*           iconLabel;
    QLabel*           typeLabel;
    QGroupBox*        GroupC1;
    QLabel*           TextLabel_NameHypothesis ;
    QLineEdit*        LineEdit_NameHypothesis ;
    QLabel*           TextLabel_MaxElementArea ;
    SMESHGUI_SpinBox* SpinBox_MaxElementArea ;
    QGroupBox*        GroupButtons;
    QPushButton*      buttonApply;
    QPushButton*      buttonOk;
    QPushButton*      buttonCancel;

private slots:

    void ClickOnOk();
    void ClickOnCancel();
    bool ClickOnApply();
    void DeactivateActiveDialog() ;
    void ActivateThisDialog() ;
};

#endif // DIALOGBOX_MAX_ELEMENT_AREA_H