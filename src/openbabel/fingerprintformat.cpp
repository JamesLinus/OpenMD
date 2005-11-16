/**********************************************************************
Copyright (C) 2005 by Chris Morley
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/
#include "fingerprintformat.hpp"

using namespace std;
namespace OpenBabel {

bool FingerprintFormat::WriteMolecule(OBBase* pOb, OBConversion* pConv)
{
	ostream &ofs = *pConv->GetOutStream();

	OBFingerprint* pFP;
	string id;
	if(pConv->IsOption("F"))
	{
		while(OBFingerprint::GetNextFPrt(id, pFP))
			ofs << id << " -- " << pFP->Description() << endl;
		return true;
	}


	bool hexoutput=false;
	if(pConv->IsOption("h") || (pConv->GetOutputIndex()==1 && pConv->IsLast()))
		hexoutput=true;

	string fpid;
	int nbits=0;
	const char* p=pConv->IsOption("f");
	if(p)
	{
		fpid=p;
		fpid = fpid.substr(0,fpid.find('"'));
	}

	pFP = OBFingerprint::FindFingerprint(fpid);
	if(!pFP)
	{
#ifdef HAVE_SSTREAM
	  stringstream errorMsg;
#else
	  strstream errorMsg;
#endif
		errorMsg << "Fingerprint type '" << fpid << "' not available" << endl;
		obErrorLog.ThrowError(__FUNCTION__, errorMsg.str(), obError);
		return false;
	}

	p=pConv->IsOption("N");
	if(p)
		nbits = atoi(p);		

	vector<unsigned int> fptvec;
	if(!pFP->GetFingerprint(pOb, fptvec, nbits))
		return false;
	
 	OBMol* pmol = dynamic_cast<OBMol*>(pOb);
	if(pmol)
		ofs << ">" << pmol->GetTitle();

	if(hexoutput)
	{
		int i, bitsset=0;
		for (i=0;i<fptvec.size();++i)
		{
			int wd = fptvec[i];
			for(;wd;wd=wd<<1)//count bits set by shifting into sign bit until word==0
				if(wd<0) ++bitsset;
		}
		ofs  << "   " << bitsset << " bits set. "; 
	}

	if(pConv->GetOutputIndex()==1)
	{
		//store the fingerprint and name of first molecule
		firstfp=fptvec;
		if(pmol)
			firstname=pmol->GetTitle();
		if(firstname.empty())
			firstname = "first mol";
	}
	else
	{
		ofs << "   Tanimoto from " << firstname << " = " << OBFingerprint::Tanimoto(firstfp, fptvec);
		if(IsPossibleSubstructure(fptvec,firstfp))
			ofs << "\nPossible superstructure of " << firstname;
	}
	ofs << endl;
	
	int i;

	if(hexoutput)
	{
		for(i=fptvec.size()-1;i>=0;i--)
		{
			ofs << hex << setfill('0') << setw(8) << fptvec[i] << " " ;
			if((fptvec.size()-i)%6==0)
				ofs <<endl;
		}
		ofs << dec << endl;
	}
	return true;
}

bool FingerprintFormat::IsPossibleSubstructure(vector<unsigned int>Mol, vector<unsigned int>Frag)
{
	//Returns false if Frag is definitely NOT a substructure of Mol
	int i;
	for (i=0;i<Mol.size();++i)
		if((Mol[i] & Frag[i]) ^ Frag[i]) return false;
	return true;
}

} //namespace OpenBabel
