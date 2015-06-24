// Copyright (C) 2007-2015  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File      : SMDS_VolumeTool.cxx
// Created   : Tue Jul 13 12:22:13 2004
// Author    : Edward AGAPOV (eap)
//
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "SMDS_VolumeTool.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"
#include "SMDS_VtkVolume.hxx"
#include "SMDS_Mesh.hxx"

#include "utilities.h"

#include <map>
#include <limits>
#include <cmath>

using namespace std;

namespace
{
// ======================================================
// Node indices in faces depending on volume orientation
// making most faces normals external
// ======================================================
// For all elements, 0-th face is bottom based on the first nodes.
// For prismatic elements (tetra,hexa,prisms), 1-th face is a top one.
// For all elements, side faces follow order of bottom nodes
// ======================================================

/*
//           N3
//           +
//          /|\
//         / | \
//        /  |  \
//    N0 +---|---+ N1                TETRAHEDRON
//       \   |   /
//        \  |  /
//         \ | /
//          \|/
//           +
//           N2
*/
static int Tetra_F [4][4] = { // FORWARD == EXTERNAL
  { 0, 1, 2, 0 },              // All faces have external normals
  { 0, 3, 1, 0 },
  { 1, 3, 2, 1 },
  { 0, 2, 3, 0 }}; 
static int Tetra_RE [4][4] = { // REVERSED -> FORWARD (EXTERNAL)
  { 0, 2, 1, 0 },              // All faces have external normals
  { 0, 1, 3, 0 },
  { 1, 2, 3, 1 },
  { 0, 3, 2, 0 }};
static int Tetra_nbN [] = { 3, 3, 3, 3 };

//
//     PYRAMID
//
static int Pyramid_F [5][5] = { // FORWARD == EXTERNAL
  { 0, 1, 2, 3, 0 },            // All faces have external normals
  { 0, 4, 1, 0, 4 },
  { 1, 4, 2, 1, 4 },
  { 2, 4, 3, 2, 4 },
  { 3, 4, 0, 3, 4 }
}; 
static int Pyramid_RE [5][5] = { // REVERSED -> FORWARD (EXTERNAL)
  { 0, 3, 2, 1, 0 },             // All faces but a bottom have external normals
  { 0, 1, 4, 0, 4 },
  { 1, 2, 4, 1, 4 },
  { 2, 3, 4, 2, 4 },
  { 3, 0, 4, 3, 4 }}; 
static int Pyramid_nbN [] = { 4, 3, 3, 3, 3 };

/*   
//            + N4
//           /|\
//          / | \
//         /  |  \
//        /   |   \
//    N3 +---------+ N5
//       |    |    |
//       |    + N1 |
//       |   / \   |                PENTAHEDRON
//       |  /   \  |
//       | /     \ |
//       |/       \|
//    N0 +---------+ N2
*/
static int Penta_F [5][5] = { // FORWARD
  { 0, 1, 2, 0, 0 },          // All faces have external normals
  { 3, 5, 4, 3, 3 },          // 0 is bottom, 1 is top face
  { 0, 3, 4, 1, 0 },
  { 1, 4, 5, 2, 1 },
  { 0, 2, 5, 3, 0 }}; 
static int Penta_RE [5][5] = { // REVERSED -> EXTERNAL
  { 0, 2, 1, 0, 0 },
  { 3, 4, 5, 3, 3 },
  { 0, 1, 4, 3, 0 },
  { 1, 2, 5, 4, 1 },
  { 0, 3, 5, 2, 0 }}; 
static int Penta_nbN [] = { 3, 3, 4, 4, 4 };

/*
//         N5+----------+N6
//          /|         /|
//         / |        / |
//        /  |       /  |
//     N4+----------+N7 |
//       |   |      |   |           HEXAHEDRON
//       | N1+------|---+N2
//       |  /       |  /
//       | /        | /
//       |/         |/
//     N0+----------+N3
*/
static int Hexa_F [6][5] = { // FORWARD
  { 0, 1, 2, 3, 0 },
  { 4, 7, 6, 5, 4 },          // all face normals are external
  { 0, 4, 5, 1, 0 },
  { 1, 5, 6, 2, 1 },
  { 3, 2, 6, 7, 3 }, 
  { 0, 3, 7, 4, 0 }};
static int Hexa_RE [6][5] = { // REVERSED -> EXTERNAL
  { 0, 3, 2, 1, 0 },
  { 4, 5, 6, 7, 4 },          // all face normals are external
  { 0, 1, 5, 4, 0 },
  { 1, 2, 6, 5, 1 },
  { 3, 7, 6, 2, 3 }, 
  { 0, 4, 7, 3, 0 }};
static int Hexa_nbN [] = { 4, 4, 4, 4, 4, 4 };
static int Hexa_oppF[] = { 1, 0, 4, 5, 2, 3 }; // oppopsite facet indices

/*   
//      N8 +------+ N9
//        /        \
//       /          \
//   N7 +            + N10
//       \          /
//        \        /
//      N6 +------+ N11
//                             HEXAGONAL PRISM
//      N2 +------+ N3
//        /        \
//       /          \
//   N1 +            + N4
//       \          /
//        \        /
//      N0 +------+ N5
*/
static int HexPrism_F [8][7] = { // FORWARD
  { 0, 1, 2, 3, 4, 5, 0 },
  { 6,11,10, 9, 8, 7, 6 },
  { 0, 6, 7, 1, 0, 0, 0 },
  { 1, 7, 8, 2, 1, 1, 1 },
  { 2, 8, 9, 3, 2, 2, 2 },
  { 3, 9,10, 4, 3, 3, 3 },
  { 4,10,11, 5, 4, 4, 4 },
  { 5,11, 6, 0, 5, 5, 5 }}; 
static int HexPrism_RE [8][7] = { // REVERSED -> EXTERNAL
  { 0, 5, 4, 3, 2, 1, 0 },
  { 6,11,10, 9, 8, 7, 6 },
  { 0, 6, 7, 1, 0, 0, 0 },
  { 1, 7, 8, 2, 1, 1, 1 },
  { 2, 8, 9, 3, 2, 2, 2 },
  { 3, 9,10, 4, 3, 3, 3 },
  { 4,10,11, 5, 4, 4, 4 },
  { 5,11, 6, 0, 5, 5, 5 }}; 
static int HexPrism_nbN [] = { 6, 6, 4, 4, 4, 4, 4, 4 };


/*
//           N3
//           +
//          /|\
//        7/ | \8
//        /  |4 \                    QUADRATIC
//    N0 +---|---+ N1                TETRAHEDRON
//       \   +9  /
//        \  |  /
//        6\ | /5
//          \|/
//           +
//           N2
*/
static int QuadTetra_F [4][7] = { // FORWARD
  { 0, 4, 1, 5, 2, 6, 0 },        // All faces have external normals
  { 0, 7, 3, 8, 1, 4, 0 },
  { 1, 8, 3, 9, 2, 5, 1 },
  { 0, 6, 2, 9, 3, 7, 0 }}; 
static int QuadTetra_RE [4][7] = { // REVERSED -> FORWARD (EXTERNAL)
  { 0, 6, 2, 5, 1, 4, 0 },         // All faces have external normals
  { 0, 4, 1, 8, 3, 7, 0 },
  { 1, 5, 2, 9, 3, 8, 1 },
  { 0, 7, 3, 9, 2, 6, 0 }};
static int QuadTetra_nbN [] = { 6, 6, 6, 6 };

//
//     QUADRATIC
//     PYRAMID
//
//            +4
//
//            
//       10+-----+11
//         |     |        9 - middle point for (0,4) etc.
//         |     |
//        9+-----+12
//
//            6
//      1+----+----+2
//       |         |
//       |         |
//      5+         +7
//       |         |
//       |         |
//      0+----+----+3
//            8
static int QuadPyram_F [5][9] = {  // FORWARD
  { 0, 5, 1, 6, 2, 7, 3, 8, 0 },   // All faces have external normals
  { 0, 9, 4, 10,1, 5, 0, 4, 4 },
  { 1, 10,4, 11,2, 6, 1, 4, 4 },
  { 2, 11,4, 12,3, 7, 2, 4, 4 },
  { 3, 12,4, 9, 0, 8, 3, 4, 4 }}; 
static int QuadPyram_RE [5][9] = { // REVERSED -> FORWARD (EXTERNAL)
  { 0, 8, 3, 7, 2, 6, 1, 5, 0 },   // All faces but a bottom have external normals
  { 0, 5, 1, 10,4, 9, 0, 4, 4 },
  { 1, 6, 2, 11,4, 10,1, 4, 4 },
  { 2, 7, 3, 12,4, 11,2, 4, 4 },
  { 3, 8, 0, 9, 4, 12,3, 4, 4 }}; 
static int QuadPyram_nbN [] = { 8, 6, 6, 6, 6 };

/*   
//            + N4
//           /|\
//         9/ | \10
//         /  |  \
//        /   |   \
//    N3 +----+----+ N5
//       |    |11  |
//       |    |    |
//       |    +13  |                QUADRATIC
//       |    |    |                PENTAHEDRON
//     12+    |    +14
//       |    |    |
//       |    |    |
//       |    + N1 |
//       |   / \   |               
//       | 6/   \7 |
//       | /     \ |
//       |/       \|
//    N0 +---------+ N2
//            8
*/
static int QuadPenta_F [5][9] = {  // FORWARD
  { 0, 6, 1, 7, 2, 8, 0, 0, 0 },
  { 3, 11,5, 10,4, 9, 3, 3, 3 },
  { 0, 12,3, 9, 4, 13,1, 6, 0 },
  { 1, 13,4, 10,5, 14,2, 7, 1 },
  { 0, 8, 2, 14,5, 11,3, 12,0 }}; 
static int QuadPenta_RE [5][9] = { // REVERSED -> EXTERNAL
  { 0, 8, 2, 7, 1, 6, 0, 0, 0 },
  { 3, 9, 4, 10,5, 11,3, 3, 3 },
  { 0, 6, 1, 13,4, 9, 3, 12,0 },
  { 1, 7, 2, 14,5, 10,4, 13,1 },
  { 0, 12,3, 11,5, 14,2, 8, 0 }}; 
static int QuadPenta_nbN [] = { 6, 6, 8, 8, 8 };

/*
//                 13                                                         
//         N5+-----+-----+N6                          +-----+-----+
//          /|          /|                           /|          /| 
//       12+ |       14+ |                          + |   +25   + |    
//        /  |        /  |                         /  |        /  |    
//     N4+-----+-----+N7 |       QUADRATIC        +-----+-----+   |  Central nodes
//       |   | 15    |   |       HEXAHEDRON       |   |       |   |  of tri-quadratic
//       |   |       |   |                        |   |       |   |  HEXAHEDRON
//       | 17+       |   +18                      |   +   22+ |   +  
//       |   |       |   |                        |21 |       |   | 
//       |   |       |   |                        | + | 26+   | + |    
//       |   |       |   |                        |   |       |23 |    
//     16+   |       +19 |                        +   | +24   +   |    
//       |   |       |   |                        |   |       |   |    
//       |   |     9 |   |                        |   |       |   |    
//       | N1+-----+-|---+N2                      |   +-----+-|---+    
//       |  /        |  /                         |  /        |  /  
//       | +8        | +10                        | +   20+   | +      
//       |/          |/                           |/          |/       
//     N0+-----+-----+N3                          +-----+-----+    
//             11                              
*/
static int QuadHexa_F [6][9] = {  // FORWARD
  { 0, 8, 1, 9, 2, 10,3, 11,0 },   // all face normals are external,
  { 4, 15,7, 14,6, 13,5, 12,4 },
  { 0, 16,4, 12,5, 17,1, 8, 0 },
  { 1, 17,5, 13,6, 18,2, 9, 1 },
  { 3, 10,2, 18,6, 14,7, 19,3 }, 
  { 0, 11,3, 19,7, 15,4, 16,0 }};
static int QuadHexa_RE [6][9] = {  // REVERSED -> EXTERNAL
  { 0, 11,3, 10,2, 9, 1, 8, 0 },   // all face normals are external
  { 4, 12,5, 13,6, 14,7, 15,4 },
  { 0, 8, 1, 17,5, 12,4, 16,0 },
  { 1, 9, 2, 18,6, 13,5, 17,1 },
  { 3, 19,7, 14,6, 18,2, 10,3 }, 
  { 0, 16,4, 15,7, 19,3, 11,0 }};
static int QuadHexa_nbN [] = { 8, 8, 8, 8, 8, 8 };

static int TriQuadHexa_F [6][9] = {  // FORWARD
  { 0, 8, 1, 9, 2, 10,3, 11, 20 },   // all face normals are external
  { 4, 15,7, 14,6, 13,5, 12, 25 },
  { 0, 16,4, 12,5, 17,1, 8,  21 },
  { 1, 17,5, 13,6, 18,2, 9,  22 },
  { 3, 10,2, 18,6, 14,7, 19, 23 }, 
  { 0, 11,3, 19,7, 15,4, 16, 24 }};
static int TriQuadHexa_RE [6][9] = {  // REVERSED -> EXTERNAL
  { 0, 11,3, 10,2, 9, 1, 8,  20 },   // opposite faces are neighbouring,
  { 4, 12,5, 13,6, 14,7, 15, 25 },   // all face normals are external
  { 0, 8, 1, 17,5, 12,4, 16, 21 },
  { 1, 9, 2, 18,6, 13,5, 17, 22 },
  { 3, 19,7, 14,6, 18,2, 10, 23 }, 
  { 0, 16,4, 15,7, 19,3, 11, 24 }};
static int TriQuadHexa_nbN [] = { 9, 9, 9, 9, 9, 9 };


// ========================================================
// to perform some calculations without linkage to CASCADE
// ========================================================
struct XYZ {
  double x;
  double y;
  double z;
  XYZ()                               { x = 0; y = 0; z = 0; }
  XYZ( double X, double Y, double Z ) { x = X; y = Y; z = Z; }
  XYZ( const XYZ& other )             { x = other.x; y = other.y; z = other.z; }
  XYZ( const SMDS_MeshNode* n )       { x = n->X(); y = n->Y(); z = n->Z(); }
  inline XYZ operator-( const XYZ& other );
  inline XYZ operator+( const XYZ& other );
  inline XYZ Crossed( const XYZ& other );
  inline double Dot( const XYZ& other );
  inline double Magnitude();
  inline double SquareMagnitude();
};
inline XYZ XYZ::operator-( const XYZ& Right ) {
  return XYZ(x - Right.x, y - Right.y, z - Right.z);
}
inline XYZ XYZ::operator+( const XYZ& Right ) {
  return XYZ(x + Right.x, y + Right.y, z + Right.z);
}
inline XYZ XYZ::Crossed( const XYZ& Right ) {
  return XYZ (y * Right.z - z * Right.y,
              z * Right.x - x * Right.z,
              x * Right.y - y * Right.x);
}
inline double XYZ::Dot( const XYZ& Other ) {
  return(x * Other.x + y * Other.y + z * Other.z);
}
inline double XYZ::Magnitude() {
  return sqrt (x * x + y * y + z * z);
}
inline double XYZ::SquareMagnitude() {
  return (x * x + y * y + z * z);
}

