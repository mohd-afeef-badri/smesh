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


#include "SMESH_ExtractGeometry.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkImplicitFunction.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"

using namespace std;

#ifdef _DEBUG_
static int MYDEBUG = 0;
#else
static int MYDEBUG = 0;
#endif

#if defined __GNUC__
  #if __GNUC__ == 2
    #define __GNUC_2__
  #endif
#endif


vtkStandardNewMacro(SMESH_ExtractGeometry);


SMESH_ExtractGeometry::SMESH_ExtractGeometry()
{}


SMESH_ExtractGeometry::~SMESH_ExtractGeometry(){}


vtkIdType SMESH_ExtractGeometry::GetElemObjId(int theVtkID){
  if(myElemVTK2ObjIds.empty() || theVtkID > myElemVTK2ObjIds.size()) return -1;
#if defined __GNUC_2__
  return myElemVTK2ObjIds[theVtkID];
#else
  return myElemVTK2ObjIds.at(theVtkID);
#endif
}


vtkIdType SMESH_ExtractGeometry::GetNodeObjId(int theVtkID){
  if(myNodeVTK2ObjIds.empty() || theVtkID > myNodeVTK2ObjIds.size()) return -1;
#if defined __GNUC_2__
  return myNodeVTK2ObjIds[theVtkID];
#else
  return myNodeVTK2ObjIds.at(theVtkID);
#endif
}


void SMESH_ExtractGeometry::Execute()
{
  vtkIdType ptId, numPts, numCells, i, cellId, newCellId, newId, *pointMap;
  vtkIdList *cellPts;
  vtkCell *cell;
  int numCellPts;
  float *x;
  float multiplier;
  vtkPoints *newPts;
  vtkIdList *newCellPts;
  vtkDataSet *input = this->GetInput();
  vtkPointData *pd = input->GetPointData();
  vtkCellData *cd = input->GetCellData();
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkPointData *outputPD = output->GetPointData();
  vtkCellData *outputCD = output->GetCellData();
  int npts;
  numCells = input->GetNumberOfCells();
  numPts = input->GetNumberOfPoints();
  
  vtkDebugMacro(<< "Extracting geometry");

  if ( ! this->ImplicitFunction )
    {
    vtkErrorMacro(<<"No implicit function specified");
    return;
    }

  newCellPts = vtkIdList::New();
  newCellPts->Allocate(VTK_CELL_SIZE);

  if ( this->ExtractInside )
    {
    multiplier = 1.0;
    }
  else 
    {
    multiplier = -1.0;
    }

  // Loop over all points determining whether they are inside the
  // implicit function. Copy the points and point data if they are.
  //
  pointMap = new vtkIdType[numPts]; // maps old point ids into new
  for (i=0; i < numPts; i++)
    {
    pointMap[i] = -1;
    }

  output->Allocate(numCells/4); //allocate storage for geometry/topology
  newPts = vtkPoints::New();
  newPts->Allocate(numPts/4,numPts);
  outputPD->CopyAllocate(pd);
  outputCD->CopyAllocate(cd);
  vtkFloatArray *newScalars = NULL;
  
  if(myStoreMapping){
    myElemVTK2ObjIds.clear();
    myElemVTK2ObjIds.reserve(numCells);
    myNodeVTK2ObjIds.clear();
    myNodeVTK2ObjIds.reserve(numPts);
  }

  if ( ! this->ExtractBoundaryCells )
    {
    for ( ptId=0; ptId < numPts; ptId++ )
      {
      x = input->GetPoint(ptId);
      if ( (this->ImplicitFunction->FunctionValue(x)*multiplier) < 0.0 )
        {
        newId = newPts->InsertNextPoint(x);
        pointMap[ptId] = newId;
	myNodeVTK2ObjIds.push_back(ptId);
        outputPD->CopyData(pd,ptId,newId);
        }
      }
    }
  else
    {
    // To extract boundary cells, we have to create supplemental information
    if ( this->ExtractBoundaryCells )
      {
      float val;
      newScalars = vtkFloatArray::New();
      newScalars->SetNumberOfValues(numPts);

      for (ptId=0; ptId < numPts; ptId++ )
        {
        x = input->GetPoint(ptId);
        val = this->ImplicitFunction->FunctionValue(x) * multiplier;
        newScalars->SetValue(ptId, val);
        if ( val < 0.0 )
          {
          newId = newPts->InsertNextPoint(x);
          pointMap[ptId] = newId;
	  myNodeVTK2ObjIds.push_back(ptId);
          outputPD->CopyData(pd,ptId,newId);
          }
        }
      }
    }

  // Now loop over all cells to see whether they are inside implicit
  // function (or on boundary if ExtractBoundaryCells is on).
  //
  for (cellId=0; cellId < numCells; cellId++)
    {
    cell = input->GetCell(cellId);
    cellPts = cell->GetPointIds();
    numCellPts = cell->GetNumberOfPoints();

    newCellPts->Reset();
    if ( ! this->ExtractBoundaryCells ) //requires less work
      {
      for ( npts=0, i=0; i < numCellPts; i++, npts++)
        {
        ptId = cellPts->GetId(i);
        if ( pointMap[ptId] < 0 )
          {
          break; //this cell won't be inserted
          }
        else
          {
          newCellPts->InsertId(i,pointMap[ptId]);
          }
        }
      } //if don't want to extract boundary cells
    
    else //want boundary cells
      {
      for ( npts=0, i=0; i < numCellPts; i++ )
        {
        ptId = cellPts->GetId(i);
        if ( newScalars->GetValue(ptId) <= 0.0 )
          {
          npts++;
          }
        }
      if ( npts > 0 )
        {
        for ( i=0; i < numCellPts; i++ )
          {
          ptId = cellPts->GetId(i);
          if ( pointMap[ptId] < 0 )
            {
            x = input->GetPoint(ptId);
            newId = newPts->InsertNextPoint(x);
            pointMap[ptId] = newId;
	    myNodeVTK2ObjIds.push_back(ptId);
            outputPD->CopyData(pd,ptId,newId);
            }
          newCellPts->InsertId(i,pointMap[ptId]);
          }
        }//a boundary or interior cell
      }//if mapping boundary cells
      
    if ( npts >= numCellPts || (this->ExtractBoundaryCells && npts > 0) )
      {
      newCellId = output->InsertNextCell(cell->GetCellType(),newCellPts);
      myElemVTK2ObjIds.push_back(cellId);
      outputCD->CopyData(cd,cellId,newCellId);
      }
    }//for all cells

  // Update ourselves and release memory
  //
  delete [] pointMap;
  newCellPts->Delete();
  output->SetPoints(newPts);
  newPts->Delete();
  
  if ( this->ExtractBoundaryCells )
    {
    newScalars->Delete();
    }

  output->Squeeze();
}