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

#include "applications/staticProps/StaticAnalyser.hpp"
#include "utils/simError.h"
#include "utils/Revision.hpp"

namespace OpenMD {
  StaticAnalyser::StaticAnalyser(SimInfo* info, const std::string& filename, unsigned int nbins) :
    info_(info), dumpFilename_(filename), step_(1), nBins_(nbins) {
    
    // Pre-load an OutputData for the count of objects:
    counts_ = new OutputData;
    counts_->units =  "objects";
    counts_->title =  "Objects";
    counts_->dataType = odtReal;
    counts_->dataHandling = odhTotal;
    counts_->accumulator.reserve(nBins_);
    for (unsigned int i = 0; i < nBins_; i++) 
      counts_->accumulator.push_back( new Accumulator() );     
  }
  
  void StaticAnalyser::writeOutput() {
    vector<OutputData*>::iterator i;
    OutputData* outputData;
        
    ofstream ofs(outputFilename_.c_str());

    if (ofs.is_open()) {
      
      Revision r;
      ofs << "# " << getAnalysisType() << "\n";
      ofs << "# OpenMD " << r.getFullRevision() << "\n";
      ofs << "# " << r.getBuildDate() << "\n";
      //ofs << "# selection script1: \"" << selectionScript1_ ;
      //ofs << "\"\tselection script2: \"" << selectionScript2_ << "\"\n";
      if (!paramString_.empty())
        ofs << "# parameters: " << paramString_ << "\n";
      ofs << "#";
      for(outputData = beginOutputData(i); outputData; 
          outputData = nextOutputData(i)) {
        ofs << "\t" << outputData->title << 
          "(" << outputData->units << ")";
        // add some extra tabs for column alignment
        if (outputData->dataType == odtVector3) ofs << "\t\t";
      }
      
      ofs << std::endl;
      
      ofs.precision(8);
      
      for (unsigned int j = 0; j < nBins_; j++) {        
        
        int counts = counts_->accumulator[j]->count();

        if (counts > 0) {
          for(outputData = beginOutputData(i); outputData; 
              outputData = nextOutputData(i)) {
            
            int n = outputData->accumulator[j]->count();
            if (n != 0) {
              writeData( ofs, outputData, j );
            }
          }
          ofs << std::endl;
        }
      }
        
      ofs << "#######################################################\n";
      ofs << "# 95% confidence intervals in those quantities follow:\n";
      ofs << "#######################################################\n";
      
      for (unsigned int j = 0; j < nBins_; j++) {
        int counts = counts_->accumulator[j]->count();
        if (counts > 0) {
          
          ofs << "#";
          for(outputData = beginOutputData(i); outputData; 
              outputData = nextOutputData(i)) {
            
            int n = outputData->accumulator[j]->count();
            if (n != 0) {
              writeErrorBars( ofs, outputData, j );
            }
          }
          ofs << std::endl;
        }
      }
      
      ofs.flush();
      ofs.close();      
      
    } else {      
      sprintf(painCave.errMsg, "StaticAnalyser: unable to open %s\n", 
              outputFilename_.c_str());
      painCave.isFatal = 1;
      simError();  
    }   
  }
    
  void StaticAnalyser::writeData(ostream& os, OutputData* dat, 
                                    unsigned int bin) {
    assert(int(bin) < nBins_);
    int n = dat->accumulator[bin]->count();
    if (n == 0) return;

    if( dat->dataType == odtReal ) {
      RealType r;
      if (dat->dataHandling == odhMax) {
	dynamic_cast<Accumulator*>(dat->accumulator[bin])->getMax(r);      
      } else {
	dynamic_cast<Accumulator*>(dat->accumulator[bin])->getAverage(r);      
      }
      if (std::isinf(r) || std::isnan(r) ) {      
        sprintf( painCave.errMsg,
                 "StaticAnalyser detected a numerical error writing:\n"
                 "\t%s for bin %u",
                 dat->title.c_str(), bin);
        painCave.isFatal = 1;
        simError();
      }
      if (dat->dataHandling == odhTotal) r *= dat->accumulator[bin]->count();
      os << "\t" << r;      

    } else if ( dat->dataType == odtVector3 ) {
      Vector3d v;
      dynamic_cast<VectorAccumulator*>(dat->accumulator[bin])->getAverage(v);     
      if (std::isinf(v[0]) || std::isnan(v[0]) || 
          std::isinf(v[1]) || std::isnan(v[1]) || 
          std::isinf(v[2]) || std::isnan(v[2]) ) {      
        sprintf( painCave.errMsg,
                 "StaticAnalyser detected a numerical error writing:\n"
                 "\t%s for bin %u",
                 dat->title.c_str(), bin);
        painCave.isFatal = 1;
        simError();
      }
      if (dat->dataHandling == odhTotal) v *= dat->accumulator[bin]->count();
      os << "\t" << v[0] << "\t" << v[1] << "\t" << v[2];
    }
  }

  void StaticAnalyser::writeErrorBars(ostream& os, OutputData* dat, 
                                    unsigned int bin) {
    assert(int(bin) < nBins_);
    int n = dat->accumulator[bin]->count();
    if (n == 0) return;

    if( dat->dataType == odtReal ) {
      RealType r;
      dynamic_cast<Accumulator*>(dat->accumulator[bin])->get95percentConfidenceInterval(r);      
      if (std::isinf(r) || std::isnan(r) ) {      
        sprintf( painCave.errMsg,
                 "StaticAnalyser detected a numerical error writing:\n"
                 "\tstandard deviation of %s for bin %u",
                 dat->title.c_str(), bin);
        painCave.isFatal = 1;
        simError();
      }
      if (dat->dataHandling == odhTotal) r *= dat->accumulator[bin]->count();
      os << "\t" << r;      

    } else if ( dat->dataType == odtVector3 ) {
      Vector3d v;
      dynamic_cast<VectorAccumulator*>(dat->accumulator[bin])->get95percentConfidenceInterval(v);
      if (std::isinf(v[0]) || std::isnan(v[0]) || 
          std::isinf(v[1]) || std::isnan(v[1]) || 
          std::isinf(v[2]) || std::isnan(v[2]) ) {      
        sprintf( painCave.errMsg,
                 "StaticAnalyser detected a numerical error writing:\n"
                 "\tstandard deviation of %s for bin %u",
                 dat->title.c_str(), bin);
        painCave.isFatal = 1;
        simError();
      }
      if (dat->dataHandling == odhTotal) v *= dat->accumulator[bin]->count();
      os << "\t" << v[0] << "\t" << v[1] << "\t" << v[2];
    }
  }
  
  
  OutputData* StaticAnalyser::beginOutputData(vector<OutputData*>::iterator& i) {
    i = data_.begin();
    return i != data_.end()? *i : NULL;
  }

  OutputData* StaticAnalyser::nextOutputData(vector<OutputData*>::iterator& i){
    ++i;
    return i != data_.end()? *i: NULL;
  }
}