  //================================================================================
  /*!
   * \brief Return linear type corresponding to a quadratic one
   */
  //================================================================================

  SMDS_VolumeTool::VolumeType quadToLinear(SMDS_VolumeTool::VolumeType quadType)
  {
    SMDS_VolumeTool::VolumeType linType = SMDS_VolumeTool::VolumeType( int(quadType)-4 );
    const int nbCornersByQuad = SMDS_VolumeTool::NbCornerNodes( quadType );
    if ( SMDS_VolumeTool::NbCornerNodes( linType ) == nbCornersByQuad )
      return linType;

    int iLin = 0;
    for ( ; iLin < SMDS_VolumeTool::NB_VOLUME_TYPES; ++iLin )
      if ( SMDS_VolumeTool::NbCornerNodes( SMDS_VolumeTool::VolumeType( iLin )) == nbCornersByQuad)
        return SMDS_VolumeTool::VolumeType( iLin );

    return SMDS_VolumeTool::UNKNOWN;
  }

} // namespace

//=======================================================================
//function : SMDS_VolumeTool
//purpose  : 
//=======================================================================

SMDS_VolumeTool::SMDS_VolumeTool ()
  : myVolumeNodes( NULL ),
    myFaceNodes( NULL )
{
  Set( 0 );
}

//=======================================================================
//function : SMDS_VolumeTool
//purpose  : 
//=======================================================================

SMDS_VolumeTool::SMDS_VolumeTool (const SMDS_MeshElement* theVolume,
                                  const bool              ignoreCentralNodes)
  : myVolumeNodes( NULL ),
    myFaceNodes( NULL )
{
  Set( theVolume, ignoreCentralNodes );
}

//=======================================================================
//function : SMDS_VolumeTool
//purpose  : 
//=======================================================================

SMDS_VolumeTool::~SMDS_VolumeTool()
{
  if ( myVolumeNodes != NULL ) delete [] myVolumeNodes;
  if ( myFaceNodes != NULL   ) delete [] myFaceNodes;

  myFaceNodeIndices = NULL;
  myVolumeNodes = myFaceNodes = NULL;
}

//=======================================================================
//function : SetVolume
//purpose  : Set volume to iterate on
//=======================================================================

