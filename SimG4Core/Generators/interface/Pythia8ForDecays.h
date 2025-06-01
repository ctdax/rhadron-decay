/*
#ifndef Pythia8ForDecays_H
#define Pythia8ForDecays_H

#include <vector>
#include "G4Track.hh"
#include <utility>
#include <memory>

//////////////////////////
// Author: C. Thompson  //
// Date: May 29th, 2025 //
//////////////////////////

class G4DynamicParticle;
class G4ParticleDefinition;

namespace Pythia8 {
  class Pythia;
  class Event;
  class Rndm;
}

class Pythia8ForDecays
{
  public:
   Pythia8ForDecays();
   virtual ~Pythia8ForDecays() = default;

   /// Function that decays the RHadron; returns products in G4 format
   void Py1ent(const G4Track&, std::vector<G4DynamicParticle*> &);

  private:
   /// Helper for getting G4ParticleDefinition from PDG ID
   G4ParticleDefinition* GetParticleDefinition(const int) const;

   /// Fill a Pythia8 event with the information from a G4Track
   void fillParticle(const G4Track&, Pythia8::Event& event) const;

   /// Get the quarks from a gluino R-hadron.  From Pythia8 code.
   std::pair<int,int> fromIdWithGluino( int idRHad, Pythia8::Rndm* rndmPtr) const;
   std::pair<int,int> fromIdWithSquark( int idRHad) const;
   bool isGluinoRHadron(int pdgId) const;

   /// The instance of Pythia8 that will do the work
   std::unique_ptr<Pythia8::Pythia> m_pythia;
};

#endif
*/