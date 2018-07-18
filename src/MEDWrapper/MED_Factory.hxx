// Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

#ifndef MED_Factory_HeaderFile
#define MED_Factory_HeaderFile

#include "MED_WrapperDef.hxx"
#include "MED_Wrapper.hxx"

#include <string>

namespace MED
{
  MEDWRAPPER_EXPORT
  std::string GetMEDVersion( const std::string& );

  MEDWRAPPER_EXPORT
  bool GetMEDVersion( const std::string&, int&, int&, int& );

  MEDWRAPPER_EXPORT
  bool CheckCompatibility( const std::string& , bool isForAppend=false);

  MEDWRAPPER_EXPORT
  PWrapper CrWrapperR( const std::string& );

  MEDWRAPPER_EXPORT
  PWrapper CrWrapperW( const std::string&, int theMinor=-1 );
}

#endif // MED_Factory_HeaderFile