bool SMDS_VolumeTool::Set (const SMDS_MeshElement* theVolume,
                           const bool              ignoreCentralNodes)
{
  // reset fields
  myVolume = 0;
  myPolyedre = 0;
  myIgnoreCentralNodes = ignoreCentralNodes;

  myVolForward = true;
  myNbFaces = 0;
  myVolumeNbNodes = 0;
  if (myVolumeNodes != NULL) {
    delete [] myVolumeNodes;
    myVolumeNodes = NULL;
  }
  myPolyIndices.clear();

  myExternalFaces = false;

  myAllFacesNodeIndices_F  = 0;
  //myAllFacesNodeIndices_FE = 0;
  myAllFacesNodeIndices_RE = 0;
  myAllFacesNbNodes        = 0;

  myCurFace = -1;
  myFaceNbNodes = 0;
  myFaceNodeIndices = NULL;
  if (myFaceNodes != NULL) {
    delete [] myFaceNodes;
    myFaceNodes = NULL;
  }

  // set volume data
  if ( !theVolume || theVolume->GetType() != SMDSAbs_Volume )
    return false;

  myVolume = theVolume;
  if (myVolume->IsPoly())
    myPolyedre = dynamic_cast<const SMDS_VtkVolume*>( myVolume );

  myNbFaces = theVolume->NbFaces();
  myVolumeNbNodes = theVolume->NbNodes();

  // set nodes
  int iNode = 0;
  myVolumeNodes = new const SMDS_MeshNode* [myVolumeNbNodes];
  SMDS_ElemIteratorPtr nodeIt = myVolume->nodesIterator();
  while ( nodeIt->more() )
    myVolumeNodes[ iNode++ ] = static_cast<const SMDS_MeshNode*>( nodeIt->next() );

  // check validity
  if ( !setFace(0) )
    return ( myVolume = 0 );

  if ( !myPolyedre )
  {
    // define volume orientation
    XYZ botNormal;
    GetFaceNormal( 0, botNormal.x, botNormal.y, botNormal.z );
    const SMDS_MeshNode* botNode = myVolumeNodes[ 0 ];
    int topNodeIndex = myVolume->NbCornerNodes() - 1;
    while ( !IsLinked( 0, topNodeIndex, /*ignoreMediumNodes=*/true )) --topNodeIndex;
    const SMDS_MeshNode* topNode = myVolumeNodes[ topNodeIndex ];
    XYZ upDir (topNode->X() - botNode->X(),
               topNode->Y() - botNode->Y(),
               topNode->Z() - botNode->Z() );
    myVolForward = ( botNormal.Dot( upDir ) < 0 );

    if ( !myVolForward )
      myCurFace = -1; // previous setFace(0) didn't take myVolForward into account
  }
  return true;
}

//=======================================================================
//function : Inverse
//purpose  : Inverse volume
//=======================================================================

#define SWAP_NODES(nodes,i1,i2)           \
{                                         \
  const SMDS_MeshNode* tmp = nodes[ i1 ]; \
  nodes[ i1 ] = nodes[ i2 ];              \
  nodes[ i2 ] = tmp;                      \
}
void SMDS_VolumeTool::Inverse ()
{
  if ( !myVolume ) return;

  if (myVolume->IsPoly()) {
    MESSAGE("Warning: attempt to inverse polyhedral volume");
    return;
  }

  myVolForward = !myVolForward;
  myCurFace = -1;

  // inverse top and bottom faces
  switch ( myVolumeNbNodes ) {
  case 4:
    SWAP_NODES( myVolumeNodes, 1, 2 );
    break;
  case 5:
    SWAP_NODES( myVolumeNodes, 1, 3 );
    break;
  case 6:
    SWAP_NODES( myVolumeNodes, 1, 2 );
    SWAP_NODES( myVolumeNodes, 4, 5 );
    break;
  case 8:
    SWAP_NODES( myVolumeNodes, 1, 3 );
    SWAP_NODES( myVolumeNodes, 5, 7 );
    break;
  case 12:
    SWAP_NODES( myVolumeNodes, 1, 5 );
    SWAP_NODES( myVolumeNodes, 2, 4 );
    SWAP_NODES( myVolumeNodes, 7, 11 );
    SWAP_NODES( myVolumeNodes, 8, 10 );
    break;

  case 10:
    SWAP_NODES( myVolumeNodes, 1, 2 );
    SWAP_NODES( myVolumeNodes, 4, 6 );
    SWAP_NODES( myVolumeNodes, 8, 9 );
    break;
  case 13:
    SWAP_NODES( myVolumeNodes, 1, 3 );
    SWAP_NODES( myVolumeNodes, 5, 8 );
    SWAP_NODES( myVolumeNodes, 6, 7 );
    SWAP_NODES( myVolumeNodes, 10, 12 );
    break;
  case 15:
    SWAP_NODES( myVolumeNodes, 1, 2 );
    SWAP_NODES( myVolumeNodes, 4, 5 );
    SWAP_NODES( myVolumeNodes, 6, 8 );
    SWAP_NODES( myVolumeNodes, 9, 11 );
    SWAP_NODES( myVolumeNodes, 13, 14 );
    break;
  case 20:
    SWAP_NODES( myVolumeNodes, 1, 3 );
    SWAP_NODES( myVolumeNodes, 5, 7 );
    SWAP_NODES( myVolumeNodes, 8, 11 );
    SWAP_NODES( myVolumeNodes, 9, 10 );
    SWAP_NODES( myVolumeNodes, 12, 15 );
    SWAP_NODES( myVolumeNodes, 13, 14 );
    SWAP_NODES( myVolumeNodes, 17, 19 );
    break;
  case 27:
    SWAP_NODES( myVolumeNodes, 1, 3 );
    SWAP_NODES( myVolumeNodes, 5, 7 );
    SWAP_NODES( myVolumeNodes, 8, 11 );
    SWAP_NODES( myVolumeNodes, 9, 10 );
    SWAP_NODES( myVolumeNodes, 12, 15 );
    SWAP_NODES( myVolumeNodes, 13, 14 );
    SWAP_NODES( myVolumeNodes, 17, 19 );
    SWAP_NODES( myVolumeNodes, 21, 24 );
    SWAP_NODES( myVolumeNodes, 22, 23 );
    break;
  default:;
  }
}

//=======================================================================
//function : GetVolumeType
//purpose  : 
//=======================================================================

SMDS_VolumeTool::VolumeType SMDS_VolumeTool::GetVolumeType() const
{
  if ( myPolyedre )
    return POLYHEDA;

  switch( myVolumeNbNodes ) {
  case 4: return TETRA;
  case 5: return PYRAM;
  case 6: return PENTA;
  case 8: return HEXA;
  case 12: return HEX_PRISM;
  case 10: return QUAD_TETRA;
  case 13: return QUAD_PYRAM;
  case 15: return QUAD_PENTA;
  case 20: return QUAD_HEXA;
  case 27: return QUAD_HEXA;
  default: break;
  }

  return UNKNOWN;
}

//=======================================================================
//function : getTetraVolume
//purpose  : 
//=======================================================================

static double getTetraVolume(const SMDS_MeshNode* n1,
                             const SMDS_MeshNode* n2,
                             const SMDS_MeshNode* n3,
                             const SMDS_MeshNode* n4)
{
  double X1 = n1->X();
  double Y1 = n1->Y();
  double Z1 = n1->Z();

  double X2 = n2->X();
  double Y2 = n2->Y();
  double Z2 = n2->Z();

  double X3 = n3->X();
  double Y3 = n3->Y();
  double Z3 = n3->Z();

  double X4 = n4->X();
  double Y4 = n4->Y();
  double Z4 = n4->Z();

  double Q1 = -(X1-X2)*(Y3*Z4-Y4*Z3);
  double Q2 =  (X1-X3)*(Y2*Z4-Y4*Z2);
  double R1 = -(X1-X4)*(Y2*Z3-Y3*Z2);
  double R2 = -(X2-X3)*(Y1*Z4-Y4*Z1);
  double S1 =  (X2-X4)*(Y1*Z3-Y3*Z1);
  double S2 = -(X3-X4)*(Y1*Z2-Y2*Z1);

  return (Q1+Q2+R1+R2+S1+S2)/6.0;
}

//=======================================================================
//function : GetSize
//purpose  : Return element volume
//=======================================================================

