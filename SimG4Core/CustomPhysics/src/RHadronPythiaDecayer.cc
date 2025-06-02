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

static inline unsigned short int nth_digit(const int& val,const unsigned short& n) { return (std::abs(val)/(int(std::pow(10,n-1))))%10;}

RHadronPythiaDecayer::RHadronPythiaDecayer( const std::string& s )
 : G4VExtDecayer(s)
{
  // Pythia instance where RHadrons can decay
  //std::string docstring = Pythia8_i::xmlpath();
  //pythia_ = std::make_unique<Pythia8::Pythia>(docstring);
  //pythia_ = new Pythia8::Pythia();
  pythia_ = std::make_unique<Pythia8::Pythia>();
  pythiaRandomEngine_ = std::make_shared<gen::P8RndmEngine>();
  pythia_->setRndmEnginePtr(pythiaRandomEngine_.get());
  pythia_->readString("SLHA:file = SLHA_INPUT.DAT");
  pythia_->readString("ProcessLevel:all = off");
  pythia_->readString("Init:showChangedSettings = off");
  pythia_->readString("RHadrons:allow = on");
  pythia_->readString("RHadrons:allowDecay = on");
  pythia_->readString("RHadrons:probGluinoball = 0.1");
  pythia_->readString("PartonLevel:FSR = off");
  pythia_->readString("Init:showAllParticleData = on");

  // Process the file of commands left for us by the python layer
  std::string line;
  std::ifstream command_stream ("PYTHIA8_COMMANDS.TXT");
  while(getline(command_stream,line)){
    // Leaving it to the top-level to get this file right
    pythia_->readString(line);
  }
  command_stream.close();

  pythia_->init();
}


G4DecayProducts* RHadronPythiaDecayer::ImportDecayProducts(const G4Track& aTrack){
  G4DecayProducts * dp = new G4DecayProducts();
  dp->SetParentParticle( *(aTrack.GetDynamicParticle()) );

  // get properties for later print outs
  G4double etot = aTrack.GetDynamicParticle()->GetTotalEnergy();
  G4int pdgEncoding = aTrack.GetDefinition()->GetPDGEncoding();

  // Outgoing particle
  std::vector<G4DynamicParticle*> particles;

  // Pythia8 decay the particle and import the decay products
  decay(aTrack, particles);

  G4cout << "Decayed an RHadron with ID " << pdgEncoding << " and momentum " << aTrack.GetMomentum() << " in Pythia.  Decay products are:" << G4endl;
  double totalE=0.0;
  for (unsigned int i=0; i<particles.size(); ++i){
    if (particles[i]) {
      dp->PushProducts(particles[i]);
      totalE += particles[i]->GetTotalEnergy();
    }
    else {
      G4cout << i << " null pointer!" << G4endl;
    }
  }

  G4cout << "Total energy in was "<<etot   << G4endl;
  G4cout << "Total energy out is "<<totalE << G4endl;

  dp->DumpInfo();

  return dp;
}


G4ParticleDefinition* RHadronPythiaDecayer::GetParticleDefinition(const int pdgEncoding) const
{
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* particleDefinition(nullptr);
  if (pdgEncoding != 0) particleDefinition = particleTable->FindParticle(pdgEncoding);
  return particleDefinition;
}


std::pair<int,int> RHadronPythiaDecayer::fromIdWithSquark( int idRHad) const
{

  // Find squark flavour content.
  int idRSb            = pythia_->settings.mode("RHadrons:idSbottom");
  int idRSt            = pythia_->settings.mode("RHadrons:idStop");
  int idLight = (abs(idRHad) - 1000000) / 10;
  int idSq    = (idLight < 100) ? idLight/10 : idLight/100;
  int id1     = (idSq == 6) ? idRSt : idRSb;
  if (idRHad < 0) id1 = -id1;

  // Find light (di)quark flavour content.
  int id2     =  (idLight < 100) ? idLight%10 : idLight%100;
  if (id2 > 10) id2 = 100 * id2 + abs(idRHad)%10;
  if ((id2 < 10 && idRHad > 0) || (id2 > 10 && idRHad < 0)) id2 = -id2;

  // Done.
  return std::make_pair( id1, id2);

}

