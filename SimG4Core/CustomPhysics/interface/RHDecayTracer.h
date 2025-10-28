#ifndef SimG4Core_CustomPhysics_RHDecayTracer_H
#define SimG4Core_CustomPhysics_RHDecayTracer_H

#include "SimG4Core/Watcher/interface/SimProducer.h"
#include "SimG4Core/Notification/interface/Observer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class RHDecayTracer : public SimProducer {
public:
  RHDecayTracer(edm::ParameterSet const& p);
  ~RHDecayTracer() override = default;
  void produce(edm::Event &, const edm::EventSetup &) override;
};

#endif