double SMDS_VolumeTool::GetSize() const
{
  double V = 0.;
  if ( !myVolume )
    return 0.;

  if ( myVolume->IsPoly() )
  {
    if ( !myPolyedre )
      return 0.;

    // split a polyhedron into tetrahedrons

    int saveCurFace = myCurFace;
    SMDS_VolumeTool* me = const_cast< SMDS_VolumeTool* > ( this );
    for ( int f = 0; f < NbFaces(); ++f )
    {
      me->setFace( f );
      XYZ area (0,0,0), p1( myFaceNodes[0] );
      for ( int n = 0; n < myFaceNbNodes; ++n )
      {
        XYZ p2( myFaceNodes[ n+1 ]);
        area = area + p1.Crossed( p2 );
        p1 = p2;
      }
      V += p1.Dot( area );
    }
    V /= 6;
    if ( saveCurFace > -1 && saveCurFace != myCurFace )
      me->setFace( myCurFace );
  }
  else 
  {
    const static int ind[] = {
      0, 1, 3, 6, 11, 23, 31, 44, 58, 78 };
    const static int vtab[][4] = { // decomposition into tetra in the order of enum VolumeType
      // tetrahedron
      { 0, 1, 2, 3 },
      // pyramid
      { 0, 1, 3, 4 },
      { 1, 2, 3, 4 },
      // pentahedron
      { 0, 1, 2, 3 },
      { 1, 5, 3, 4 },
      { 1, 5, 2, 3 },
      // hexahedron
      { 1, 4, 3, 0 },
      { 4, 1, 6, 5 },
      { 1, 3, 6, 2 },
      { 4, 6, 3, 7 },
      { 1, 4, 6, 3 },
      // hexagonal prism
      { 0, 1, 2, 7 },
      { 0, 7, 8, 6 },
      { 2, 7, 8, 0 },

      { 0, 3, 4, 9 },
      { 0, 9, 10, 6 },
      { 4, 9, 10, 0 },

      { 0, 3, 4, 9 },
      { 0, 9, 10, 6 },
      { 4, 9, 10, 0 },

      { 0, 4, 5, 10 },
      { 0, 10, 11, 6 },
      { 5, 10, 11, 0 },

      // quadratic tetrahedron
      { 0, 4, 6, 7 },
      { 1, 5, 4, 8 },
      { 2, 6, 5, 9 },
      { 7, 8, 9, 3 },
      { 4, 6, 7, 9 },
      { 4, 5, 6, 9 },
      { 4, 7, 8, 9 },
      { 4, 5, 9, 8 },

      // quadratic pyramid
      { 0, 5, 8, 9 },
      { 1, 5,10, 6 },
      { 2, 6,11, 7 },
      { 3, 7,12, 8 },
      { 4, 9,11,10 },
      { 4, 9,12,11 },
      { 10, 5, 9, 8 },
      { 10, 8, 9,12 },
      { 10, 8,12, 7 },
      { 10, 7,12,11 },
      { 10, 7,11, 6 },
      { 10, 5, 8, 6 },
      { 10, 6, 8, 7 },

      // quadratic pentahedron
      { 12, 0, 8, 6 },
      { 12, 8, 7, 6 },
      { 12, 8, 2, 7 },
      { 12, 6, 7, 1 },
      { 12, 1, 7,13 },
      { 12, 7, 2,13 },
      { 12, 2,14,13 },

      { 12, 3, 9,11 },
      { 12,11, 9,10 },
      { 12,11,10, 5 },
      { 12, 9, 4,10 },
      { 12,14, 5,10 },
      { 12,14,10, 4 },
      { 12,14, 4,13 },

      // quadratic hexahedron
      { 16, 0,11, 8 },
      { 16,11, 9, 8 },
      { 16, 8, 9, 1 },
      { 16,11, 3,10 },
      { 16,11,10, 9 },
      { 16,10, 2, 9 },
      { 16, 3,19, 2 },
      { 16, 2,19,18 },
      { 16, 2,18,17 },
      { 16, 2,17, 1 },

      { 16, 4,12,15 },
      { 16,12, 5,13 },
      { 16,12,13,15 },
      { 16,13, 6,14 },
      { 16,13,14,15 },
      { 16,14, 7,15 },
      { 16, 6, 5,17 },
      { 16,18, 6,17 },
      { 16,18, 7, 6 },
      { 16,18,19, 7 },

    };

    int type = GetVolumeType();
    int n1 = ind[type];
    int n2 = ind[type+1];

    for (int i = n1; i <  n2; i++) {
      V -= getTetraVolume( myVolumeNodes[ vtab[i][0] ],
                           myVolumeNodes[ vtab[i][1] ],
                           myVolumeNodes[ vtab[i][2] ],
                           myVolumeNodes[ vtab[i][3] ]);
    }
  }
  return V;
}

//=======================================================================
//function : GetBaryCenter
//purpose  : 
//=======================================================================

bool SMDS_VolumeTool::GetBaryCenter(double & X, double & Y, double & Z) const
{
  X = Y = Z = 0.;
  if ( !myVolume )
    return false;

  for ( int i = 0; i < myVolumeNbNodes; i++ ) {
    X += myVolumeNodes[ i ]->X();
    Y += myVolumeNodes[ i ]->Y();
    Z += myVolumeNodes[ i ]->Z();
  }
  X /= myVolumeNbNodes;
  Y /= myVolumeNbNodes;
  Z /= myVolumeNbNodes;

  return true;
}

//================================================================================
/*!
 * \brief Classify a point
 *  \param tol - thickness of faces
 */
//================================================================================

bool SMDS_VolumeTool::IsOut(double X, double Y, double Z, double tol) const
{
  // LIMITATION: for convex volumes only
  XYZ p( X,Y,Z );
  for ( int iF = 0; iF < myNbFaces; ++iF )
  {
    XYZ faceNormal;
    if ( !GetFaceNormal( iF, faceNormal.x, faceNormal.y, faceNormal.z ))
      continue;
    if ( !IsFaceExternal( iF ))
      faceNormal = XYZ() - faceNormal; // reverse

    XYZ face2p( p - XYZ( myFaceNodes[0] ));
    if ( face2p.Dot( faceNormal ) > tol )
      return true;
  }
  return false;
}

//=======================================================================
//function : SetExternalNormal
//purpose  : Node order will be so that faces normals are external
//=======================================================================

void SMDS_VolumeTool::SetExternalNormal ()
{
  myExternalFaces = true;
  myCurFace = -1;
}

//=======================================================================
//function : NbFaceNodes
//purpose  : Return number of nodes in the array of face nodes
//=======================================================================

int SMDS_VolumeTool::NbFaceNodes( int faceIndex ) const
{
    if ( !setFace( faceIndex ))
      return 0;
    return myFaceNbNodes;
}

//=======================================================================
//function : GetFaceNodes
//purpose  : Return pointer to the array of face nodes.
//           To comfort link iteration, the array
//           length == NbFaceNodes( faceIndex ) + 1 and
//           the last node == the first one.
//=======================================================================

const SMDS_MeshNode** SMDS_VolumeTool::GetFaceNodes( int faceIndex ) const
{
  if ( !setFace( faceIndex ))
    return 0;
  return myFaceNodes;
}

//=======================================================================
//function : GetFaceNodesIndices
//purpose  : Return pointer to the array of face nodes indices
//           To comfort link iteration, the array
//           length == NbFaceNodes( faceIndex ) + 1 and
//           the last node index == the first one.
//=======================================================================

const int* SMDS_VolumeTool::GetFaceNodesIndices( int faceIndex ) const
{
  if ( !setFace( faceIndex ))
    return 0;

  if (myPolyedre)
  {
    SMDS_VolumeTool* me = const_cast< SMDS_VolumeTool* > ( this );
    me->myPolyIndices.resize( myFaceNbNodes + 1 );
    me->myFaceNodeIndices = & me->myPolyIndices[0];
    for ( int i = 0; i <= myFaceNbNodes; ++i )
      me->myFaceNodeIndices[i] = myVolume->GetNodeIndex( myFaceNodes[i] );
  }
  return myFaceNodeIndices;
}

//=======================================================================
//function : GetFaceNodes
//purpose  : Return a set of face nodes.
//=======================================================================

bool SMDS_VolumeTool::GetFaceNodes (int                        faceIndex,
                                    set<const SMDS_MeshNode*>& theFaceNodes ) const
{
  if ( !setFace( faceIndex ))
    return false;

  theFaceNodes.clear();
  theFaceNodes.insert( myFaceNodes, myFaceNodes + myFaceNbNodes );

  return true;
}

//=======================================================================
//function : IsFaceExternal
//purpose  : Check normal orientation of a given face
//=======================================================================

bool SMDS_VolumeTool::IsFaceExternal( int faceIndex ) const
{
  if ( myExternalFaces || !myVolume )
    return true;

  if (myVolume->IsPoly()) {
    XYZ aNormal, baryCenter, p0 (myPolyedre->GetFaceNode(faceIndex + 1, 1));
    GetFaceNormal(faceIndex, aNormal.x, aNormal.y, aNormal.z);
    GetBaryCenter(baryCenter.x, baryCenter.y, baryCenter.z);
    XYZ insideVec (baryCenter - p0);
    if (insideVec.Dot(aNormal) > 0)
      return false;
    return true;
  }

  // switch ( myVolumeNbNodes ) {
  // case 4:
  // case 5:
  // case 10:
  // case 13:
  //   // only the bottom of a reversed tetrahedron can be internal
  //   return ( myVolForward || faceIndex != 0 );
  // case 6:
  // case 15:
  // case 12:
  //   // in a forward prism, the top is internal, in a reversed one - bottom
  //   return ( myVolForward ? faceIndex != 1 : faceIndex != 0 );
  // case 8:
  // case 20:
  // case 27: {
  //   // in a forward hexahedron, even face normal is external, odd - internal
  //   bool odd = faceIndex % 2;
  //   return ( myVolForward ? !odd : odd );
  // }
  // default:;
  // }
  // return false;
  return true;
}

//=======================================================================
//function : GetFaceNormal
//purpose  : Return a normal to a face
//=======================================================================

bool SMDS_VolumeTool::GetFaceNormal (int faceIndex, double & X, double & Y, double & Z) const
{
  if ( !setFace( faceIndex ))
    return false;

  const int iQuad = ( myFaceNbNodes > 6 && !myPolyedre ) ? 2 : 1;
  XYZ p1 ( myFaceNodes[0*iQuad] );
  XYZ p2 ( myFaceNodes[1*iQuad] );
  XYZ p3 ( myFaceNodes[2*iQuad] );
  XYZ aVec12( p2 - p1 );
  XYZ aVec13( p3 - p1 );
  XYZ cross = aVec12.Crossed( aVec13 );

  if ( myFaceNbNodes >3*iQuad ) {
    XYZ p4 ( myFaceNodes[3*iQuad] );
    XYZ aVec14( p4 - p1 );
    XYZ cross2 = aVec13.Crossed( aVec14 );
    cross = cross + cross2;
  }

  double size = cross.Magnitude();
  if ( size <= numeric_limits<double>::min() )
    return false;

  X = cross.x / size;
  Y = cross.y / size;
  Z = cross.z / size;

  return true;
}

