/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RHadronPythiaDecayer_H
#define RHadronPythiaDecayer_H

#include "SimG4Core/Generators/interface/Pythia8ForDecays.h"

#include "G4VExtDecayer.hh"
#include "G4Track.hh"
#include <string>
//#include "CxxUtils/checker_macros.h"

class G4DecayProducts;

//class ATLAS_NOT_THREAD_SAFE RHadronPythiaDecayer: public G4VExtDecayer
class RHadronPythiaDecayer: public G4VExtDecayer
{
  public:
   RHadronPythiaDecayer( const std::string& s );
   virtual G4DecayProducts* ImportDecayProducts(const G4Track&);
  private:
   Pythia8ForDecays m_pythia;
};

#endif