//  SMESH SMESH : implementaion of SMESH idl descriptions
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
//  File   : StdMeshers_NumberOfSegments.hxx
//           Moved here from SMESH_NumberOfSegments.hxx
//  Author : Paul RASCLE, EDF
//  Module : SMESH
//  $Header$

#ifndef _SMESH_NUMBEROFSEGMENTS_HXX_
#define _SMESH_NUMBEROFSEGMENTS_HXX_

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"
#include <vector>

/*!
 * \brief This class represents hypothesis for 1d algorithm
 * 
 * It provides parameters for subdivision an edge by various
 * distribution types, considering the given number of resulting segments
 */
class StdMeshers_NumberOfSegments:
  public SMESH_Hypothesis
{
public:
  StdMeshers_NumberOfSegments(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~StdMeshers_NumberOfSegments();

  /*!
   * \brief Set the number of segments
    * \param segmentsNumber - must be greater than zero
   */
  void SetNumberOfSegments(int segmentsNumber)
    throw (SALOME_Exception);

  /*!
   * \brief Get the number of segments
   */
  int GetNumberOfSegments() const;

  /*!
   * \brief This enumeration presents available types of distribution
   */
  enum DistrType
  {
    DT_Regular, //!< equidistant distribution
    DT_Scale,   //!< scale distribution
    DT_TabFunc, //!< distribution with density function presented by table
    DT_ExprFunc //!< distribution with density function presented by expression
  };

  /*!
   * \brief Set distribution type
   */
  void SetDistrType(DistrType typ)
    throw (SALOME_Exception);

  /*!
   * \brief Get distribution type
   */
  DistrType GetDistrType() const;

  /*!
   * \brief Set scale factor for scale distribution
   * \param scaleFactor - positive value different from 1
   * 
   * Throws SALOME_Exception if distribution type is not DT_Scale,
   * or scaleFactor is not a positive value different from 1
   */
  virtual void SetScaleFactor(double scaleFactor)
    throw (SALOME_Exception);

  /*!
   * \brief Get scale factor for scale distribution
   * 
   * Throws SALOME_Exception if distribution type is not DT_Scale
   */
  double GetScaleFactor() const
    throw (SALOME_Exception);

  /*!
   * \brief Set table function for distribution DT_TabFunc
    * \param table - this vector contains the pairs (parameter, value)
   * following each by other, so the number of elements in the vector
   * must be even. The parameters must be in range [0,1] and sorted in
   * increase order. The values of function must be positive.
   * 
   * Throws SALOME_Exception if distribution type is not DT_TabFunc
   */
  void SetTableFunction(const std::vector<double>& table)
    throw (SALOME_Exception);

  /*!
   * \brief Get table function for distribution DT_TabFunc
   * 
   * Throws SALOME_Exception if distribution type is not DT_TabFunc
   */
  const std::vector<double>& GetTableFunction() const
    throw (SALOME_Exception);

  /*!
   * \brief Set expression function for distribution DT_ExprFunc
    * \param expr - string containing the expression of the function
    *               f(t), e.g. "sin(t)"
   * 
   * Throws SALOME_Exception if distribution type is not DT_ExprFunc
   */
  void SetExpressionFunction( const char* expr)
    throw (SALOME_Exception);

  /*!
   * \brief Get expression function for distribution DT_ExprFunc
   * 
   * Throws SALOME_Exception if distribution type is not DT_ExprFunc
   */
  const char* GetExpressionFunction() const
    throw (SALOME_Exception);

  /*!
   * \brief When exponent mode is set, the function of distribution of density
   * is used as an exponent of 10, i,e, 10^f(t). This mode is sensible only when
   * function distribution is used (DT_TabFunc or DT_ExprFunc)
    * \param isExp - boolean switching on/off the mode
   * 
   * Throws SALOME_Exception if distribution type is not functional
   */
  void SetExponentMode(bool isExp)
    throw (SALOME_Exception);

  /*!
   * \brief Returns true if the exponent mode is set
   * 
   * Throws SALOME_Exception if distribution type is not functional
   */
  bool IsExponentMode() const
    throw (SALOME_Exception);

  virtual ostream & SaveTo(ostream & save);
  virtual istream & LoadFrom(istream & load);
  friend ostream& operator << (ostream & save, StdMeshers_NumberOfSegments & hyp);
  friend istream& operator >> (istream & load, StdMeshers_NumberOfSegments & hyp);

protected:
  int                 _numberOfSegments; //!< an edge will be split on to this number of segments
  DistrType           _distrType;        //!< the type of distribution of density function
  double              _scaleFactor;      //!< the scale parameter for DT_Scale
  std::vector<double> _table;            //!< the table for DT_TabFunc, a sequence of pairs of numbers
  std::string         _func;             //!< the expression of the function for DT_ExprFunc
  bool                _expMode;          //!< flag of exponent mode
};

#endif