//================================================================================
/*!
 * \brief Return barycenter of a face
 */
//================================================================================

bool SMDS_VolumeTool::GetFaceBaryCenter (int faceIndex, double & X, double & Y, double & Z) const
{
  if ( !setFace( faceIndex ))
    return false;

  X = Y = Z = 0.0;
  for ( int i = 0; i < myFaceNbNodes; ++i )
  {
    X += myFaceNodes[i]->X() / myFaceNbNodes;
    Y += myFaceNodes[i]->Y() / myFaceNbNodes;
    Z += myFaceNodes[i]->Z() / myFaceNbNodes;
  }
  return true;
}

//=======================================================================
//function : GetFaceArea
//purpose  : Return face area
//=======================================================================

double SMDS_VolumeTool::GetFaceArea( int faceIndex ) const
{
  if (myVolume->IsPoly()) {
    MESSAGE("Warning: attempt to obtain area of a face of polyhedral volume");
    return 0;
  }

  if ( !setFace( faceIndex ))
    return 0;

  XYZ p1 ( myFaceNodes[0] );
  XYZ p2 ( myFaceNodes[1] );
  XYZ p3 ( myFaceNodes[2] );
  XYZ aVec12( p2 - p1 );
  XYZ aVec13( p3 - p1 );
  double area = aVec12.Crossed( aVec13 ).Magnitude() * 0.5;

  if ( myFaceNbNodes == 4 ) {
    XYZ p4 ( myFaceNodes[3] );
    XYZ aVec14( p4 - p1 );
    area += aVec14.Crossed( aVec13 ).Magnitude() * 0.5;
  }
  return area;
}

//================================================================================
/*!
 * \brief Return index of the node located at face center of a quadratic element like HEX27
 */
//================================================================================

int SMDS_VolumeTool::GetCenterNodeIndex( int faceIndex ) const
{
  if ( myAllFacesNbNodes && myVolumeNbNodes == 27 ) // classic element with 27 nodes
  {
    switch ( faceIndex ) {
    case 0: return 20;
    case 1: return 25;
    default:
      return faceIndex + 19;
    }
  }
  return -1;
}

//=======================================================================
//function : GetOppFaceIndex
//purpose  : Return index of the opposite face if it exists, else -1.
//=======================================================================

int SMDS_VolumeTool::GetOppFaceIndex( int faceIndex ) const
{
  int ind = -1;
  if (myPolyedre) {
    MESSAGE("Warning: attempt to obtain opposite face on polyhedral volume");
    return ind;
  }

  const int nbHoriFaces = 2;

  if ( faceIndex >= 0 && faceIndex < NbFaces() ) {
    switch ( myVolumeNbNodes ) {
    case 6:
    case 15:
      if ( faceIndex == 0 || faceIndex == 1 )
        ind = 1 - faceIndex;
        break;
    case 8:
    case 12:
      if ( faceIndex <= 1 ) // top or bottom
        ind = 1 - faceIndex;
      else {
        const int nbSideFaces = myAllFacesNbNodes[0];
        ind = ( faceIndex - nbHoriFaces + nbSideFaces/2 ) % nbSideFaces + nbHoriFaces;
      }
      break;
    case 20:
    case 27:
      ind = GetOppFaceIndexOfHex( faceIndex );
      break;
    default:;
    }
  }
  return ind;
}

//=======================================================================
//function : GetOppFaceIndexOfHex
//purpose  : Return index of the opposite face of the hexahedron
//=======================================================================

int SMDS_VolumeTool::GetOppFaceIndexOfHex( int faceIndex )
{
  return Hexa_oppF[ faceIndex ];
}

//=======================================================================
//function : IsLinked
//purpose  : return true if theNode1 is linked with theNode2
// If theIgnoreMediumNodes then corner nodes of quadratic cell are considered linked as well
//=======================================================================

bool SMDS_VolumeTool::IsLinked (const SMDS_MeshNode* theNode1,
                                const SMDS_MeshNode* theNode2,
                                const bool           theIgnoreMediumNodes) const
{
  if ( !myVolume )
    return false;

  if (myVolume->IsPoly()) {
    if (!myPolyedre) {
      MESSAGE("Warning: bad volumic element");
      return false;
    }
    bool isLinked = false;
    int iface;
    for (iface = 1; iface <= myNbFaces && !isLinked; iface++) {
      int inode, nbFaceNodes = myPolyedre->NbFaceNodes(iface);

      for (inode = 1; inode <= nbFaceNodes && !isLinked; inode++) {
        const SMDS_MeshNode* curNode = myPolyedre->GetFaceNode(iface, inode);

        if (curNode == theNode1 || curNode == theNode2) {
          int inextnode = (inode == nbFaceNodes) ? 1 : inode + 1;
          const SMDS_MeshNode* nextNode = myPolyedre->GetFaceNode(iface, inextnode);

          if ((curNode == theNode1 && nextNode == theNode2) ||
              (curNode == theNode2 && nextNode == theNode1)) {
            isLinked = true;
          }
        }
      }
    }
    return isLinked;
  }

  // find nodes indices
  int i1 = -1, i2 = -1, nbFound = 0;
  for ( int i = 0; i < myVolumeNbNodes && nbFound < 2; i++ )
  {
    if ( myVolumeNodes[ i ] == theNode1 )
      i1 = i, ++nbFound;
    else if ( myVolumeNodes[ i ] == theNode2 )
      i2 = i, ++nbFound;
  }
  return IsLinked( i1, i2 );
}

//=======================================================================
//function : IsLinked
//purpose  : return true if the node with theNode1Index is linked
//           with the node with theNode2Index
// If theIgnoreMediumNodes then corner nodes of quadratic cell are considered linked as well
//=======================================================================

bool SMDS_VolumeTool::IsLinked (const int theNode1Index,
                                const int theNode2Index,
                                bool      theIgnoreMediumNodes) const
{
  if ( myVolume->IsPoly() ) {
    return IsLinked(myVolumeNodes[theNode1Index], myVolumeNodes[theNode2Index]);
  }

  int minInd = min( theNode1Index, theNode2Index );
  int maxInd = max( theNode1Index, theNode2Index );

  if ( minInd < 0 || maxInd > myVolumeNbNodes - 1 || maxInd == minInd )
    return false;

  VolumeType type = GetVolumeType();
  if ( myVolume->IsQuadratic() )
  {
    int firstMediumInd = myVolume->NbCornerNodes();
    if ( minInd >= firstMediumInd )
      return false; // both nodes are medium - not linked
    if ( maxInd < firstMediumInd ) // both nodes are corners
    {
      if ( theIgnoreMediumNodes )
        type = quadToLinear(type); // to check linkage of corner nodes only
      else
        return false; // corner nodes are not linked directly in a quadratic cell
    }
  }

  switch ( type ) {
  case TETRA:
    return true;
  case HEXA:
    switch ( maxInd - minInd ) {
    case 1: return minInd != 3;
    case 3: return minInd == 0 || minInd == 4;
    case 4: return true;
    default:;
    }
    break;
  case PYRAM:
    if ( maxInd == 4 )
      return true;
    switch ( maxInd - minInd ) {
    case 1:
    case 3: return true;
    default:;
    }
    break;
  case PENTA:
    switch ( maxInd - minInd ) {
    case 1: return minInd != 2;
    case 2: return minInd == 0 || minInd == 3;
    case 3: return true;
    default:;
    }
    break;
  case QUAD_TETRA:
    {
      switch ( minInd ) {
      case 0: if( maxInd==4 ||  maxInd==6 ||  maxInd==7 ) return true;
      case 1: if( maxInd==4 ||  maxInd==5 ||  maxInd==8 ) return true;
      case 2: if( maxInd==5 ||  maxInd==6 ||  maxInd==9 ) return true;
      case 3: if( maxInd==7 ||  maxInd==8 ||  maxInd==9 ) return true;
      default:;
      }
      break;
    }
  case QUAD_HEXA:
    {
      switch ( minInd ) {
      case 0: if( maxInd==8 ||  maxInd==11 ||  maxInd==16 ) return true;
      case 1: if( maxInd==8 ||  maxInd==9 ||  maxInd==17 ) return true;
      case 2: if( maxInd==9 ||  maxInd==10 ||  maxInd==18 ) return true;
      case 3: if( maxInd==10 ||  maxInd==11 ||  maxInd==19 ) return true;
      case 4: if( maxInd==12 ||  maxInd==15 ||  maxInd==16 ) return true;
      case 5: if( maxInd==12 ||  maxInd==13 ||  maxInd==17 ) return true;
      case 6: if( maxInd==13 ||  maxInd==14 ||  maxInd==18 ) return true;
      case 7: if( maxInd==14 ||  maxInd==15 ||  maxInd==19 ) return true;
      default:;
      }
      break;
    }
  case QUAD_PYRAM:
    {
      switch ( minInd ) {
      case 0: if( maxInd==5 ||  maxInd==8 ||  maxInd==9 ) return true;
      case 1: if( maxInd==5 ||  maxInd==6 ||  maxInd==10 ) return true;
      case 2: if( maxInd==6 ||  maxInd==7 ||  maxInd==11 ) return true;
      case 3: if( maxInd==7 ||  maxInd==8 ||  maxInd==12 ) return true;
      case 4: if( maxInd==9 ||  maxInd==10 ||  maxInd==11 ||  maxInd==12 ) return true;
      default:;
      }
      break;
    }
  case QUAD_PENTA:
    {
      switch ( minInd ) {
      case 0: if( maxInd==6 ||  maxInd==8 ||  maxInd==12 ) return true;
      case 1: if( maxInd==6 ||  maxInd==7 ||  maxInd==13 ) return true;
      case 2: if( maxInd==7 ||  maxInd==8 ||  maxInd==14 ) return true;
      case 3: if( maxInd==9 ||  maxInd==11 ||  maxInd==12 ) return true;
      case 4: if( maxInd==9 ||  maxInd==10 ||  maxInd==13 ) return true;
      case 5: if( maxInd==10 ||  maxInd==11 ||  maxInd==14 ) return true;
      default:;
      }
      break;
    }
  case HEX_PRISM:
    {
      const int diff = maxInd-minInd;
      if ( diff > 6  ) return false;// not linked top and bottom
      if ( diff == 6 ) return true; // linked top and bottom
      return diff == 1 || diff == 7;
    }
  default:;
  }
  return false;
}

