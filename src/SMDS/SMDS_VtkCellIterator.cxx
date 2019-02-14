// Copyright (C) 2010-2019  CEA/DEN, EDF R&D, OPEN CASCADE
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

#include "SMDS_VtkCellIterator.hxx"
#include "utilities.h"

#include <vtkCell.h>
#include <vtkIdList.h>

_GetVtkNodes::_GetVtkNodes( vtkIdList*         _vtkIdList,
                            SMDS_Mesh*         mesh,
                            int                vtkCellId,
                            SMDSAbs_EntityType aType )
{
  vtkUnstructuredGrid* grid = mesh->GetGrid();
  const std::vector<int>& interlace = SMDS_MeshCell::fromVtkOrder( aType );
  if ( interlace.empty() )
  {
    grid->GetCellPoints( vtkCellId, _vtkIdList );
  }
  else
  {
    vtkIdType npts, *pts;
    grid->GetCellPoints( vtkCellId, npts, pts );
    _vtkIdList->SetNumberOfIds( npts );
    for (int i = 0; i < npts; i++)
      _vtkIdList->SetId(i, pts[interlace[i]]);
  }
}

_GetVtkNodesToUNV::_GetVtkNodesToUNV( vtkIdList*         _vtkIdList,
                                      SMDS_Mesh*         mesh,
                                      int                vtkCellId,
                                      SMDSAbs_EntityType aType )
{
  vtkIdType * pts, npts;
  vtkUnstructuredGrid* grid = mesh->GetGrid();
  grid->GetCellPoints( (vtkIdType)vtkCellId, npts, pts );
  const int *ids = 0;
  switch (aType)
  {
  case SMDSEntity_Quad_Edge:
  {
    static int id[] = { 0, 2, 1 };
    ids = id;
    break;
  }
  case SMDSEntity_Quad_Triangle:
  case SMDSEntity_BiQuad_Triangle:
  {
    static int id[] = { 0, 3, 1, 4, 2, 5 };
    ids = id;
    npts = 6;
    break;
  }
  case SMDSEntity_Quad_Quadrangle:
  case SMDSEntity_BiQuad_Quadrangle:
  {
    static int id[] = { 0, 4, 1, 5, 2, 6, 3, 7 };
    ids = id;
    npts = 8;
    break;
  }
  case SMDSEntity_Quad_Tetra:
  {
    static int id[] = { 0, 4, 1, 5, 2, 6, 7, 8, 9, 3 };
    ids = id;
    break;
  }
  case SMDSEntity_Quad_Pyramid:
  {
    static int id[] = { 0, 5, 1, 6, 2, 7, 3, 8, 9, 10, 11, 12, 4 };
    ids = id;
    break;
  }
  case SMDSEntity_Penta:
  {
    static int id[] = { 0, 2, 1, 3, 5, 4 };
    ids = id;
    break;
  }
  case SMDSEntity_Quad_Penta:
  case SMDSEntity_BiQuad_Penta: //TODO: check
  {
    static int id[] = { 0, 8, 2, 7, 1, 6, 12, 14, 13, 3, 11, 5, 10, 4, 9 };
    ids = id;
    break;
  }
  case SMDSEntity_Quad_Hexa:
  case SMDSEntity_TriQuad_Hexa:
  {
    static int id[] = { 0, 8, 1, 9, 2, 10, 3, 11, 16, 17, 18, 19, 4, 12, 5, 13, 6, 14, 7, 15 };
    ids = id;
    npts = 20;
    break;
  }
  case SMDSEntity_Polygon:
  case SMDSEntity_Quad_Polygon:
  case SMDSEntity_Polyhedra:
  case SMDSEntity_Quad_Polyhedra:
  default:
    const std::vector<int>& i = SMDS_MeshCell::interlacedSmdsOrder( aType, npts );
    if ( !i.empty() )
      ids = & i[0];
  }

  _vtkIdList->SetNumberOfIds( npts );

  if ( ids )
    for (int i = 0; i < npts; i++)
      _vtkIdList->SetId(i, pts[ids[i]]);
  else
    for (int i = 0; i < npts; i++)
      _vtkIdList->SetId(i, pts[i]);
}

_GetVtkNodesPolyh::_GetVtkNodesPolyh( vtkIdList*         _vtkIdList,
                                      SMDS_Mesh*         mesh,
                                      int                vtkCellId,
                                      SMDSAbs_EntityType aType )
{
  vtkUnstructuredGrid* grid = mesh->GetGrid();
  switch (aType)
  {
  case SMDSEntity_Polyhedra:
  {
    vtkIdType nFaces = 0;
    vtkIdType* ptIds = 0;
    grid->GetFaceStream( vtkCellId, nFaces, ptIds );
    int id = 0, nbNodesInFaces = 0;
    for (int i = 0; i < nFaces; i++)
    {
      int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
      nbNodesInFaces += nodesInFace;
      id += (nodesInFace + 1);
    }
    _vtkIdList->SetNumberOfIds( nbNodesInFaces );
    id = 0;
    int n = 0;
    for (int i = 0; i < nFaces; i++)
    {
      int nodesInFace = ptIds[id]; // nodeIds in ptIds[id+1 .. id+nodesInFace]
      for (int k = 1; k <= nodesInFace; k++)
        _vtkIdList->SetId(n++, ptIds[id + k]);
      id += (nodesInFace + 1);
    }
    break;
  }
  default:
    assert(0);
  }
}