// TODO: Would be nice for this to be a public function in Pythia8::RHadrons.hh
std::pair<int,int> RHadronPythiaDecayer::fromIdWithGluino( int idRHad, Pythia8::Rndm* rndmPtr) const
{
  // Find light flavour content of R-hadron.
  int idLight = (abs(idRHad) - 1000000) / 10;
  int id1, id2, idTmp, idA, idB, idC;
  double diquarkSpin1RH = 0.5;

  // Gluinoballs: split g into d dbar or u ubar.
  if (idLight < 100) {
    id1 = (rndmPtr->flat() < 0.5) ? 1 : 2;
    id2 = -id1;

  // Gluino-meson: split into q + qbar.
  } else if (idLight < 1000) {
    id1 = (idLight / 10) % 10;
    id2 = -(idLight % 10);
    // Flip signs when first quark of down-type.
    if (id1%2 == 1) {
      idTmp = id1;
      id1   = -id2;
      id2   = -idTmp;
    }

  // Gluino-baryon: split to q + qq (diquark).
  // Pick diquark at random, except if c or b involved.
  } else {
    idA = (idLight / 100) % 10;
    idB = (idLight / 10) % 10;
    idC = idLight % 10;
    double rndmQ = 3. * rndmPtr->flat();
    if (idA > 3) rndmQ = 0.5;
    if (rndmQ < 1.) {
      id1 = idA;
      id2 = 1000 * idB + 100 * idC + 3;
      if (idB != idC && rndmPtr->flat() > diquarkSpin1RH) id2 -= 2;
    } else if (rndmQ < 2.) {
      id1 = idB;
      id2 = 1000 * idA + 100 * idC + 3;
      if (idA != idC && rndmPtr->flat() > diquarkSpin1RH) id2 -= 2;
    } else {
      id1 = idC;
      id2 = 1000 * idA + 100 * idB +3;
      if (idA != idB && rndmPtr->flat() > diquarkSpin1RH) id2 -= 2;
    }
  }
  // Flip signs for anti-R-hadron.
  if (idRHad < 0) {
    idTmp = id1;
    id1   = -id2;
    id2   = -idTmp;
  }

  // Done.
  return std::make_pair( id1, id2);

}

/// Add a G4Track to a Pythia8 event to make it a single-particle gun. The particle must be a colour singlet.
/// Input: particle, Pythia8 event
void RHadronPythiaDecayer::fillParticle(const G4Track& aTrack, Pythia8::Event& event) const
{
  // Reset event record to allow for new event.
  event.reset();

  // Select particle mass; where relevant according to Breit-Wigner.
  double mm = aTrack.GetDynamicParticle()->GetMass();

  // Store the particle in the event record.
  event.append( aTrack.GetDefinition()->GetPDGEncoding(), 1, 0, 0, aTrack.GetMomentum().x()/CLHEP::GeV, aTrack.GetMomentum().y()/CLHEP::GeV,
                aTrack.GetMomentum().z()/CLHEP::GeV, aTrack.GetDynamicParticle()->GetTotalEnergy()/CLHEP::GeV, mm/CLHEP::GeV);
  // Note: this function returns an int, but we don't need or use its output
}

bool RHadronPythiaDecayer::isGluinoRHadron(int pdgId) const{
  // Checking what kind of RHadron this is based on the digits in its PDGID
  const unsigned short digitValue_q1 = nth_digit(pdgId,4);
  const unsigned short digitValue_l = nth_digit(pdgId,5);

  // Gluino R-Hadrons have the form 109xxxx or 1009xxx
  if (digitValue_l == 9 || (digitValue_l==0 && digitValue_q1 == 9) ){
    // This is a gluino R-Hadron
    return true;
  }

  // Special case : R-gluinoball
  if (pdgId==1000993) return true;

  // This is not a gluino R-Hadron (probably a squark R-Hadron)
  return false;

}