//=======================================================================
//function : GetNodeIndex
//purpose  : Return an index of theNode
//=======================================================================

int SMDS_VolumeTool::GetNodeIndex(const SMDS_MeshNode* theNode) const
{
  if ( myVolume ) {
    for ( int i = 0; i < myVolumeNbNodes; i++ ) {
      if ( myVolumeNodes[ i ] == theNode )
        return i;
    }
  }
  return -1;
}

//================================================================================
/*!
 * \brief Fill vector with boundary faces existing in the mesh
  * \param faces - vector of found nodes
  * \retval int - nb of found faces
 */
//================================================================================

int SMDS_VolumeTool::GetAllExistingFaces(vector<const SMDS_MeshElement*> & faces) const
{
  faces.clear();
  for ( int iF = 0; iF < NbFaces(); ++iF ) {
    const SMDS_MeshFace* face = 0;
    const SMDS_MeshNode** nodes = GetFaceNodes( iF );
    switch ( NbFaceNodes( iF )) {
    case 3:
      face = SMDS_Mesh::FindFace( nodes[0], nodes[1], nodes[2] ); break;
    case 4:
      face = SMDS_Mesh::FindFace( nodes[0], nodes[1], nodes[2], nodes[3] ); break;
    case 6:
      face = SMDS_Mesh::FindFace( nodes[0], nodes[1], nodes[2],
                                  nodes[3], nodes[4], nodes[5]); break;
    case 8:
      face = SMDS_Mesh::FindFace( nodes[0], nodes[1], nodes[2], nodes[3],
                                  nodes[4], nodes[5], nodes[6], nodes[7]); break;
    }
    if ( face )
      faces.push_back( face );
  }
  return faces.size();
}


//================================================================================
/*!
 * \brief Fill vector with boundary edges existing in the mesh
  * \param edges - vector of found edges
  * \retval int - nb of found faces
 */
//================================================================================

int SMDS_VolumeTool::GetAllExistingEdges(vector<const SMDS_MeshElement*> & edges) const
{
  edges.clear();
  edges.reserve( myVolumeNbNodes * 2 );
  for ( int i = 0; i < myVolumeNbNodes-1; ++i ) {
    for ( int j = i + 1; j < myVolumeNbNodes; ++j ) {
      if ( IsLinked( i, j )) {
        const SMDS_MeshElement* edge =
          SMDS_Mesh::FindEdge( myVolumeNodes[i], myVolumeNodes[j] );
        if ( edge )
          edges.push_back( edge );
      }
    }
  }
  return edges.size();
}

//================================================================================
/*!
 * \brief Return minimal square distance between connected corner nodes
 */
//================================================================================

double SMDS_VolumeTool::MinLinearSize2() const
{
  double minSize = 1e+100;
  int iQ = myVolume->IsQuadratic() ? 2 : 1;

  // store current face data
  int curFace = myCurFace, nbN = myFaceNbNodes;
  int* ind = myFaceNodeIndices;
  myFaceNodeIndices = NULL;
  const SMDS_MeshNode** nodes = myFaceNodes;
  myFaceNodes = NULL;
  
  // it seems that compute distance twice is faster than organization of a sole computing
  myCurFace = -1;
  for ( int iF = 0; iF < myNbFaces; ++iF )
  {
    setFace( iF );
    for ( int iN = 0; iN < myFaceNbNodes; iN += iQ )
    {
      XYZ n1( myFaceNodes[ iN ]);
      XYZ n2( myFaceNodes[(iN + iQ) % myFaceNbNodes]);
      minSize = std::min( minSize, (n1 - n2).SquareMagnitude());
    }
  }
  // restore current face data
  myCurFace = curFace;
  myFaceNbNodes = nbN;
  myFaceNodeIndices = ind;
  delete [] myFaceNodes; myFaceNodes = nodes;

  return minSize;
}

//================================================================================
/*!
 * \brief Return maximal square distance between connected corner nodes
 */
//================================================================================

double SMDS_VolumeTool::MaxLinearSize2() const
{
  double maxSize = -1e+100;
  int iQ = myVolume->IsQuadratic() ? 2 : 1;

  // store current face data
  int curFace = myCurFace, nbN = myFaceNbNodes;
  int* ind = myFaceNodeIndices;
  myFaceNodeIndices = NULL;
  const SMDS_MeshNode** nodes = myFaceNodes;
  myFaceNodes = NULL;
  
  // it seems that compute distance twice is faster than organization of a sole computing
  myCurFace = -1;
  for ( int iF = 0; iF < myNbFaces; ++iF )
  {
    setFace( iF );
    for ( int iN = 0; iN < myFaceNbNodes; iN += iQ )
    {
      XYZ n1( myFaceNodes[ iN ]);
      XYZ n2( myFaceNodes[(iN + iQ) % myFaceNbNodes]);
      maxSize = std::max( maxSize, (n1 - n2).SquareMagnitude());
    }
  }
  // restore current face data
  myCurFace         = curFace;
  myFaceNbNodes     = nbN;
  myFaceNodeIndices = ind;
  delete [] myFaceNodes; myFaceNodes = nodes;

  return maxSize;
}

//================================================================================
/*!
 * \brief fast check that only one volume is build on the face nodes
 *        This check is valid for conformal meshes only
 */
//================================================================================

bool SMDS_VolumeTool::IsFreeFace( int faceIndex, const SMDS_MeshElement** otherVol/*=0*/ ) const
{
  const bool isFree = true;

  if (!setFace( faceIndex ))
    return !isFree;

  const SMDS_MeshNode** nodes = GetFaceNodes( faceIndex );

  const int  di = myVolume->IsQuadratic() ? 2 : 1;
  const int nbN = ( myFaceNbNodes/di <= 4 && !IsPoly()) ? 3 : myFaceNbNodes/di; // nb nodes to check

  SMDS_ElemIteratorPtr eIt = nodes[0]->GetInverseElementIterator( SMDSAbs_Volume );
  while ( eIt->more() )
  {
    const SMDS_MeshElement* vol = eIt->next();
    if ( vol == myVolume )
      continue;
    int iN;
    for ( iN = 1; iN < nbN; ++iN )
      if ( vol->GetNodeIndex( nodes[ iN*di ]) < 0 )
        break;
    if ( iN == nbN ) // nbN nodes are shared with vol
    {
      // if ( vol->IsPoly() || vol->NbFaces() > 6 ) // vol is polyhed or hex prism 
      // {
      //   int nb = myFaceNbNodes;
      //   if ( myVolume->GetEntityType() != vol->GetEntityType() )
      //     nb -= ( GetCenterNodeIndex(0) > 0 );
      //   set<const SMDS_MeshNode*> faceNodes( nodes, nodes + nb );
      //   if ( SMDS_VolumeTool( vol ).GetFaceIndex( faceNodes ) < 0 )
      //     continue;
      // }
      if ( otherVol ) *otherVol = vol;
      return !isFree;
    }
  }
  if ( otherVol ) *otherVol = 0;
  return isFree;
}

//================================================================================
/*!
 * \brief Thorough check that only one volume is build on the face nodes
 */
//================================================================================

