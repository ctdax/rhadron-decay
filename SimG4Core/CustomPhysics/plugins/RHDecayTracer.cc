#include "SimG4Core/CustomPhysics/interface/RHDecayTracer.h"
#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayDataManager.h"
#include "FWCore/Framework/interface/Event.h"
#include "HepMC/GenEvent.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

RHDecayTracer::RHDecayTracer(edm::ParameterSet const& p) {
  genToken_ = consumes<edm::HepMCProduct>(edm::InputTag("generator"));
}

void RHDecayTracer::produce(edm::Event& iEvent, const edm::EventSetup&) {
  // Get data from shared manager
  edm::Handle<edm::HepMCProduct> genHandle;
  iEvent.getByToken(genToken_, genHandle);
  const HepMC::GenEvent* mcEvent = genHandle->GetEvent();
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

  iEvent.put(std::move(idsPtr), "RHadronDecayID");
  iEvent.put(std::move(xsPtr), "RHadronDecayX");
  iEvent.put(std::move(ysPtr), "RHadronDecayY");
  iEvent.put(std::move(zsPtr), "RHadronDecayZ");
  iEvent.put(std::move(tsPtr), "RHadronDecayTime");
  iEvent.put(std::move(pxsPtr), "RHadronDecayPx");
  iEvent.put(std::move(pysPtr), "RHadronDecayPy");
  iEvent.put(std::move(pzsPtr), "RHadronDecayPz");
  iEvent.put(std::move(esPtr), "RHadronDecayE");  

  // Clear data for the next event
  RHadronPythiaDecayDataManager::getInstance().clearDecayInfo();
}