void RHadronPythiaDecayer::decay(const G4Track& aTrack, std::vector<G4DynamicParticle*> & particles)
{
  // Randomize pythia engine
  std::unique_ptr<CLHEP::HepRandomEngine> engine_;
  edm::RandomEngineSentry<gen::P8RndmEngine> sentry(pythiaRandomEngine_.get(), engine_.get());

  // Get members from Pythia8 instance where RHadrons can decay
  Pythia8::Event& event      = pythia_->event;
  Pythia8::ParticleData& pdt = pythia_->particleData;

  // Use pythiaDecay information to fill event with the input particle
  fillParticle(aTrack, event);

  // Copy and paste of RHadron decay code
  int    iRNow  = 1;
  int    idRHad = event[iRNow].id();
  double mRHad  = event[iRNow].m();
  int    iR0    = 0;
  int    iR2    = 0;

  bool isTriplet = !isGluinoRHadron(idRHad);

  // Find flavour content of squark or gluino R-hadron.
  std::pair<int,int> idPair = (isTriplet) ? fromIdWithSquark( idRHad) : fromIdWithGluino( idRHad, &(pythia_->rndm));
  int id1 = idPair.first;
  int id2 = idPair.second;

  // Sharing of momentum: the squark/gluino should be restored
  // to original mass, but error if negative-mass spectators.
  int idRSb            = pythia_->settings.mode("RHadrons:idSbottom");
  int idRSt            = pythia_->settings.mode("RHadrons:idStop");
  int idRGo            = pythia_->settings.mode("RHadrons:idGluino");
  int idLight = (abs(idRHad) - 1000000) / 10;
  int idSq    = (idLight < 100) ? idLight/10 : idLight/100;
  int idRSq     = (idSq == 6) ? idRSt : idRSb;

  // Handling R-Hadrons with anti-squarks
  idRSq = idRSq * std::copysign(1, idRHad);

  int idRBef = isTriplet ? idRSq : idRGo;

  // Mass of the underlying sparticle
  double mRBef = pdt.mSel(idRBef);

  // Fraction of the RHadron mass given by the sparticle
  double fracR = mRBef / mRHad;
  int counter=0;
  while (fracR>=1.){
    if (counter==10){
      G4cout << "Needed more than 10 attempts with constituent " << idRBef << " mass (" << mRBef << " above R-Hadron " << idRHad << " mass " << mRHad << G4endl;
    } else if (counter>100){
      G4cout << "RHadronPythiaDecayer::decay ERROR   Failed >100 times. Constituent " << idRBef << " mass (" << mRBef << " above R-Hadron " << idRHad << " mass " << mRHad << G4endl;
      return;
    }
    mRBef = pdt.mSel(idRBef);
    fracR = mRBef / mRHad;
    counter++;
  }

  // Squark case
  if(isTriplet){
    const int col = (pdt.colType(idRBef) != 0) ? event.nextColTag() : 0; // NB There should be no way that this can be zero (see discussion on ATLASSIM-6687), but leaving check in there just in case something changes in the future.
    int tmpSparticleColor = id1>0 ? col : 0;
    int tmpSparticleAnticolor = id1>0 ? 0 : col;

    // Store the constituents of a squark R-hadron.

    // Sparticle
    // (id, status, mother1, mother2, daughter1, daughter2, col, acol, px, py, pz, e, m=0., scaleIn=0., polIn=9.)
    iR0 = event.append( id1, 106, iRNow, 0, 0, 0, tmpSparticleColor, tmpSparticleAnticolor, fracR * event[iRNow].p(), fracR * mRHad, 0.);
    // Spectator quark
    iR2 = event.append( id2, 106, iRNow, 0, 0, 0, tmpSparticleAnticolor, tmpSparticleColor, (1. - fracR) * event[iRNow].p(), (1. - fracR) * mRHad, 0.);
  }
  // Gluino case
  else{
    double mOffsetCloudRH = 0.2; // could be read from internal data?
    double m1Eff  = pdt.constituentMass(id1) + mOffsetCloudRH;
    double m2Eff  = pdt.constituentMass(id2) + mOffsetCloudRH;
    double frac1 = (1. - fracR) * m1Eff / ( m1Eff + m2Eff);
    double frac2 = (1. - fracR) * m2Eff / ( m1Eff + m2Eff);

    // Two new colours needed in the breakups.
    int col1 = event.nextColTag();
    int col2 = event.nextColTag();

    // Store the constituents of a gluino R-hadron.
    iR0 = event.append( idRBef, 106, iRNow, 0, 0, 0, col2, col1, fracR * event[iRNow].p(), fracR * mRHad, 0.);
    event.append( id1, 106, iRNow, 0, 0, 0, col1, 0, frac1 * event[iRNow].p(), frac1 * mRHad, 0.);
    iR2 = event.append( id2, 106, iRNow, 0, 0, 0, 0, col2, frac2 * event[iRNow].p(), frac2 * mRHad, 0.);
  }

  // Mark R-hadron as decayed and update history.
  event[iRNow].statusNeg();
  event[iRNow].daughters( iR0, iR2);

  // Generate events. Quit if failure.
  if (!pythia_->next()) {
    pythia_->forceRHadronDecays();
  }

  ///////////////////////////////////////////////////////////////////////////
  // Add the particles from the Pythia event into the GEANT particle vector
  ///////////////////////////////////////////////////////////////////////////
  particles.clear();

  for(int i=0; i<pythia_->event.size(); i++){
    if ( pythia_->event[i].status()<0 ) continue; // stable only
    G4ThreeVector momentum( pythia_->event[i].px() , pythia_->event[i].py() , pythia_->event[i].pz() );
    momentum*=1000.0;//GeV to MeV
    const G4ParticleDefinition * particleDefinition = GetParticleDefinition( pythia_->event[i].id() );

    if (!particleDefinition){
      G4cout << "I don't know a definition for pdgid "<<pythia_->event[i].id()<<"! Skipping it..." << G4endl;
      continue;
    }

    G4DynamicParticle* dynamicParticle = new G4DynamicParticle(particleDefinition, momentum);
    particles.push_back( dynamicParticle );
  }

}