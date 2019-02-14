#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2019  CEA/DEN, EDF R&D, OPEN CASCADE
#
# Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
# CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# Hexahedrization of the geometry generated by the Python script
# SMESH_fixation.py
# Hypothesis and algorithms for the mesh generation are global
#
import salome
import SMESH_fixation

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New()

compshell = SMESH_fixation.compshell
idcomp = SMESH_fixation.idcomp
geompy = SMESH_fixation.geompy
salome = SMESH_fixation.salome

print("Analysis of the geometry to be meshed :")
subShellList = geompy.SubShapeAll(compshell, geompy.ShapeType["SHELL"])
subFaceList  = geompy.SubShapeAll(compshell, geompy.ShapeType["FACE"])
subEdgeList  = geompy.SubShapeAll(compshell, geompy.ShapeType["EDGE"])

print("number of Shells in compshell : ", len(subShellList))
print("number of Faces  in compshell : ", len(subFaceList))
print("number of Edges  in compshell : ", len(subEdgeList))

status = geompy.CheckShape(compshell)
print(" check status ", status)

### ---------------------------- SMESH --------------------------------------
smesh.UpdateStudy()

# ---- init a Mesh with the compshell
shape_mesh = salome.IDToObject( idcomp  )

mesh = smesh.Mesh(shape_mesh, "MeshCompShell")


# ---- set Hypothesis and Algorithm

print("-------------------------- NumberOfSegments")

numberOfSegments = 5

regular1D = mesh.Segment()
regular1D.SetName("Wire Discretisation")
hypNbSeg = regular1D.NumberOfSegments(numberOfSegments)
print(hypNbSeg.GetName())
print(hypNbSeg.GetId())
print(hypNbSeg.GetNumberOfSegments())
smesh.SetName(hypNbSeg, "NumberOfSegments_" + str(numberOfSegments))

print("-------------------------- Quadrangle_2D")

quad2D = mesh.Quadrangle()
quad2D.SetName("Quadrangle_2D")

print("-------------------------- Hexa_3D")

hexa3D = mesh.Hexahedron()
hexa3D.SetName("Hexa_3D")

print("-------------------------- compute compshell")
ret = mesh.Compute()
print(ret)
if ret != 0:
    log = mesh.GetLog(0) # no erase trace
    # for linelog in log:
    #     print(linelog)
    print("Information about the MeshcompShel:")
    print("Number of nodes       : ", mesh.NbNodes())
    print("Number of edges       : ", mesh.NbEdges())
    print("Number of faces       : ", mesh.NbFaces())
    print("Number of quadrangles : ", mesh.NbQuadrangles())
    print("Number of volumes     : ", mesh.NbVolumes())
    print("Number of hexahedrons : ", mesh.NbHexas())
else:
    print("problem when Computing the mesh")

salome.sg.updateObjBrowser()
