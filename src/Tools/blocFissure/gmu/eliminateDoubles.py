# -*- coding: utf-8 -*-
# Copyright (C) 2014-2022  EDF R&D
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
"""éliminer les doublons d'une liste de subshapes"""

import logging
from .geomsmesh import geompy

def eliminateDoubles(obj, subshapes):
  """éliminer les doublons d'une liste de subshapes"""

  idsubs = dict()
  for sub in subshapes:
    subid = geompy.GetSubShapeID(obj, sub)
    if subid in idsubs:
      idsubs[subid].append(sub)
    else:
      idsubs[subid] = [sub]

  shortList = list()
  for _, l_sub in idsubs.items():
    shortList.append(l_sub[0])
  logging.debug("shortList=%s", shortList)

  return shortList
