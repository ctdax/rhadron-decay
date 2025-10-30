#include "SimG4Core/CustomPhysics/interface/RHDecayTracer.h"
#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayDataManager.h"
#include "FWCore/Framework/interface/Event.h"

RHDecayTracer::RHDecayTracer(edm::ParameterSet const& p) {
  produces<std::vector<int> >("RHadronDecayID");
  produces<std::vector<float> >("RHadronDecayX");
  produces<std::vector<float> >("RHadronDecayY");
  produces<std::vector<float> >("RHadronDecayZ");
  produces<std::vector<float> >("RHadronDecayTime");
  produces<std::vector<float> >("RHadronDecayPx");
  produces<std::vector<float> >("RHadronDecayPy");
  produces<std::vector<float> >("RHadronDecayPz");
  produces<std::vector<float> >("RHadronDecayE");
}

void RHDecayTracer::produce(edm::Event& fEvent, const edm::EventSetup&) {
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "RHDecayTracer::produce->";
  // Get data from shared manager
  std::vector<int> ids;
  std::vector<float> xs, ys, zs, ts, pxs, pys, pzs, es;
  
  RHadronPythiaDecayDataManager::getInstance().getDecayInfo(ids, xs, ys, zs, ts, pxs, pys, pzs, es);
  
  // Create unique_ptrs and put into event
  std::unique_ptr<std::vector<int>> idsPtr(new std::vector<int>(ids));
  std::unique_ptr<std::vector<float>> xsPtr(new std::vector<float>(xs));
  std::unique_ptr<std::vector<float>> ysPtr(new std::vector<float>(ys));
  std::unique_ptr<std::vector<float>> zsPtr(new std::vector<float>(zs));
  std::unique_ptr<std::vector<float>> tsPtr(new std::vector<float>(ts));
  std::unique_ptr<std::vector<float>> pxsPtr(new std::vector<float>(pxs));
  std::unique_ptr<std::vector<float>> pysPtr(new std::vector<float>(pys));
  std::unique_ptr<std::vector<float>> pzsPtr(new std::vector<float>(pzs));
  std::unique_ptr<std::vector<float>> esPtr(new std::vector<float>(es));  

  fEvent.put(std::move(idsPtr), "RHadronDecayID");
  fEvent.put(std::move(xsPtr), "RHadronDecayX");
  fEvent.put(std::move(ysPtr), "RHadronDecayY");
  fEvent.put(std::move(zsPtr), "RHadronDecayZ");
  fEvent.put(std::move(tsPtr), "RHadronDecayTime");
  fEvent.put(std::move(pxsPtr), "RHadronDecayPx");
  fEvent.put(std::move(pysPtr), "RHadronDecayPy");
  fEvent.put(std::move(pzsPtr), "RHadronDecayPz");
  fEvent.put(std::move(esPtr), "RHadronDecayE");  

  // Clear data for the next event
  RHadronPythiaDecayDataManager::getInstance().clearDecayInfo();
}