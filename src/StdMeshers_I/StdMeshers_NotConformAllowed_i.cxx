//  SMESH StdMeshers_I : idl implementation based on 'SMESH' unit's classes
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
//  File   : StdMeshers_NotConformAllowed_i.cxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

using namespace std;
using namespace std;
#include "StdMeshers_NotConformAllowed_i.hxx"
#include "SMESH_Gen.hxx"

#include "utilities.h"

//=============================================================================
/*!
 *  Constructor: 
 *  _name is related to the class name: prefix = SMESH_ ; suffix = _i .
 */
//=============================================================================

StdMeshers_NotConformAllowed_i::StdMeshers_NotConformAllowed_i
                                (PortableServer::POA_ptr thePOA,
                                 int                     studyId,
                                 ::SMESH_Gen*            genImpl)
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA )
{
  MESSAGE("StdMeshers_NotConformAllowed_i::StdMeshers_NotConformAllowed_i");
  myBaseImpl = new ::StdMeshers_NotConformAllowed(genImpl->GetANewId(),
                                                  studyId,
                                                  genImpl);
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

StdMeshers_NotConformAllowed_i::~StdMeshers_NotConformAllowed_i()
{
}