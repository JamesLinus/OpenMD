/*
 * Copyright (c) 2007 The University of Notre Dame. All Rights Reserved.
 *
 * The University of Notre Dame grants you ("Licensee") a
 * non-exclusive, royalty free, license to use, modify and
 * redistribute this software in source and binary code form, provided
 * that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * This software is provided "AS IS," without a warranty of any
 * kind. All express or implied conditions, representations and
 * warranties, including any implied warranty of merchantability,
 * fitness for a particular purpose or non-infringement, are hereby
 * excluded.  The University of Notre Dame and its licensors shall not
 * be liable for any damages suffered by licensee as a result of
 * using, modifying or distributing the software or its
 * derivatives. In no event will the University of Notre Dame or its
 * licensors be liable for any lost revenue, profit or data, or for
 * direct, indirect, special, consequential, incidental or punitive
 * damages, however caused and regardless of the theory of liability,
 * arising out of the use of or inability to use software, even if the
 * University of Notre Dame has been advised of the possibility of
 * such damages.
 *
 * SUPPORT OPEN SCIENCE!  If you use OpenMD or its source code in your
 * research, please cite the appropriate papers when you publish your
 * work.  Good starting points are:
 *                                                                      
 * [1]  Meineke, et al., J. Comp. Chem. 26, 252-271 (2005).             
 * [2]  Fennell & Gezelter, J. Chem. Phys. 124, 234104 (2006).          
 * [3]  Sun, Lin & Gezelter, J. Chem. Phys. 128, 234107 (2008).          
 * [4]  Kuang & Gezelter,  J. Chem. Phys. 133, 164101 (2010).
 * [5]  Vardeman, Stocker & Gezelter, J. Chem. Theory Comput. 7, 834 (2011).
 */

/**
 * @file NonBondedInteractionType.hpp
 * @author    gezelter
 * @date  07/04/2007
 * @version 1.0
 */

#ifndef TYPES_NONBONDEDINTERACTIONTYPE_HPP
#define TYPES_NONBONDEDINTERACTIONTYPE_HPP

#include "types/AtomType.hpp"

namespace OpenMD {

  typedef  struct{
    bool is_LennardJones;
    bool is_Morse;
    bool is_MAW;
    bool is_EAM;
    bool is_SC;
    bool is_RepulsivePower;
    bool is_Mie;
    bool is_Buckingham;
  } NonBondedInteractionTypeProperties;

  /**
   * @class NonBondedInteractionType 
   *
   * NonBondedInteractionType class is responsible for keeping track
   * of static (unchanging) parameters for explicit non-bonded
   * interactions.
   */
  class NonBondedInteractionType {
  public:
    NonBondedInteractionType();
    virtual ~NonBondedInteractionType() { } ;
   
    void setLennardJones();    
    bool isLennardJones();
    void setMorse();    
    bool isMorse();
    void setMAW();
    bool isMAW();
    void setEAM();
    bool isEAM();
    bool isSC();
    void setSC();
    bool isMetal();
    void setRepulsivePower();
    bool isRepulsivePower();
    void setMie();
    bool isMie();
    void setBuckingham();
    bool isBuckingham();
    
    void setAtomTypes(std::pair<AtomType*, AtomType*> ats);
    std::pair<AtomType*, AtomType*> getAtomTypes();
   
    //below functions are just forward functions
    /**
     * Adds property into property map
     * @param genData GenericData to be added into PropertyMap
     */
    void addProperty(GenericData* genData);
    
    /**
     * Removes property from PropertyMap by name
     * @param propName the name of property to be removed
     */
    void removeProperty(const std::string& propName);
    
    /**
     * clear all of the properties
     */
    void clearProperties();
    
    /**
     * Returns all names of properties
     * @return all names of properties
     */
    std::vector<std::string> getPropertyNames();
    
    /**
     * Returns all of the properties in PropertyMap
     * @return all of the properties in PropertyMap
     */      
    std::vector<GenericData*> getProperties();
    
    /**
     * Returns property 
     * @param propName name of property
     * @return a pointer point to property with propName. If no
     * property named propName exists, return NULL
     */      
    GenericData* getPropertyByName(const std::string& propName);

    
  protected:
    NonBondedInteractionTypeProperties nbitp;
    std::pair<AtomType*, AtomType*> atomTypes_;
    
  private:
    //prevent copy construction and copy assignment, since property
    //map contains pointers which can not be copied and managed
    //safely, except make the generic data at PropertyMap as copy on
    //write shared pointer
    NonBondedInteractionType(const NonBondedInteractionType&);
    NonBondedInteractionType& operator=(const NonBondedInteractionType& nbit);
    PropertyMap properties_;
    
  };  

  struct EAMMixingParam {
    int nr;
    RealType dr;
    std::vector<RealType> phi; // phi(r)
  };

  typedef SimpleTypeData<EAMMixingParam> EAMMixingData;

} //end namespace OpenMD
#endif //TYPES_NONBONDEDINTERACTIONTYPE_HPP

