//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
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
// File      : SMESH_Pattern_i.cxx
// Created   : Fri Aug 20 16:15:49 2004
// Author    : Edward AGAPOV (eap)
//  $Header: 

#include "SMESH_Pattern_i.hxx"

#include "GEOM_Client.hxx"
#include "SMESH_Gen_i.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMDS_MeshFace.hxx"
#include "SMDS_MeshVolume.hxx"

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>

#include <sstream>
#include <set>

// =============================================================================
//=============================================================================
/*!
// =============================================================================
 *  SMESH_Gen_i::GetPattern
 *
 *  Create pattern mapper
 */
//=============================================================================

SMESH::SMESH_Pattern_ptr SMESH_Gen_i::GetPattern()
{
  SMESH_Pattern_i* i = new SMESH_Pattern_i( this );
  SMESH::SMESH_Pattern_var anObj = i->_this();
  return anObj._retn();
}

//=======================================================================
//function : SMESH_Pattern_i
//purpose  : 
//=======================================================================

SMESH_Pattern_i::SMESH_Pattern_i( SMESH_Gen_i* theGen_i ):
       myGen( theGen_i )
{
}

//=======================================================================
//function : getMesh
//purpose  : 
//=======================================================================

::SMESH_Mesh* SMESH_Pattern_i::getMesh( SMESH::SMESH_Mesh_ptr & theMesh )
{
  SMESH_Mesh_i* anImplPtr = 
    dynamic_cast<SMESH_Mesh_i*>( SMESH_Gen_i::GetServant( theMesh ).in() );
  if ( anImplPtr )
    return & anImplPtr->GetImpl();

  return 0;
}

//=======================================================================
//function : LoadFromFile
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::LoadFromFile(const char* theFileContents)
{
  return myPattern.Load( theFileContents );
}

//=======================================================================
//function : LoadFromFace
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::LoadFromFace(SMESH::SMESH_Mesh_ptr theMesh,
                                             GEOM::GEOM_Object_ptr theFace,
                                             CORBA::Boolean        theProject)
{
  if ( theMesh->_is_nil() || theFace->_is_nil() )
    return false;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return false;

  TopoDS_Face aFace = TopoDS::Face( myGen->GeomObjectToShape( theFace ));
  if ( aFace.IsNull() )
    return false;

  return myPattern.Load( aMesh, aFace, theProject );
}

//=======================================================================
//function : LoadFrom3DBlock
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::LoadFrom3DBlock(SMESH::SMESH_Mesh_ptr theMesh,
                                                GEOM::GEOM_Object_ptr theBlock)
{
  if ( theMesh->_is_nil() || theBlock->_is_nil() )
    return false;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return false;

  TopoDS_Shape aShape = myGen->GeomObjectToShape( theBlock );
  if ( aShape.IsNull())
    return false;

  TopExp_Explorer exp ( aShape, TopAbs_SHELL );
  if ( !exp.More() )
    return false;

  return myPattern.Load( aMesh, TopoDS::Shell( exp.Current() ));
}

//=======================================================================
//function : ApplyToFace
//purpose  : 
//=======================================================================

