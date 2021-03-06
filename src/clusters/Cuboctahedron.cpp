/*
 * Copyright (c) 2016 The University of Notre Dame. All Rights Reserved.
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

#include "clusters/Cuboctahedron.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include "utils/CaseConversion.hpp"

using namespace std;

namespace OpenMD {

  bool pairCompare ( const pair<RealType, int>& l, const pair<RealType, int>& r) {
    return l.first < r.first;
  }
  
  Cuboctahedron::Cuboctahedron(std::string lattice, int cells, int planes) :
    lattice_(lattice), L_(cells), M_(planes) {
    
    Basis.clear();
    Points.clear();

    //
    // Initialize Basis vectors.
    //
    toUpper(lattice);

    if (lattice == "BCC") {
      Basis.push_back( Vector3d(0.0, 0.0, 0.0) );
      Basis.push_back( Vector3d(0.5, 0.5, 0.5) );
    } else if (lattice == "SC") {
      Basis.push_back( Vector3d(0.0, 0.0, 0.0) );
    } else {
      // Default is FCC:
      Basis.push_back( Vector3d(0.0, 0.0, 0.0) );
      Basis.push_back( Vector3d(0.5, 0.5, 0.0) );
      Basis.push_back( Vector3d(0.5, 0.0, 0.5) );
      Basis.push_back( Vector3d(0.0, 0.5, 0.5) );
    }

  }
  
  Cuboctahedron::~Cuboctahedron() {
    Basis.clear();
    Points.clear();
  }

  vector<Vector3d> Cuboctahedron::getPoints() {

    // center of cluster
    Vector3d c(0.5 * L_);

    Vector3d d;
    vector<Vector3d> rawPoints;       
    vector<pair<RealType, int> > dists;
    int idx;

    for (int i = 0; i < L_; i++) {
      for (int j = 0; j < L_; j++) {
        for (int k = 0; k < L_; k++) {
          for( vector<Vector3d>::iterator l = Basis.begin();
               l != Basis.end(); ++l ) {
            Vector3d point = (*l) + Vector3d(i, j, k);
            if (inCluster( point )) {
              rawPoints.push_back( point );
              d = point - c;
              idx = dists.size();
              dists.push_back( make_pair(d.lengthSquare(), idx) );
            }
          }
        }
      }
    }

    // Sort the atoms using distance from center of cluster:
    
    sort(dists.begin(), dists.end(), pairCompare);

    for( vector<pair<RealType, int> >::iterator i = dists.begin();
         i != dists.end(); ++i) {
      Points.push_back( rawPoints[ (*i).second ] - c );
    }
        
    return Points;
  }

  bool Cuboctahedron::inCluster110( Vector3d r ) {

    Vector3d c = (r - Vector3d(0.5 * L_)).abs();
    
    if ((c.x() > 0.5*L_) || (c.y() > 0.5*L_) || (c.z() > 0.5*L_)) {
      return false;
    }
    if ((c.x()+c.y() > 0.5*M_) || (c.y()+c.z() > 0.5*M_) ||
        (c.z()+c.y() > 0.5*M_)) {
      return false;
    }
    return true;
  }
  
  bool Cuboctahedron::inCluster111( Vector3d r ) {

    Vector3d c = (r - Vector3d(0.5 * L_)).abs();

    if ((c.x() < 0.5*L_) && (c.y() < 0.5*L_) && (c.z() < 0.5*L_) &&
        (c.x()+c.y()+c.z() < M_))
      return true;
    else
      return false;
  }

  bool Cuboctahedron::inCluster( Vector3d r ) {
    return inCluster111(r);
  }
}
