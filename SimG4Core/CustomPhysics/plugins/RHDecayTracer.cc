#include "SimG4Core/CustomPhysics/interface/RHDecayTracer.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayDataManager.h"
#include "SimG4Core/Notification/interface/TrackInformation.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "HepMC/GenEvent.h"
#include "HepMC/GenVertex.h"
#include "HepMC/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

// Producer that adds R-hadron decay information from RHadronPythiaDecayer to the HepMC event
using TrackData = RHadronPythiaDecayDataManager::TrackData;

RHDecayTracer::RHDecayTracer(edm::ParameterSet const& p)
    : edm::one::EDProducer<edm::one::SharedResources>() 
{
  genToken_      = consumes<edm::HepMCProduct>(edm::InputTag("generator"));
  simTrackToken_ = consumes<edm::SimTrackContainer>(edm::InputTag("g4SimHits"));
}

void RHDecayTracer::produce(edm::Event& iEvent, const edm::EventSetup&) {
  // Get track data from RHadronPythiaDecayDataManager
  std::map<int, TrackData> decayParents;
  std::map<int, std::vector<TrackData>> decayDaughters;
  RHadronPythiaDecayDataManager::getInstance().getDecayInfo(decayParents, decayDaughters);

  // If no decays recorded, skip producer
  if (decayParents.empty()) return;

  // Get the HepMC event and SimTrack collection
  edm::Handle<edm::HepMCProduct> genHandle;
  edm::Handle<edm::SimTrackContainer> simTrackHandle;
  iEvent.getByToken(genToken_, genHandle);
  iEvent.getByToken(simTrackToken_, simTrackHandle);

  HepMC::GenEvent* mcEvent = const_cast<HepMC::GenEvent*>(genHandle->GetEvent());
  const edm::SimTrackContainer& simTracks = *simTrackHandle;

  // Loop over each decay parent and create a HepMC vertex with its daughters
  for (const auto& parentEntry : decayParents) {
    const int decayID = parentEntry.first;
    const TrackData& parentData = parentEntry.second;
    
    // Get the SimTrack associated with the parent
    const SimTrack* parentSimTrack = findSimTrack(parentData.trackID, simTracks);
    if (!parentSimTrack) continue; // Skip if SimTrack not found

    // Get the corresponding HepMC GenParticle of the parent
    HepMC::GenParticle* parentGenParticle = mcEvent->barcode_to_particle(parentSimTrack->genpartIndex());

    // Create a new HepMC vertex for the decay and asign the parent particle
    HepMC::GenVertex* decayVertex = new HepMC::GenVertex(HepMC::FourVector(parentData.x, parentData.y, parentData.z, parentData.time));
    decayVertex->add_particle_in(parentGenParticle);

    // Add daughter particles to the vertex
    addSecondariesToGenVertex(decayDaughters, decayID, decayVertex);

    // Mark the parent as decayed and add the vertex to the event
    parentGenParticle->set_status(2);
    mcEvent->add_vertex(decayVertex);
  }

  // Clear data for the next event
  RHadronPythiaDecayDataManager::getInstance().clearDecayInfo();
}


const SimTrack* RHDecayTracer::findSimTrack(int trackID, const edm::SimTrackContainer& simTracks) {
    for (const auto& simTrack : simTracks) {
        if (simTrack.trackId() == static_cast<unsigned int>(trackID)) return &simTrack;
    }
    return nullptr;
}


void RHDecayTracer::addSecondariesToGenVertex(std::map<int, std::vector<TrackData>> decayDaughters, const int decayID, HepMC::GenVertex* decayVertex) {
  const auto& daughtersData = decayDaughters[decayID];
  for (const auto& daughterData : daughtersData) {
      HepMC::GenParticle* daughter = new HepMC::GenParticle(HepMC::FourVector(1000.0 * daughterData.px, 1000.0 * daughterData.py, 1000.0 * daughterData.pz, 1000.0 * daughterData.energy), daughterData.pdgID, 1);
      decayVertex->add_particle_out(daughter);
  }
}