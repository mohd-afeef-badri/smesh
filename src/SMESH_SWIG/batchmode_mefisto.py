import os
import re

import batchmode_salome
import batchmode_geompy
import batchmode_smesh

geom = batchmode_geompy.geom
smesh = batchmode_smesh.smesh

geom.GetCurrentStudy(batchmode_salome.myStudyId)
smesh.SetCurrentStudy(batchmode_salome.myStudy)

ShapeType = batchmode_smesh.ShapeType

import StdMeshers

def CreateMesh (theFileName, area, len = None, nbseg = None):
    
    if not(os.path.isfile(theFileName)) or re.search("\.brep$", theFileName) is None :
        print "Incorrect file name !"
        return

    if (len is None) and (nbseg is None):
        print "Define length or number of segments !"
        return

    if (len is not None) and (nbseg is not None):
        print "Only one Hypothesis (from length and number of segments) can be defined !"
        return

    
    # ----  Import shape from BREP file and add it to the study  
    shape_mesh = geom.ImportBREP(theFileName)
    Id_shape = batchmode_geompy.addToStudy( shape_mesh, "shape_mesh")



    # ---- SMESH
      
    # ---- create Hypothesis

    print "-------------------------- create Hypothesis"
    if (len is not None):
        print "-------------------------- LocalLength"
        hypLength1 = smesh.CreateHypothesis("LocalLength", "libStdMeshersEngine.so")
        hypLength1.SetLength(len)
        print "Hypothesis type: ", hypLength1.GetName()
        print "Hypothesis ID: ", hypLength1.GetId()
        print "Hypothesis Value: ", hypLength1.GetLength()
    
    if (nbseg is not None):   
        print "-------------------------- NumberOfSegments"
        hypNbSeg1 = smesh.CreateHypothesis("NumberOfSegments", "libStdMeshersEngine.so")
        hypNbSeg1.SetNumberOfSegments(nbseg)
        print "Hypothesis type: ", hypNbSeg1.GetName()
        print "Hypothesis ID: ", hypNbSeg1.GetId()
        print "Hypothesis Value: ", hypNbSeg1.GetNumberOfSegments()


    if (area == "LengthFromEdges"):
        print "-------------------------- LengthFromEdges"
        hypLengthFromEdges = smesh.CreateHypothesis("LengthFromEdges", "libStdMeshersEngine.so")
        hypLengthFromEdges.SetMode(1)
        print "Hypothesis type: ", hypLengthFromEdges.GetName()
        print "Hypothesis ID: ", hypLengthFromEdges.GetId()
        print "LengthFromEdges Mode: ", hypLengthFromEdges.GetMode()
       
    else:
        print "-------------------------- MaxElementArea"
        hypArea1 = smesh.CreateHypothesis("MaxElementArea", "libStdMeshersEngine.so")
        hypArea1.SetMaxElementArea(area)
        print "Hypothesis type: ", hypArea1.GetName()
        print "Hypothesis ID: ", hypArea1.GetId()
        print "Hypothesis Value: ", hypArea1.GetMaxElementArea()

              
    
    print "-------------------------- Regular_1D"
    algoReg = smesh.CreateHypothesis("Regular_1D", "libStdMeshersEngine.so")
   
    listHyp = algoReg.GetCompatibleHypothesis()
    for hyp in listHyp:
        print hyp
    
    print "Algo name: ", algoReg.GetName()
    print "Algo ID: ", algoReg.GetId()
   
    print "-------------------------- MEFISTO_2D"
    algoMef = smesh.CreateHypothesis("MEFISTO_2D", "libStdMeshersEngine.so")
    
    listHyp=algoMef.GetCompatibleHypothesis()
    for hyp in listHyp:
        print hyp
        
    print "Algo name: ", algoMef.GetName()
    print "Algo ID: ", algoMef.GetId()



    # ---- add hypothesis to shape

    print "-------------------------- add hypothesis to shape"
    mesh = smesh.CreateMesh(shape_mesh) 

    ret = mesh.AddHypothesis(shape_mesh, algoReg)
    print "Add Regular_1D algo .... ", 
    print ret
    
    if (nbseg is not None):
        ret=mesh.AddHypothesis(shape_mesh, hypNbSeg1)
        print "Add Number Of Segements algo .... ", 
        print ret

    if (len is not None):
        ret=mesh.AddHypothesis(shape_mesh,hypLength1)
        print "Add  Local Length algo .... ", 
        print ret

    ret=mesh.AddHypothesis(shape_mesh, algoMef)
    print "Add MEFISTO_2D algo....", 
    print ret
    
    if (area == "LengthFromEdges"):
        ret = mesh.AddHypothesis( shape_mesh, hypLengthFromEdges)    # length from edge 
        print "Add Length From Edges algo .... ",
        print ret
    else:
        ret=mesh.AddHypothesis(shape_mesh, hypArea1)
        print "Add Max Triangle Area algo .... ", 
        print ret
    
    # ---- compute mesh

    print "-------------------------- compute mesh"
    ret=smesh.Compute(mesh,shape_mesh)
    print  "Compute Mesh .... ", 
    print ret
    log=mesh.GetLog(0); # no erase trace
    #for linelog in log:
    #    print linelog

    print "------------ INFORMATION ABOUT MESH ------------"
    
    print "Number of nodes: ", mesh.NbNodes()
    print "Number of edges: ", mesh.NbEdges()
    print "Number of faces: ", mesh.NbFaces()
    print "Number of triangles: ", mesh.NbTriangles()

    return mesh