SMESH::point_array* SMESH_Pattern_i::ApplyToFace(GEOM::GEOM_Object_ptr theFace,
                                                 GEOM::GEOM_Object_ptr theVertexOnKeyPoint1,
                                                 CORBA::Boolean        theReverse)
{
  SMESH::point_array_var points = new SMESH::point_array;
  list<const gp_XYZ *> xyzList;

  TopoDS_Shape F = myGen->GeomObjectToShape( theFace );
  TopoDS_Shape V = myGen->GeomObjectToShape( theVertexOnKeyPoint1 );

  if (!F.IsNull() && F.ShapeType() == TopAbs_FACE &&
      !V.IsNull() && V.ShapeType() == TopAbs_VERTEX
      &&
      myPattern.Apply( TopoDS::Face( F ), TopoDS::Vertex( V ), theReverse ) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  return points._retn();
}

//=======================================================================
//function : ApplyTo3DBlock
//purpose  : 
//=======================================================================

SMESH::point_array* SMESH_Pattern_i::ApplyTo3DBlock(GEOM::GEOM_Object_ptr theBlock,
                                                    GEOM::GEOM_Object_ptr theVertex000,
                                                    GEOM::GEOM_Object_ptr theVertex001)
{
  SMESH::point_array_var points = new SMESH::point_array;
  list<const gp_XYZ *> xyzList;

  TopExp_Explorer exp( myGen->GeomObjectToShape( theBlock ), TopAbs_SHELL );
  TopoDS_Shape V000 = myGen->GeomObjectToShape( theVertex000 );
  TopoDS_Shape V001 = myGen->GeomObjectToShape( theVertex001 );

  if (exp.More() &&
      !V000.IsNull() && V000.ShapeType() == TopAbs_VERTEX &&
      !V001.IsNull() && V001.ShapeType() == TopAbs_VERTEX 
      &&
      myPattern.Apply(TopoDS::Shell( exp.Current() ),
                      TopoDS::Vertex( V000 ),
                      TopoDS::Vertex( V001 )) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  return points._retn();
}

//=======================================================================
//function : ApplyToMeshFaces
//purpose  : 
//=======================================================================

SMESH::point_array*
  SMESH_Pattern_i::ApplyToMeshFaces(SMESH::SMESH_Mesh_ptr    theMesh,
                                    const SMESH::long_array& theFacesIDs,
                                    CORBA::Long              theNodeIndexOnKeyPoint1,
                                    CORBA::Boolean           theReverse)
{
  SMESH::point_array_var points = new SMESH::point_array;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return points._retn();

  list<const gp_XYZ *> xyzList;
  set<const SMDS_MeshFace*> fset;
  for (int i = 0; i < theFacesIDs.length(); i++)
  {
    CORBA::Long index = theFacesIDs[i];
    const SMDS_MeshElement * elem = aMesh->GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Face )
      fset.insert( static_cast<const SMDS_MeshFace *>( elem ));
  }
  if (myPattern.Apply( fset, theNodeIndexOnKeyPoint1, theReverse ) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  return points._retn();
}

//=======================================================================
//function : ApplyToHexahedrons
//purpose  : 
//=======================================================================

SMESH::point_array*
  SMESH_Pattern_i::ApplyToHexahedrons(SMESH::SMESH_Mesh_ptr    theMesh,
                                      const SMESH::long_array& theVolumesIDs,
                                      CORBA::Long              theNode000Index,
                                      CORBA::Long              theNode001Index)
{
  SMESH::point_array_var points = new SMESH::point_array;

  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return points._retn();

  list<const gp_XYZ *> xyzList;
  set<const SMDS_MeshVolume*> vset;
  for (int i = 0; i < theVolumesIDs.length(); i++)
  {
    CORBA::Long index = theVolumesIDs[i];
    const SMDS_MeshElement * elem = aMesh->GetMeshDS()->FindElement(index);
    if ( elem && elem->GetType() == SMDSAbs_Volume && elem->NbNodes() == 8 )
      vset.insert( static_cast<const SMDS_MeshVolume *>( elem ));
  }
  if (myPattern.Apply( vset, theNode000Index, theNode001Index ) &&
      myPattern.GetMappedPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  return points._retn();
}

//=======================================================================
//function : MakeMesh
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::MakeMesh(SMESH::SMESH_Mesh_ptr theMesh)
{
  ::SMESH_Mesh* aMesh = getMesh( theMesh );
  if ( !aMesh )
    return false;

  return myPattern.MakeMesh( aMesh );
}

//=======================================================================
//function : GetString
//purpose  : 
//=======================================================================

char* SMESH_Pattern_i::GetString()
{
  ostringstream os;
  myPattern.Save( os );
  
  return CORBA::string_dup( os.str().c_str() );
}

//=======================================================================
//function : Is2D
//purpose  : 
//=======================================================================

CORBA::Boolean SMESH_Pattern_i::Is2D()
{
  return myPattern.Is2D();
}

//=======================================================================
//function : GetPoints
//purpose  : 
//=======================================================================

SMESH::point_array* SMESH_Pattern_i::GetPoints()
{
  SMESH::point_array_var points = new SMESH::point_array;
  list<const gp_XYZ *> xyzList;

  if (myPattern.GetPoints( xyzList ))
  {
    points->length( xyzList.size() );
    list<const gp_XYZ *>::iterator xyzIt = xyzList.begin();
    for ( int i = 0; xyzIt != xyzList.end(); xyzIt++ ) {
      SMESH::PointStruct & p = points[ i++ ];
      (*xyzIt)->Coord( p.x, p.y, p.z );
    }
  }

  return points._retn();
}

//=======================================================================
//function : GetKeyPoints
//purpose  : 
//=======================================================================

SMESH::long_array* SMESH_Pattern_i::GetKeyPoints()
{
  SMESH::long_array_var ids = new SMESH::long_array;
  if ( myPattern.IsLoaded() ) {
    const list< int > & idList = myPattern.GetKeyPointIDs();
    ids->length( idList.size() );
    list< int >::const_iterator iIt = idList.begin();
    for ( int i = 0; iIt != idList.end(); iIt++, i++ )
      ids[ i ] = *iIt;
  }
  return ids._retn();
}

//=======================================================================
//function : GetElementPoints
//purpose  : 
//=======================================================================

SMESH::array_of_long_array* SMESH_Pattern_i::GetElementPoints(CORBA::Boolean applied)
{
  SMESH::array_of_long_array_var arrayOfArray = new SMESH::array_of_long_array;

  const list< list< int > >& listOfIdList = myPattern.GetElementPointIDs(applied);
  arrayOfArray->length( listOfIdList.size() );
  list< list< int > >::const_iterator llIt = listOfIdList.begin();
  for ( int i = 0 ; llIt != listOfIdList.end(); llIt++, i++ )
  {
    const list< int > & idList = (*llIt);
    SMESH::long_array& ids = arrayOfArray[ i ];
    ids.length( idList.size() );
    list< int >::const_iterator iIt = idList.begin();
    for ( int j = 0; iIt != idList.end(); iIt++, j++ )
      ids[ j ] = *iIt;
  }
  return arrayOfArray._retn();
}

//=======================================================================
//function : GetErrorCode
//purpose  : 
//=======================================================================

#define RETCASE(enm) case ::SMESH_Pattern::enm: return SMESH::SMESH_Pattern::enm;

SMESH::SMESH_Pattern::ErrorCode SMESH_Pattern_i::GetErrorCode()
{
  switch ( myPattern.GetErrorCode() ) {
    RETCASE( ERR_OK );
    RETCASE( ERR_READ_NB_POINTS );
    RETCASE( ERR_READ_POINT_COORDS );
    RETCASE( ERR_READ_TOO_FEW_POINTS );
    RETCASE( ERR_READ_3D_COORD );
    RETCASE( ERR_READ_NO_KEYPOINT );
    RETCASE( ERR_READ_BAD_INDEX );
    RETCASE( ERR_READ_ELEM_POINTS );
    RETCASE( ERR_READ_NO_ELEMS );
    RETCASE( ERR_READ_BAD_KEY_POINT );
    RETCASE( ERR_SAVE_NOT_LOADED );
    RETCASE( ERR_LOAD_EMPTY_SUBMESH );
    RETCASE( ERR_LOADF_NARROW_FACE );
    RETCASE( ERR_LOADF_CLOSED_FACE );
    RETCASE( ERR_LOADV_BAD_SHAPE );
    RETCASE( ERR_LOADV_COMPUTE_PARAMS );
    RETCASE( ERR_APPL_NOT_LOADED );
    RETCASE( ERR_APPL_BAD_DIMENTION );
    RETCASE( ERR_APPL_BAD_NB_VERTICES );
    RETCASE( ERR_APPLF_BAD_TOPOLOGY );
    RETCASE( ERR_APPLF_BAD_VERTEX );
    RETCASE( ERR_APPLF_INTERNAL_EEROR );
    RETCASE( ERR_APPLV_BAD_SHAPE );
    RETCASE( ERR_MAKEM_NOT_COMPUTED );
  default:;
  };
  return SMESH::SMESH_Pattern::ERR_OK;
}