bool SMDS_VolumeTool::IsFreeFaceAdv( int faceIndex, const SMDS_MeshElement** otherVol/*=0*/ ) const
{
  const bool isFree = true;

  if (!setFace( faceIndex ))
    return !isFree;

  const SMDS_MeshNode** nodes = GetFaceNodes( faceIndex );
  const int nbFaceNodes = myFaceNbNodes;

  // evaluate nb of face nodes shared by other volumes
  int maxNbShared = -1;
  typedef map< const SMDS_MeshElement*, int > TElemIntMap;
  TElemIntMap volNbShared;
  TElemIntMap::iterator vNbIt;
  for ( int iNode = 0; iNode < nbFaceNodes; iNode++ ) {
    const SMDS_MeshNode* n = nodes[ iNode ];
    SMDS_ElemIteratorPtr eIt = n->GetInverseElementIterator( SMDSAbs_Volume );
    while ( eIt->more() ) {
      const SMDS_MeshElement* elem = eIt->next();
      if ( elem != myVolume ) {
        vNbIt = volNbShared.insert( make_pair( elem, 0 )).first;
        (*vNbIt).second++;
        if ( vNbIt->second > maxNbShared )
          maxNbShared = vNbIt->second;
      }
    }
  }
  if ( maxNbShared < 3 )
    return isFree; // is free

  // find volumes laying on the opposite side of the face
  // and sharing all nodes
  XYZ intNormal; // internal normal
  GetFaceNormal( faceIndex, intNormal.x, intNormal.y, intNormal.z );
  if ( IsFaceExternal( faceIndex ))
    intNormal = XYZ( -intNormal.x, -intNormal.y, -intNormal.z );
  XYZ p0 ( nodes[0] ), baryCenter;
  for ( vNbIt = volNbShared.begin(); vNbIt != volNbShared.end();  ) {
    const int& nbShared = (*vNbIt).second;
    if ( nbShared >= 3 ) {
      SMDS_VolumeTool volume( (*vNbIt).first );
      volume.GetBaryCenter( baryCenter.x, baryCenter.y, baryCenter.z );
      XYZ intNormal2( baryCenter - p0 );
      if ( intNormal.Dot( intNormal2 ) < 0 ) {
        // opposite side
        if ( nbShared >= nbFaceNodes )
        {
          // a volume shares the whole facet
          if ( otherVol ) *otherVol = vNbIt->first;
          return !isFree; 
        }
        ++vNbIt;
        continue;
      }
    }
    // remove a volume from volNbShared map
    volNbShared.erase( vNbIt++ );
  }

  // here volNbShared contains only volumes laying on the opposite side of
  // the face and sharing 3 or more but not all face nodes with myVolume
  if ( volNbShared.size() < 2 ) {
    return isFree; // is free
  }

  // check if the whole area of a face is shared
  for ( int iNode = 0; iNode < nbFaceNodes; iNode++ )
  {
    const SMDS_MeshNode* n = nodes[ iNode ];
    // check if n is shared by one of volumes of volNbShared
    bool isShared = false;
    SMDS_ElemIteratorPtr eIt = n->GetInverseElementIterator( SMDSAbs_Volume );
    while ( eIt->more() && !isShared )
      isShared = volNbShared.count( eIt->next() );
    if ( !isShared )
      return isFree;
  }
  if ( otherVol ) *otherVol = volNbShared.begin()->first;
  return !isFree;

//   if ( !myVolume->IsPoly() )
//   {
//     bool isShared[] = { false, false, false, false }; // 4 triangle parts of a quadrangle
//     for ( vNbIt = volNbShared.begin(); vNbIt != volNbShared.end(); vNbIt++ ) {
//       SMDS_VolumeTool volume( (*vNbIt).first );
//       bool prevLinkShared = false;
//       int nbSharedLinks = 0;
//       for ( int iNode = 0; iNode < nbFaceNodes; iNode++ ) {
//         bool linkShared = volume.IsLinked( nodes[ iNode ], nodes[ iNode + 1] );
//         if ( linkShared )
//           nbSharedLinks++;
//         if ( linkShared && prevLinkShared &&
//              volume.IsLinked( nodes[ iNode - 1 ], nodes[ iNode + 1] ))
//           isShared[ iNode ] = true;
//         prevLinkShared = linkShared;
//       }
//       if ( nbSharedLinks == nbFaceNodes )
//         return !free; // is not free
//       if ( nbFaceNodes == 4 ) {
//         // check traingle parts 1 & 3
//         if ( isShared[1] && isShared[3] )
//           return !free; // is not free
//         // check triangle parts 0 & 2;
//         // 0 part could not be checked in the loop; check it here
//         if ( isShared[2] && prevLinkShared &&
//              volume.IsLinked( nodes[ 0 ], nodes[ 1 ] ) &&
//              volume.IsLinked( nodes[ 1 ], nodes[ 3 ] ) )
//           return !free; // is not free
//       }
//     }
//   }
//  return free;
}

//=======================================================================
//function : GetFaceIndex
//purpose  : Return index of a face formed by theFaceNodes
//=======================================================================

int SMDS_VolumeTool::GetFaceIndex( const set<const SMDS_MeshNode*>& theFaceNodes,
                                   const int                        theFaceIndexHint ) const
{
  if ( theFaceIndexHint >= 0 )
  {
    int nbNodes = NbFaceNodes( theFaceIndexHint );
    if ( nbNodes == (int) theFaceNodes.size() )
    {
      const SMDS_MeshNode** nodes = GetFaceNodes( theFaceIndexHint );
      while ( nbNodes )
        if ( theFaceNodes.count( nodes[ nbNodes-1 ]))
          --nbNodes;
        else
          break;
      if ( nbNodes == 0 )
        return theFaceIndexHint;
    }
  }
  for ( int iFace = 0; iFace < myNbFaces; iFace++ )
  {
    if ( iFace == theFaceIndexHint )
      continue;
    int nbNodes = NbFaceNodes( iFace );
    if ( nbNodes == (int) theFaceNodes.size() )
    {
      const SMDS_MeshNode** nodes = GetFaceNodes( iFace );
      while ( nbNodes )
        if ( theFaceNodes.count( nodes[ nbNodes-1 ]))
          --nbNodes;
        else
          break;
      if ( nbNodes == 0 )
        return iFace;
    }
  }
  return -1;
}

//=======================================================================
//function : GetFaceIndex
//purpose  : Return index of a face formed by theFaceNodes
//=======================================================================

/*int SMDS_VolumeTool::GetFaceIndex( const set<int>& theFaceNodesIndices )
{
  for ( int iFace = 0; iFace < myNbFaces; iFace++ ) {
    const int* nodes = GetFaceNodesIndices( iFace );
    int nbFaceNodes = NbFaceNodes( iFace );
    set<int> nodeSet;
    for ( int iNode = 0; iNode < nbFaceNodes; iNode++ )
      nodeSet.insert( nodes[ iNode ] );
    if ( theFaceNodesIndices == nodeSet )
      return iFace;
  }
  return -1;
}*/

//=======================================================================
//function : setFace
//purpose  : 
//=======================================================================

