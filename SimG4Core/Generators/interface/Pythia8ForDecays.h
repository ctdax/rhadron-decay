/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Abused from Geant4 version of Pythai6.hh from extended examples

#ifndef Pythia8ForDecays_H
#define Pythia8ForDecays_H

// For std::vector
#include <vector>
// G4 track function parameter
#include "G4Track.hh"
// For std::pair
#include <utility>
// For all the various Pythia8 classes used here
#include "Pythia8_i/Pythia8_i.h"
// For unique_ptr
#include <memory>
#include "CxxUtils/checker_macros.h"

class G4DynamicParticle;
class G4ParticleDefinition;

class ATLAS_NOT_THREAD_SAFE Pythia8ForDecays
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