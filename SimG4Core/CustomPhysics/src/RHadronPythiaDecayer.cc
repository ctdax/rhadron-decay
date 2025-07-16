#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayer.h"

#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "G4Track.hh"
#include "G4DynamicParticle.hh"
#include "G4Decay.hh"
#include "G4Step.hh"
#include "G4DecayProducts.hh"
#include "G4VParticleChange.hh"
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

RHadronPythiaDecayer::RHadronPythiaDecayer( const std::string& SLHAParticleDefinitionsFile, const std::string& commandFile )
 : pythia_(new Pythia8::Pythia())
{
  // Initialize the Pythia8 instance for R-hadron decays
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Initializing Pythia8 instance for R-hadron decays.";

  // Read in the SLHA particle definitions file if provided
  if (SLHAParticleDefinitionsFile.empty()) {
    edm::LogWarning("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: No SLHA particle definitions file provided. Using default Pythia8 settings.";
  }
  else {
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Using SLHA particle definitions file: " << SLHAParticleDefinitionsFile;
    pythia_->readString("SLHA:file = " + SLHAParticleDefinitionsFile);
  }

  // Read in the command file for Pythia8 settings. If none is given use the following default settings.
  if (commandFile.empty()) {
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: No command file provided. Using default RHadronPythiaDecayer settings.";
    pythia_->readString("ProcessLevel:all = off");
    pythia_->readString("SUSY:all = on");
    pythia_->readString("RHadrons:all = on");
    pythia_->readString("RHadrons:allowDecay = on");
    pythia_->readString("RHadrons:probGluinoball = 0.1");
    pythia_->readString("PartonLevel:FSR = off");
  } 
  else {
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Using command file: " << commandFile;
    std::string line;
    std::ifstream command_stream(commandFile);
    if (!command_stream.is_open()) {
      edm::LogError("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer`: Could not open command file: " << commandFile;
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


G4VParticleChange* RHadronPythiaDecayer::DecayIt(const G4Track& aTrack, const G4Step& aStep) {
  // First call the standard DecayIt to generate secondaries
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Inside of RHadronPythiaDecayer::DecayIt!";
  G4VParticleChange* fParticleChangeForDecay = G4Decay::DecayIt(aTrack, aStep);
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Just ran G4Decay::DecayIt!";

  // Update the position of the secondaries in geant to match the potentially displaced positions from pythia. The list is stored in reverse order
  for (G4int i = fParticleChangeForDecay->GetNumberOfSecondaries() - 1; i >= 0; --i) {
    G4Track* secondary = fParticleChangeForDecay->GetSecondary(i);
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Updating secondary particle " << i << " name is " << secondary->GetDefinition()->GetParticleName() << ". Initial position was " << secondary->GetPosition() << ". Displacement is " << secondaryDisplacements_[i];
    secondary->SetPosition(secondary->GetPosition() + secondaryDisplacements_[i]);
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Updated position to " << secondary->GetPosition();
  }

  return fParticleChangeForDecay;
}


G4DecayProducts* RHadronPythiaDecayer::ImportDecayProducts(const G4Track& aTrack){
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Importing decay products for track with ID " << aTrack.GetTrackID() << ". Name is " << aTrack.GetDefinition()->GetParticleName();
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Location is " << aTrack.GetPosition() << ". Velocity is " << aTrack.GetVelocity() / CLHEP::c_light << ". Proper time is " << aTrack.GetProperTime() << ". Global time is " << aTrack.GetGlobalTime();
  G4DecayProducts * dp = new G4DecayProducts();
  dp->SetParentParticle( *(aTrack.GetDynamicParticle()) );

  // get properties for later print outs
  G4double E_initial = aTrack.GetDynamicParticle()->GetTotalEnergy();

  // Outgoing particle
  std::vector<G4DynamicParticle*> particles;

  // Use Pythia8 to decay the particle and import the decay products
  pythiaDecay(aTrack, particles);

  double E_final=0.0;
  for (unsigned int i=0; i<particles.size(); ++i){
    if (particles[i]) {
      dp->PushProducts(particles[i]);
      E_final += particles[i]->GetTotalEnergy();
    }
  }

  if ((E_initial / CLHEP::GeV >= 1.01 * E_final / CLHEP::GeV) || (E_initial / CLHEP::GeV <= 0.99 * E_final / CLHEP::GeV)) {
    edm::LogWarning("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Energy not conserved in decay. Initial energy = " << E_initial / CLHEP::GeV << " GeV. Final energy = " << E_final / CLHEP::GeV << " GeV.";
  }
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Total energy in was " << E_initial / CLHEP::GeV << " GeV, total energy out is " << E_final / CLHEP::GeV << " GeV.";

  dp->DumpInfo();

  return dp;
}


void RHadronPythiaDecayer::fillParticle(const G4Track& aTrack, Pythia8::Event& event) const
{
  // Reset event record to allow for new event.
  event.reset();

  // Get particle mass and 4-momentum.
  double mass = aTrack.GetDynamicParticle()->GetMass() / CLHEP::GeV;
  const G4LorentzVector g4p4 = aTrack.GetDynamicParticle()->Get4Momentum() / CLHEP::GeV;
  Pythia8::Vec4 p4(g4p4.px(), g4p4.py(), g4p4.pz(), g4p4.e());

  // Store the particle in the event record.
  event.append( aTrack.GetDefinition()->GetPDGEncoding(), 1, 0, 0, p4, mass);
}


void RHadronPythiaDecayer::pythiaDecay(const G4Track& aTrack, std::vector<G4DynamicParticle*> & particles)
{
  Pythia8::Event& event = pythia_->event;

  fillParticle(aTrack, event); // Fill the pythia event with the Rhadron
  if (!pythia_->next()){ // Let pythia decay the Rhadron
    edm::LogWarning("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Pythia failed to generate the event, forcing Rhadron decay.";
    pythia_->forceRHadronDecays();
  }

  // Add the particles from the Pythia event into the Geant particle vector
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  for(int i=0; i<pythia_->event.size(); i++){
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Decay product " << i << " with ID " << pythia_->event[i].id() << " and status " << pythia_->event[i].status() << " has production vertex " << pythia_->event[i].vProd() << " and decay vertex " << pythia_->event[i].vDec();
    if ( pythia_->event[i].status()<0 ) continue; // stable only
    G4ThreeVector displacement(pythia_->event[i].xProd(), pythia_->event[i].yProd(), pythia_->event[i].zProd());
    G4ThreeVector momentum(pythia_->event[i].px(), pythia_->event[i].py(), pythia_->event[i].pz());
    momentum *= 1000.0; // Convert GeV to MeV

    const G4ParticleDefinition* particleDefinition = particleTable->FindParticle(pythia_->event[i].id()); // Get the particle definition from the Pythia event
    if (!particleDefinition){
      edm::LogWarning("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: I don't know a definition for pdgid " << pythia_->event[i].id() << "! Skipping it...";
      continue;
    }

    G4DynamicParticle* dynamicParticle = new G4DynamicParticle(particleDefinition, momentum); // Create the dynamic particle and add it to Geant
    edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHadronPythiaDecayer: Adding " << particleDefinition->GetParticleName() << " with ID " << pythia_->event[i].id() << " and momentum " << momentum << " MeV to Geant.";
    particles.push_back( dynamicParticle );
    secondaryDisplacements_.push_back(displacement); // Store the position of the secondary particle to update in RHadronPythiaDecayer::DecayIt
  }
}