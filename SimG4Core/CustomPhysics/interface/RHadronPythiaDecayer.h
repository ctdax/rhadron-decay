#ifndef RHadronPythiaDecayer_H
#define RHadronPythiaDecayer_H

#include "G4VExtDecayer.hh"
#include "G4Track.hh"
#include <string>
#include <vector>
#include <utility>
#include <memory>

//////////////////////////
// Author: C. Thompson  //
// Date: May 29th, 2025 //
//////////////////////////

namespace gen {
  class P8RndmEngine;
  typedef std::shared_ptr<P8RndmEngine> P8RndmEnginePtr;
}

namespace Pythia8 {
  class Pythia;
  class Event;
  class Rndm;
}

class G4DynamicParticle;
class G4DecayProducts;
class RHadronPythiaDecayer: public G4VExtDecayer
{
  public:
   RHadronPythiaDecayer( const std::string& s, const std::string& SLHAParticleDefinitionsFile, const std::string& commandFile ); //Constructor
   virtual ~RHadronPythiaDecayer() = default; //Destructor

   virtual G4DecayProducts* ImportDecayProducts(const G4Track&); //Import the decay products

   void decay(const G4Track&, std::vector<G4DynamicParticle*> &); //Function to decay the RHadron and return products in G4 format

  private:
   void fillParticle(const G4Track&, Pythia8::Event& event) const; //Fill a Pythia8 event with the information from a G4Track

   std::unique_ptr<Pythia8::Pythia> pythia_; // Instance of pythia
   gen::P8RndmEnginePtr pythiaRandomEngine_; // Instance of pythia Random engine
   std::string SLHAParticleDefinitionsFile_; // Text file with particle definitions for Pythia8
   std::string commandFile_; // Text file with commands for Pythia8
};

#endif