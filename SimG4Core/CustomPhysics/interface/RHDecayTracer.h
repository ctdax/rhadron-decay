#ifndef SimG4Core_CustomPhysics_RHDecayTracer_H
#define SimG4Core_CustomPhysics_RHDecayTracer_H

#include "SimG4Core/Watcher/interface/SimProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

namespace edm {
  class HepMCProduct;
}

class RHDecayTracer : public SimProducer {
public:
  RHDecayTracer(edm::ParameterSet const& p);
  ~RHDecayTracer() override = default;
  void produce(edm::Event &, const edm::EventSetup &) override;

private:
  edm::EDGetTokenT<edm::HepMCProduct> genToken_;
};

#endif