bool SMDS_VolumeTool::setFace( int faceIndex ) const
{
  if ( !myVolume )
    return false;

  if ( myCurFace == faceIndex )
    return true;

  myCurFace = -1;

  if ( faceIndex < 0 || faceIndex >= NbFaces() )
    return false;

  if (myFaceNodes != NULL) {
    delete [] myFaceNodes;
    myFaceNodes = NULL;
  }

  if (myVolume->IsPoly())
  {
    if (!myPolyedre) {
      MESSAGE("Warning: bad volumic element");
      return false;
    }

    // set face nodes
    int iNode;
    myFaceNbNodes = myPolyedre->NbFaceNodes(faceIndex + 1);
    myFaceNodes = new const SMDS_MeshNode* [myFaceNbNodes + 1];
    for ( iNode = 0; iNode < myFaceNbNodes; iNode++ )
      myFaceNodes[ iNode ] = myPolyedre->GetFaceNode(faceIndex + 1, iNode + 1);
    myFaceNodes[ myFaceNbNodes ] = myFaceNodes[ 0 ]; // last = first

    // check orientation
    if (myExternalFaces)
    {
      myCurFace = faceIndex; // avoid infinite recursion in IsFaceExternal()
      myExternalFaces = false; // force normal computation by IsFaceExternal()
      if ( !IsFaceExternal( faceIndex ))
        for ( int i = 0, j = myFaceNbNodes; i < j; ++i, --j )
          std::swap( myFaceNodes[i], myFaceNodes[j] );
      myExternalFaces = true;
    }
  }
  else
  {
    if ( !myAllFacesNodeIndices_F )
    {
      // choose data for an element type
      switch ( myVolumeNbNodes ) {
      case 4:
        myAllFacesNodeIndices_F  = &Tetra_F [0][0];
        //myAllFacesNodeIndices_FE = &Tetra_F [0][0];
        myAllFacesNodeIndices_RE = &Tetra_RE[0][0];
        myAllFacesNbNodes        = Tetra_nbN;
        myMaxFaceNbNodes         = sizeof(Tetra_F[0])/sizeof(Tetra_F[0][0]);
        break;
      case 5:
        myAllFacesNodeIndices_F  = &Pyramid_F [0][0];
        //myAllFacesNodeIndices_FE = &Pyramid_F [0][0];
        myAllFacesNodeIndices_RE = &Pyramid_RE[0][0];
        myAllFacesNbNodes        = Pyramid_nbN;
        myMaxFaceNbNodes         = sizeof(Pyramid_F[0])/sizeof(Pyramid_F[0][0]);
        break;
      case 6:
        myAllFacesNodeIndices_F  = &Penta_F [0][0];
        //myAllFacesNodeIndices_FE = &Penta_FE[0][0];
        myAllFacesNodeIndices_RE = &Penta_RE[0][0];
        myAllFacesNbNodes        = Penta_nbN;
        myMaxFaceNbNodes         = sizeof(Penta_F[0])/sizeof(Penta_F[0][0]);
        break;
      case 8:
        myAllFacesNodeIndices_F  = &Hexa_F [0][0];
        ///myAllFacesNodeIndices_FE = &Hexa_FE[0][0];
        myAllFacesNodeIndices_RE = &Hexa_RE[0][0];
        myAllFacesNbNodes        = Hexa_nbN;
        myMaxFaceNbNodes         = sizeof(Hexa_F[0])/sizeof(Hexa_F[0][0]);
        break;
      case 10:
        myAllFacesNodeIndices_F  = &QuadTetra_F [0][0];
        //myAllFacesNodeIndices_FE = &QuadTetra_F [0][0];
        myAllFacesNodeIndices_RE = &QuadTetra_RE[0][0];
        myAllFacesNbNodes        = QuadTetra_nbN;
        myMaxFaceNbNodes         = sizeof(QuadTetra_F[0])/sizeof(QuadTetra_F[0][0]);
        break;
      case 13:
        myAllFacesNodeIndices_F  = &QuadPyram_F [0][0];
        //myAllFacesNodeIndices_FE = &QuadPyram_F [0][0];
        myAllFacesNodeIndices_RE = &QuadPyram_RE[0][0];
        myAllFacesNbNodes        = QuadPyram_nbN;
        myMaxFaceNbNodes         = sizeof(QuadPyram_F[0])/sizeof(QuadPyram_F[0][0]);
        break;
      case 15:
        myAllFacesNodeIndices_F  = &QuadPenta_F [0][0];
        //myAllFacesNodeIndices_FE = &QuadPenta_FE[0][0];
        myAllFacesNodeIndices_RE = &QuadPenta_RE[0][0];
        myAllFacesNbNodes        = QuadPenta_nbN;
        myMaxFaceNbNodes         = sizeof(QuadPenta_F[0])/sizeof(QuadPenta_F[0][0]);
        break;
      case 20:
      case 27:
        myAllFacesNodeIndices_F  = &QuadHexa_F [0][0];
        //myAllFacesNodeIndices_FE = &QuadHexa_FE[0][0];
        myAllFacesNodeIndices_RE = &QuadHexa_RE[0][0];
        myAllFacesNbNodes        = QuadHexa_nbN;
        myMaxFaceNbNodes         = sizeof(QuadHexa_F[0])/sizeof(QuadHexa_F[0][0]);
        if ( !myIgnoreCentralNodes && myVolumeNbNodes == 27 )
        {
          myAllFacesNodeIndices_F  = &TriQuadHexa_F [0][0];
          //myAllFacesNodeIndices_FE = &TriQuadHexa_FE[0][0];
          myAllFacesNodeIndices_RE = &TriQuadHexa_RE[0][0];
          myAllFacesNbNodes        = TriQuadHexa_nbN;
          myMaxFaceNbNodes         = sizeof(TriQuadHexa_F[0])/sizeof(TriQuadHexa_F[0][0]);
        }
        break;
      case 12:
        myAllFacesNodeIndices_F  = &HexPrism_F [0][0];
        //myAllFacesNodeIndices_FE = &HexPrism_FE[0][0];
        myAllFacesNodeIndices_RE = &HexPrism_RE[0][0];
        myAllFacesNbNodes        = HexPrism_nbN;
        myMaxFaceNbNodes         = sizeof(HexPrism_F[0])/sizeof(HexPrism_F[0][0]);
        break;
      default:
        return false;
      }
    }
    myFaceNbNodes = myAllFacesNbNodes[ faceIndex ];
    // if ( myExternalFaces )
    //   myFaceNodeIndices = (int*)( myVolForward ? myAllFacesNodeIndices_FE + faceIndex*myMaxFaceNbNodes : myAllFacesNodeIndices_RE + faceIndex*myMaxFaceNbNodes );
    // else
    //   myFaceNodeIndices = (int*)( myAllFacesNodeIndices_F + faceIndex*myMaxFaceNbNodes );
    myFaceNodeIndices = (int*)( myVolForward ? myAllFacesNodeIndices_F + faceIndex*myMaxFaceNbNodes : myAllFacesNodeIndices_RE + faceIndex*myMaxFaceNbNodes );

    // set face nodes
    myFaceNodes = new const SMDS_MeshNode* [myFaceNbNodes + 1];
    for ( int iNode = 0; iNode < myFaceNbNodes; iNode++ )
      myFaceNodes[ iNode ] = myVolumeNodes[ myFaceNodeIndices[ iNode ]];
    myFaceNodes[ myFaceNbNodes ] = myFaceNodes[ 0 ];
  }

  myCurFace = faceIndex;

  return true;
}

//=======================================================================
//function : GetType
//purpose  : return VolumeType by nb of nodes in a volume
//=======================================================================

SMDS_VolumeTool::VolumeType SMDS_VolumeTool::GetType(int nbNodes)
{
  switch ( nbNodes ) {
  case 4: return TETRA;
  case 5: return PYRAM;
  case 6: return PENTA;
  case 8: return HEXA;
  case 10: return QUAD_TETRA;
  case 13: return QUAD_PYRAM;
  case 15: return QUAD_PENTA;
  case 20:
  case 27: return QUAD_HEXA;
  case 12: return HEX_PRISM;
  default:return UNKNOWN;
  }
}

//=======================================================================
//function : NbFaces
//purpose  : return nb of faces by volume type
//=======================================================================

int SMDS_VolumeTool::NbFaces( VolumeType type )
{
  switch ( type ) {
  case TETRA     :
  case QUAD_TETRA: return 4;
  case PYRAM     :
  case QUAD_PYRAM: return 5;
  case PENTA     :
  case QUAD_PENTA: return 5;
  case HEXA      :
  case QUAD_HEXA : return 6;
  case HEX_PRISM : return 8;
  default:         return 0;
  }
}

//================================================================================
/*!
 * \brief Useful to know nb of corner nodes of a quadratic volume
  * \param type - volume type
  * \retval int - nb of corner nodes
 */
//================================================================================

int SMDS_VolumeTool::NbCornerNodes(VolumeType type)
{
  switch ( type ) {
  case TETRA     :
  case QUAD_TETRA: return 4;
  case PYRAM     :
  case QUAD_PYRAM: return 5;
  case PENTA     :
  case QUAD_PENTA: return 6;
  case HEXA      :
  case QUAD_HEXA : return 8;
  case HEX_PRISM : return 12;
  default:         return 0;
  }
  return 0;
}
  // 

//=======================================================================
//function : GetFaceNodesIndices
//purpose  : Return the array of face nodes indices
//           To comfort link iteration, the array
//           length == NbFaceNodes( faceIndex ) + 1 and
//           the last node index == the first one.
//=======================================================================

const int* SMDS_VolumeTool::GetFaceNodesIndices(VolumeType type,
                                                int        faceIndex,
                                                bool       external)
{
  switch ( type ) {
  case TETRA: return Tetra_F[ faceIndex ];
  case PYRAM: return Pyramid_F[ faceIndex ];
  case PENTA: return external ? Penta_F[ faceIndex ] : Penta_F[ faceIndex ];
  case HEXA:  return external ? Hexa_F[ faceIndex ] : Hexa_F[ faceIndex ];
  case QUAD_TETRA: return QuadTetra_F[ faceIndex ];
  case QUAD_PYRAM: return QuadPyram_F[ faceIndex ];
  case QUAD_PENTA: return external ? QuadPenta_F[ faceIndex ] : QuadPenta_F[ faceIndex ];
    // what about SMDSEntity_TriQuad_Hexa?
  case QUAD_HEXA:  return external ? QuadHexa_F[ faceIndex ] : QuadHexa_F[ faceIndex ];
  case HEX_PRISM:  return external ? HexPrism_F[ faceIndex ] : HexPrism_F[ faceIndex ];
  default:;
  }
  return 0;
}

//=======================================================================
//function : NbFaceNodes
//purpose  : Return number of nodes in the array of face nodes
//=======================================================================

int SMDS_VolumeTool::NbFaceNodes(VolumeType type,
                                 int        faceIndex )
{
  switch ( type ) {
  case TETRA: return Tetra_nbN[ faceIndex ];
  case PYRAM: return Pyramid_nbN[ faceIndex ];
  case PENTA: return Penta_nbN[ faceIndex ];
  case HEXA:  return Hexa_nbN[ faceIndex ];
  case QUAD_TETRA: return QuadTetra_nbN[ faceIndex ];
  case QUAD_PYRAM: return QuadPyram_nbN[ faceIndex ];
  case QUAD_PENTA: return QuadPenta_nbN[ faceIndex ];
    // what about SMDSEntity_TriQuad_Hexa?
  case QUAD_HEXA:  return QuadHexa_nbN[ faceIndex ];
  case HEX_PRISM:  return HexPrism_nbN[ faceIndex ];
  default:;
  }
  return 0;
}

//=======================================================================
//function : Element
//purpose  : return element
//=======================================================================

const SMDS_MeshVolume* SMDS_VolumeTool::Element() const
{
  return static_cast<const SMDS_MeshVolume*>( myVolume );
}

//=======================================================================
//function : ID
//purpose  : return element ID
//=======================================================================

int SMDS_VolumeTool::ID() const
{
  return myVolume ? myVolume->GetID() : 0;
}
