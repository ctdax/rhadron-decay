#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayer.h"

#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "G4Track.hh"
#include "G4DynamicParticle.hh"
#include "G4DecayProducts.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"
#include "TLorentzVector.h"

#include "Pythia8/Pythia.h"
#include "Pythia8/RHadrons.h"
#include "GeneratorInterface/Pythia8Interface/interface/P8RndmEngine.h"
#include "FWCore/ServiceRegistry/interface/RandomEngineSentry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "HepMC/GenEvent.h"
#include "HepMC/GenParticle.h"

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>

//////////////////////////
// Author: C. Thompson  //
// Date: May 29th, 2025 //
//////////////////////////

RHadronPythiaDecayer::RHadronPythiaDecayer( const std::string& s, const std::string& SLHAParticleDefinitionsFile, const std::string& commandFile )
 : G4VExtDecayer(s), SLHAParticleDefinitionsFile_(SLHAParticleDefinitionsFile), commandFile_(commandFile)
{
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Initializing Pythia8 instance for R-hadron decays.";
  pythia_ = std::make_unique<Pythia8::Pythia>();
  pythiaRandomEngine_ = std::make_shared<gen::P8RndmEngine>();
  //pythia_->setRndmEnginePtr(pythiaRandomEngine_.get());
  if (SLHAParticleDefinitionsFile_.empty()) {
    edm::LogWarning("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: No SLHA particle definitions file provided. Using default Pythia8 settings.";
  }
  else {
    pythia_->readString("SLHA:file = " + SLHAParticleDefinitionsFile_);
  }

  if (commandFile_.empty()) {
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: No command file provided. Using default RHadronPythiaDecayer settings.";
    pythia_->readString("RHadrons:all = on");
    pythia_->readString("RHadrons:allowDecay = on");
    pythia_->readString("RHadrons:probGluinoball = 0.1");
    pythia_->readString("Next:showMothersAndDaughters = on");
  } 
  else {
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Using command file: " << commandFile_;
    std::string line;
    std::ifstream command_stream(commandFile_);
    if (!command_stream.is_open()) {
      edm::LogError("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Could not open command file: " << commandFile_;
    }
    while(getline(command_stream, line)){
      pythia_->readString(line);
      edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Pythia8 command: " << line;
    }
    command_stream.close();
  }

  pythia_->init();
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Pythia8 instance initialized.";
}


G4DecayProducts* RHadronPythiaDecayer::ImportDecayProducts(const G4Track& aTrack){
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Importing decay products for track with ID " << aTrack.GetTrackID() << ". Name is " << aTrack.GetDefinition()->GetParticleName();
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Location is " << aTrack.GetPosition() << ". Velocity is " << aTrack.GetVelocity() / CLHEP::c_light << ". Proper time is " << aTrack.GetProperTime() << ". Global time is " << aTrack.GetGlobalTime();
  G4DecayProducts * dp = new G4DecayProducts();
  dp->SetParentParticle( *(aTrack.GetDynamicParticle()) );

  // get properties for later print outs
  G4double etot = aTrack.GetDynamicParticle()->GetTotalEnergy();

  // Outgoing particle
  std::vector<G4DynamicParticle*> particles;

  // Use Pythia8 to decay the particle and import the decay products
  decay(aTrack, particles);

  double totalE=0.0;
  for (unsigned int i=0; i<particles.size(); ++i){
    if (particles[i]) {
      dp->PushProducts(particles[i]);
      totalE += particles[i]->GetTotalEnergy();
    }
  }

  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Total energy in was " << etot << ". Total energy out was " << totalE;

  dp->DumpInfo();

  return dp;
}


void RHadronPythiaDecayer::fillParticle(const G4Track& aTrack, Pythia8::Event& event) const
{
  // Reset event record to allow for new event.
  event.reset();

  // Select particle mass; where relevant according to Breit-Wigner.
  double mm = aTrack.GetDynamicParticle()->GetMass();

  // Store the particle in the event record.
  event.append( aTrack.GetDefinition()->GetPDGEncoding(), 1, 0, 0, aTrack.GetMomentum().x()/CLHEP::GeV, aTrack.GetMomentum().y()/CLHEP::GeV,
                aTrack.GetMomentum().z()/CLHEP::GeV, aTrack.GetDynamicParticle()->GetTotalEnergy()/CLHEP::GeV, mm/CLHEP::GeV);
}


void RHadronPythiaDecayer::decay(const G4Track& aTrack, std::vector<G4DynamicParticle*> & particles)
{
  // Randomize the pythia engine
  std::unique_ptr<CLHEP::HepRandomEngine> engine_;
  edm::RandomEngineSentry<gen::P8RndmEngine> sentry(pythiaRandomEngine_.get(), engine_.get());
  Pythia8::Event& event = pythia_->event;

  fillParticle(aTrack, event); // Fill the pythia event with the Rhadron
  if (!pythia_->next()){ // Let pythia decay the Rhadron
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Pythia failed to generate the event, forcing Rhadron decay.";
    pythia_->forceRHadronDecays();
  }

  // Add the particles from the Pythia event into the GEANT particle vector
  for(int i=0; i<pythia_->event.size(); i++){
    if ( pythia_->event[i].status()<0 ) continue; // stable only
    G4ThreeVector momentum( pythia_->event[i].px() , pythia_->event[i].py() , pythia_->event[i].pz() );
    momentum *= 1000.0; // Convert GeV to MeV

    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    const G4ParticleDefinition* particleDefinition = particleTable->FindParticle(pythia_->event[i].id()); // Get the particle definition from the Pythia event
    if (!particleDefinition){
      edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: I don't know a definition for pdgid "<<pythia_->event[i].id()<<"! Skipping it...";
      continue;
    }

    G4DynamicParticle* dynamicParticle = new G4DynamicParticle(particleDefinition, momentum); // Create the dynamic particle and add it to Geant
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Adding " << particleDefinition->GetParticleName() << " with ID " << pythia_->event[i].id() << " and momentum " << momentum << " MeV to Geant.";
    particles.push_back( dynamicParticle );

    delete dynamicParticle;
  }

}