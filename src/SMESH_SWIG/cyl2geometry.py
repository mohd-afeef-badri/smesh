# CEA/LGLS 2004, Francis KLOSS (OCC)
# ==================================

# Import
# ------

from geompy import *
from meshpy import *

# Piece
# -----

# Creer la geometrie en bloc hexahedrique d'une piece en forme de T composee de 2 cylindres de diametre different dont les axes se coupent orthogonalement,
# puis mailler en hexahedrique.

cx = 0
cy = 0
cz = 0

g_rayon   = 100.0
g_hauteur = 500

p_rayon   =  50.0
p_hauteur = 500

g_trim = 1000

# Geometrie
# =========

cpd = []

# Gros cylindre
# -------------

g_base = MakeVertex(cx, cy, cz)
g_dir  = MakeVectorDXDYDZ(0, 0, 1)
t_hauteur = p_rayon+10.0

g_cyl = MakeCylinder(g_base, g_dir, g_rayon, g_hauteur)

g_coupe = MakeVectorDXDYDZ(1, 0, 0)

g_tools = []
g_tools.append(MakePlane(MakeVertex(cx+t_hauteur, cy, cz), g_coupe, g_trim))
g_tools.append(MakePlane(MakeVertex(cx-t_hauteur, cy, cz), g_coupe, g_trim))

g_partie = MakePartition([g_cyl], g_tools, [], [], ShapeType["SOLID"])
g_bas, g_centre, g_haut = SubShapeAllSorted(g_partie, ShapeType["SOLID"])

# Partie basse du gros cylindre
# -----------------------------

b_hauteur = 10
b_base    = 20

b_boite = MakeBox(cx-t_hauteur, cy-b_base, cz,  cx-t_hauteur-b_hauteur, cy+b_base, cz+g_hauteur)
cpd.append(b_boite)

b_cyl = MakeCut(g_bas, b_boite)

b_tools = []
b_tools.append(MakePlane(MakeVertex(cx-t_hauteur-b_hauteur, cy+b_base, cz), MakeVectorDXDYDZ( 1, 1, 0), g_trim))
b_tools.append(MakePlane(MakeVertex(cx-t_hauteur-b_hauteur, cy-b_base, cz), MakeVectorDXDYDZ(-1, 1, 0), g_trim))

b_partie = MakePartition([b_cyl], b_tools, [], [], ShapeType["SOLID"])
b_element = SubShapeAll(b_partie, ShapeType["SOLID"])
cpd = cpd + b_element

# Partie haute du gros cylindre
# -----------------------------

h_plan = MakePlane(g_base, g_coupe, g_trim)

cpd.append(MakeMirrorByPlane(b_boite, h_plan))

for h in b_element:
    h_symetrie = MakeMirrorByPlane(h, h_plan)
    cpd.append(h_symetrie)

# Petit cylindre
# --------------

z_arete = p_rayon/2
x_arete = z_arete*t_hauteur*2/g_hauteur

px = cx-x_arete
py = cy-1.5*g_rayon
pz = cz+g_hauteur/2

p_base = MakeVertex(cx, py, pz)
p_dir  = MakeVectorDXDYDZ(0, 1, 0)
p_cyl  = MakeCylinder(p_base, p_dir, p_rayon, p_hauteur)

p_boite = MakeBox(px, py, pz-z_arete,  cx+x_arete, py+p_hauteur, pz+z_arete)

# Partie interieure du petit cylindre
# -----------------------------------

i_cyl   = MakeCommon(p_cyl, g_cyl)
i_tuyau = MakeCut(i_cyl, p_boite)
i_boite = MakeCommon(p_boite, g_cyl)

# Partie exterieure du petit cylindre
# -----------------------------------

e_cyl0 = MakeCut(p_cyl, g_cyl)
e_cyl  = SubShapeAllSorted(e_cyl0, ShapeType["SOLID"])

e_tuyau = MakeCut(e_cyl[1], p_boite)

e_boite0 = MakeCut(p_boite, g_cyl)
e_boite  = SubShapeAllSorted(e_boite0, ShapeType["SOLID"])

cpd.append(e_boite[1])

# Partie centrale du gros cylindre
# --------------------------------

c_cyl = MakeCut(g_centre, p_cyl)

# Partitionner
# ------------

p_tools = []
p_tools.append(MakePlane(MakeVertex(px, py, pz-z_arete), MakeVectorDXDYDZ(-z_arete, 0, x_arete), g_trim))
p_tools.append(MakePlane(MakeVertex(px, py, pz+z_arete), MakeVectorDXDYDZ( z_arete, 0, x_arete), g_trim))

p_partie = MakePartition([e_tuyau], p_tools, [], [], ShapeType["SOLID"])
p_element = SubShapeAll(p_partie, ShapeType["SOLID"])
cpd = cpd + p_element

q_partie = MakePartition([i_tuyau, c_cyl], p_tools, [], [], ShapeType["SOLID"])
q_element = SubShapeAll(q_partie, ShapeType["SOLID"])

q_element = q_element + [i_boite]

q_tools = []
q_tools.append(MakePlane(MakeVertex(cx, cy-b_base, cz), MakeVectorDXDYDZ(0, 1, 0), g_trim))
q_tools.append(MakePlane(MakeVertex(cx, cy+b_base, cz), MakeVectorDXDYDZ(0, 1, 0), g_trim))

r_element = []
for e in q_element:
    r_partie = MakePartition([e], q_tools, [], [], ShapeType["SOLID"])
    r_element = r_element + SubShapeAll(r_partie, ShapeType["SOLID"])

cpd = cpd + r_element

# Compound
# --------

comp_all = MakeCompound(cpd)
piece = BlocksOp.RemoveExtraEdges(comp_all)

# Ajouter la piece dans l'etude
# -----------------------------

piece_id = addToStudy(piece, "T2Cylindres")

# Maillage
# ========

# Mailler des hexahedres
# ----------------------

m_hexa=MeshHexa(piece, 4, "T2CylindresHexa")

# Calculer le maillage
# --------------------

m_hexa.Compute()