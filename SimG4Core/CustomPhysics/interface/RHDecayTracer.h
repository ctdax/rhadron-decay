#ifndef SimG4Core_CustomPhysics_RHDecayTracer_H
#define SimG4Core_CustomPhysics_RHDecayTracer_H

#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayDataManager.h"
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "SimG4Core/Watcher/interface/SimProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class SimTrack;

namespace HepMC {
  class GenVertex;
}

namespace edm {
  class HepMCProduct;
}

class RHDecayTracer : public SimProducer,
                      public edm::one::EDProducer<edm::one::SharedResources> {
public:
  RHDecayTracer(edm::ParameterSet const& p);
  ~RHDecayTracer() noexcept override = default;
  void produce(edm::Event &, const edm::EventSetup &) override;

private:
  const SimTrack* findSimTrack(int trackID, const edm::SimTrackContainer& simTracks);
  void addSecondariesToGenVertex(std::map<int, std::vector<RHadronPythiaDecayDataManager::TrackData>> decayDaughters, const int decayID, HepMC::GenVertex* decayVertex);

  edm::EDGetTokenT<edm::HepMCProduct> genToken_;
  edm::EDGetTokenT<edm::SimTrackContainer> simTrackToken_;
};

#endif