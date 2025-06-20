// -*- C++ -*-
//
// Package:    SpikedRHadronAnalyzer
// Class:      SpikedRHadronAnalyzer
//
/**\class SpikedRHadronAnalyzer src/SpikedRHadronAnalyzer.cc

 Description: [Reads SIM ROOTs with gluino samples and outputs histograms for: calorimiter hit location, 4 momenta of R-Hadrons 1 and 2, ...]

 Implementation:
     [Github repository: https://github.com/ctdax/SUSYBSMAnalysis]
*/
//
// Original Author:  Colby Thompson
//         Created:  Thu, 22 Feb 2024 20:45:33 GMT
//
//


//System include files
#include <memory>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

//Triggers and Handles
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
//#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

//Tracker
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "SimDataFormats/Track/interface/CoreSimTrack.h"
#include "SimDataFormats/Track/interface/SimTrack.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/CoreSimVertex.h"

//ECAL
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

//HCAL
#include "Geometry/Records/interface/HcalGeometryRecord.h"
#include "Geometry/HcalTowerAlgo/interface/HcalGeometry.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"
#include "Geometry/CaloTopology/interface/CaloSubdetectorTopology.h"
#include "Geometry/CaloTopology/interface/HcalTopology.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/HcalCommonData/interface/HcalDDDRecConstants.h"
#include "Geometry/HcalCommonData/interface/HcalHitRelabeller.h"
#include "Geometry/Records/interface/HcalRecNumberingRecord.h"
#include "CondFormats/HcalObjects/interface/HcalRespCorrs.h"
#include "CondFormats/DataRecord/interface/HcalRespCorrsRcd.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"

//Muon Chamber
#include "Geometry/Records/interface/MuonGeometryRcd.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/DTGeometry/interface/DTLayer.h"
#include "Geometry/DTGeometry/interface/DTChamber.h"
#include "Geometry/DTGeometry/interface/DTSuperLayer.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"

//Calculations
#include "DataFormats/Common/interface/HLTPathStatus.h"
#include "DataFormats/Common/interface/HLTenums.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/Vector3D.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/METReco/interface/MET.h"
#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"

//ROOT
#include "TFile.h"
#include "TH1.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TString.h"
#include "TCanvas.h"
#include "TEfficiency.h"
#include "TStyle.h"
#include "TObject.h"
#include "TDirectory.h"
#include "TMatrix.h"
#include "TRandom3.h"
#include "TVector3.h"
#include "TGraph.h"

//FWCORE
#define FWCORE

using namespace edm;
using namespace reco;
using namespace std;
using namespace __gnu_cxx;
using namespace trigger;

int evtcount = 0;

class TupleMaker;
class MCWeight;

class SpikedRHadronAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit SpikedRHadronAnalyzer (const edm::ParameterSet&);
  ~SpikedRHadronAnalyzer();


private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<vector<reco::GenParticle>> genParticlesToken_;
  std::string outputFileName;

  // Tracks and Vertices
  edm::EDGetTokenT<edm::SimTrackContainer> edmSimTrackContainerToken_;
  edm::EDGetTokenT<edm::SimVertexContainer> edmSimVertexContainerToken_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmSimHitContainerToken_;

  // Tracker hits
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> tkGeometryToken_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTIBLow_Token_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTIBHigh_Token_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTOBLow_Token_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTOBHigh_Token_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTIDLow_Token_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTIDHigh_Token_;
  edm::EDGetTokenT<edm::PSimHitContainer> edmPSimHitContainer_siTECLow_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_siTECHigh_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_pxlBrlLow_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_pxlBrlHigh_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_pxlFwdLow_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_pxlFwdHigh_Token_;

  // ECAL hits
  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeometryToken_;
  edm::EDGetTokenT <edm::PCaloHitContainer> edmCaloHitContainer_EcalHitsEB_Token_;
  edm::EDGetTokenT <edm::PCaloHitContainer> edmCaloHitContainer_EcalHitsEE_Token_;
  edm::EDGetTokenT <edm::PCaloHitContainer> edmCaloHitContainer_EcalHitsES_Token_;
  edm::EDGetTokenT <edm::PCaloHitContainer> edmCaloHitContainer_HcalHits_Token_;

  // HCAL hits
  /*
  const HcalDDDRecConstants *hcons_         ;
  const CaloGeometry        *geometry_      ;
  const CaloTopology        *topology_      ;
  const HcalTopology        *hcalTopology_  ;
  const HcalRespCorrs       *respCorrs_     ;
  HcalRespCorrs             *respCorrs      ;

  const edm::ESGetToken< HcalDDDRecConstants, HcalRecNumberingRecord > tok_HRNDC_        ;
  const edm::ESGetToken< CaloTopology,        CaloTopologyRecord     > tok_caloTopology_ ;
  const edm::ESGetToken< HcalTopology,        HcalRecNumberingRecord > tok_hcalTopology_ ;
  const edm::ESGetToken< HcalRespCorrs,       HcalRespCorrsRcd       > tok_resp_         ;
  */

  // Muon hits
  edm::ESGetToken<CSCGeometry, MuonGeometryRecord> cscGeometryToken_;
  edm::ESGetToken<DTGeometry, MuonGeometryRecord> dtGeometryToken_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> gemGeometryToken_;
  edm::ESGetToken<RPCGeometry, MuonGeometryRecord> rpcGeometryToken_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_muonCSC_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_muonDT_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_muonRPC_Token_;
  edm::EDGetTokenT <edm::PSimHitContainer> edmPSimHitContainer_muonGEM_Token_;

  std::ofstream csv;
};

//constructor
SpikedRHadronAnalyzer::SpikedRHadronAnalyzer(const edm::ParameterSet& iConfig) {

  outputFileName = iConfig.getParameter<std::string>("outputFileName");
  edmSimTrackContainerToken_ = consumes<edm::SimTrackContainer>(iConfig.getParameter<edm::InputTag>("G4TrkSrc"));
  edmSimVertexContainerToken_ = consumes<edm::SimVertexContainer>(iConfig.getParameter<edm::InputTag>("G4VtxSrc"));

  // Tracker hits   
  tkGeometryToken_ = esConsumes<TrackerGeometry, TrackerDigiGeometryRecord>();
  edmPSimHitContainer_siTIBLow_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTIBLowTof"));
  edmPSimHitContainer_siTIBHigh_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTIBHighTof"));
  edmPSimHitContainer_siTOBLow_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTOBLowTof"));
  edmPSimHitContainer_siTOBHigh_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTOBHighTof"));
  edmPSimHitContainer_siTIDLow_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTIDLowTof"));
  edmPSimHitContainer_siTIDHigh_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTIDHighTof"));
  edmPSimHitContainer_siTECLow_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTECLowTof"));
  edmPSimHitContainer_siTECHigh_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsTECHighTof"));
  edmPSimHitContainer_pxlBrlLow_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsPixelBarrelLowTof"));
  edmPSimHitContainer_pxlBrlHigh_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsPixelBarrelHighTof"));
  edmPSimHitContainer_pxlFwdLow_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsPixelEndcapLowTof"));
  edmPSimHitContainer_pxlFwdHigh_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("TrackerHitsPixelEndcapHighTof"));

  // Calorimiter
  caloGeometryToken_ = esConsumes<CaloGeometry, CaloGeometryRecord>();
  edmCaloHitContainer_EcalHitsEB_Token_ = consumes<edm::PCaloHitContainer>(iConfig.getParameter<edm::InputTag>("EcalHitsEB"));
  edmCaloHitContainer_EcalHitsEE_Token_ = consumes<edm::PCaloHitContainer>(iConfig.getParameter<edm::InputTag>("EcalHitsEE"));
  edmCaloHitContainer_EcalHitsES_Token_ = consumes<edm::PCaloHitContainer>(iConfig.getParameter<edm::InputTag>("EcalHitsES"));
  edmCaloHitContainer_HcalHits_Token_ = consumes<edm::PCaloHitContainer>(iConfig.getParameter<edm::InputTag>("HcalHits"));

  // Muon Chamber
  cscGeometryToken_ = esConsumes<CSCGeometry, MuonGeometryRecord>();
  dtGeometryToken_ = esConsumes<DTGeometry, MuonGeometryRecord>();
  gemGeometryToken_ = esConsumes<GEMGeometry, MuonGeometryRecord>();
  rpcGeometryToken_ = esConsumes<RPCGeometry, MuonGeometryRecord>();
  edmPSimHitContainer_muonCSC_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("MuonCSCHits"));
  edmPSimHitContainer_muonDT_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("MuonDTHits"));
  edmPSimHitContainer_muonRPC_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("MuonRPCHits"));
  edmPSimHitContainer_muonGEM_Token_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("MuonGEMHits"));

  // Create csv for energy spike R-hadron analysis
  csv.open (outputFileName);
  csv << "Event,Energy Deposit,isHCAL,x [cm],y [cm],z [cm],r [cm],PDG,TrackID,Track Energy,px,py,pz,primaryVertexX,primaryVertexY,primaryVertexZ\n";

  evtcount = 0;
}


//destructor
SpikedRHadronAnalyzer::~SpikedRHadronAnalyzer() {
  csv.close();
}

void SpikedRHadronAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

  edm::SimTrackContainer::const_iterator simTrack;
  edm::SimVertexContainer::const_iterator simVertex;
  evtcount++;

  // Tracker Containers
   edm::Handle<edm::PSimHitContainer> SiTIBLowContainer;
   iEvent.getByToken(edmPSimHitContainer_siTIBLow_Token_, SiTIBLowContainer);
   if (!SiTIBLowContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTIBLowTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTIBHighContainer;
   iEvent.getByToken(edmPSimHitContainer_siTIBHigh_Token_, SiTIBHighContainer);
   if (!SiTIBHighContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTIBHighTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTOBLowContainer;
   iEvent.getByToken(edmPSimHitContainer_siTOBLow_Token_, SiTOBLowContainer);
   if (!SiTOBLowContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTOBLowTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTOBHighContainer;
   iEvent.getByToken(edmPSimHitContainer_siTOBHigh_Token_, SiTOBHighContainer);
   if (!SiTOBHighContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTOBHighTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTIDLowContainer;
   iEvent.getByToken(edmPSimHitContainer_siTIDLow_Token_, SiTIDLowContainer);
   if (!SiTIDLowContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTIDLowTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTIDHighContainer;
   iEvent.getByToken(edmPSimHitContainer_siTIDHigh_Token_, SiTIDHighContainer);
   if (!SiTIDHighContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTIDHighTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTECLowContainer;
   iEvent.getByToken(edmPSimHitContainer_siTECLow_Token_, SiTECLowContainer);
   if (!SiTECLowContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTECLowTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> SiTECHighContainer;
   iEvent.getByToken(edmPSimHitContainer_siTECHigh_Token_, SiTECHighContainer);
   if (!SiTECHighContainer.isValid()) {
     edm::LogError("TrackerHitProducer::analyze") << "Unable to find TrackerHitsTECHighTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> PxlBrlLowContainer;
   iEvent.getByToken(edmPSimHitContainer_pxlBrlLow_Token_, PxlBrlLowContainer);
   if (!PxlBrlLowContainer.isValid()) {
     edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find TrackerHitsPixelBarrelLowTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> PxlBrlHighContainer;
   iEvent.getByToken(edmPSimHitContainer_pxlBrlHigh_Token_, PxlBrlHighContainer);
   if (!PxlBrlHighContainer.isValid()) {
     edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find TrackerHitsPixelBarrelHighTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> PxlFwdLowContainer;
   iEvent.getByToken(edmPSimHitContainer_pxlFwdLow_Token_, PxlFwdLowContainer);
   if (!PxlFwdLowContainer.isValid()) {
     edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find TrackerHitsPixelEndcapLowTof in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> PxlFwdHighContainer;
   iEvent.getByToken(edmPSimHitContainer_pxlFwdHigh_Token_, PxlFwdHighContainer);
   if (!PxlFwdHighContainer.isValid()) {
     edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find TrackerHitsPixelEndcapHighTof in event!";
     return;
   }

    // Calorimiter Containers
    edm::Handle<edm::PCaloHitContainer> EcalEBContainer;
    iEvent.getByToken(edmCaloHitContainer_EcalHitsEB_Token_, EcalEBContainer);
    if (!EcalEBContainer.isValid()) {
        edm::LogError("CaloHitProducer::analyze") << "Unable to find EcalHitsEB in event!";
        return;
   }

    edm::Handle<edm::PCaloHitContainer> EcalEEContainer;
    iEvent.getByToken(edmCaloHitContainer_EcalHitsEE_Token_, EcalEEContainer);
    if (!EcalEEContainer.isValid()) {
        edm::LogError("CaloHitProducer::analyze") << "Unable to find EcalHitsEE in event!";
        return;
   }

    edm::Handle<edm::PCaloHitContainer> EcalESContainer;
    iEvent.getByToken(edmCaloHitContainer_EcalHitsES_Token_, EcalESContainer);
    if (!EcalESContainer.isValid()) {
        edm::LogError("CaloHitProducer::analyze") << "Unable to find EcalHitsES in event!";
        return;
   }
  
    edm::Handle<edm::PCaloHitContainer> HcalContainer;
    iEvent.getByToken(edmCaloHitContainer_HcalHits_Token_, HcalContainer);
    if (!HcalContainer.isValid()) {
        edm::LogError("CaloHitProducer::analyze") << "Unable to find HcalHits in event!";
        return;
   }

   // Muon Chamber Containers
   edm::Handle<edm::PSimHitContainer> MuonDTContainer;
   iEvent.getByToken(edmPSimHitContainer_muonDT_Token_, MuonDTContainer);
   if (!MuonDTContainer.isValid()) {
     edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find MuonDT simhits in event!";
     return;
   }

   edm::Handle<edm::PSimHitContainer> MuonCSCContainer;
   iEvent.getByToken(edmPSimHitContainer_muonCSC_Token_, MuonCSCContainer);
   if (!MuonCSCContainer.isValid()) {
     edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find MuonCSC simhits in event!";
     return;
   }

    edm::Handle<edm::PSimHitContainer> MuonRPCContainer;
    iEvent.getByToken(edmPSimHitContainer_muonRPC_Token_, MuonRPCContainer);
    if (!MuonRPCContainer.isValid()) {
        edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find MuonRPC simhits in event!";
        return;
    }

    edm::Handle<edm::PSimHitContainer> MuonGEMContainer;
    iEvent.getByToken(edmPSimHitContainer_muonGEM_Token_, MuonGEMContainer);
    if (!MuonGEMContainer.isValid()) {
        edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find MuonGEM simhits in event!";
        return;
    }

  // Get G4SimTracks
  edm::Handle<edm::SimTrackContainer> G4TrkContainer;
  iEvent.getByToken(edmSimTrackContainerToken_, G4TrkContainer);
  if (!G4TrkContainer.isValid()) {
    edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find SimTrack in event!";
    return;
  }

  // Get G4SimVertices
  edm::Handle<edm::SimVertexContainer> G4VtxContainer;
  iEvent.getByToken(edmSimVertexContainerToken_, G4VtxContainer);
  if (!G4VtxContainer.isValid()) {
    edm::LogError("TrackerHitAnalyzer::analyze") << "Unable to find SimVertex in event!";
    return;
  }

  // Combine tracker simhit containers
  edm::PSimHitContainer G4SimHitContainer;
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTIBLowContainer->begin(), SiTIBLowContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTIBHighContainer->begin(), SiTIBHighContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTOBLowContainer->begin(), SiTOBLowContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTOBHighContainer->begin(), SiTOBHighContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTIDLowContainer->begin(), SiTIDLowContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTIDHighContainer->begin(), SiTIDHighContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTECLowContainer->begin(), SiTECLowContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), SiTECHighContainer->begin(), SiTECHighContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), PxlBrlLowContainer->begin(), PxlBrlLowContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), PxlBrlHighContainer->begin(), PxlBrlHighContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), PxlFwdLowContainer->begin(), PxlFwdLowContainer->end());
  G4SimHitContainer.insert(G4SimHitContainer.end(), PxlFwdHighContainer->begin(), PxlFwdHighContainer->end());
  
  // Combine ECAL calohit containers
  edm::PCaloHitContainer G4CaloHitContainer;
  G4CaloHitContainer.insert(G4CaloHitContainer.end(), EcalEBContainer->begin(), EcalEBContainer->end());
  G4CaloHitContainer.insert(G4CaloHitContainer.end(), EcalEEContainer->begin(), EcalEEContainer->end());
  G4CaloHitContainer.insert(G4CaloHitContainer.end(), EcalESContainer->begin(), EcalESContainer->end());

  // Combine muon chamber simhit containers
  edm::PSimHitContainer G4MuonContainer;
  G4MuonContainer.insert(G4MuonContainer.end(), MuonDTContainer->begin(), MuonDTContainer->end());
  G4MuonContainer.insert(G4MuonContainer.end(), MuonCSCContainer->begin(), MuonCSCContainer->end());
  G4MuonContainer.insert(G4MuonContainer.end(), MuonRPCContainer->begin(), MuonRPCContainer->end());
  G4MuonContainer.insert(G4MuonContainer.end(), MuonGEMContainer->begin(), MuonGEMContainer->end());

  // Grab geometries
  const CaloGeometry* caloGeometry = &iSetup.getData(caloGeometryToken_);
  const TrackerGeometry* tkGeometry = &iSetup.getData(tkGeometryToken_);
  const CSCGeometry* cscGeometry = &iSetup.getData(cscGeometryToken_);
  const DTGeometry* dtGeometry = &iSetup.getData(dtGeometryToken_);
  const GEMGeometry* gemGeometry = &iSetup.getData(gemGeometryToken_);
  const RPCGeometry* rpcGeometry = &iSetup.getData(rpcGeometryToken_);

  // Begin loop over tracker sim hits
  for (auto simHit = G4SimHitContainer.begin(); simHit != G4SimHitContainer.end(); ++simHit) {
    // Get the energy deposited
    double energyDeposit = simHit->energyLoss();

    // Get the location (r and z) of the hit
    DetId detId = DetId(simHit->detUnitId());
    try {
      const GeomDetUnit *det = (const GeomDetUnit *)tkGeometry->idToDetUnit(detId);
      GlobalPoint globalPosition = det->toGlobal(simHit->localPosition());
      double x = globalPosition.x();
      double y = globalPosition.y();
      double z = globalPosition.z();
      double r = sqrt(globalPosition.x() * globalPosition.x() + globalPosition.y() * globalPosition.y());

      // Find the corresponding SimTrack
      auto trackId = simHit->trackId();
      auto simTrack = std::find_if(G4TrkContainer->begin(), G4TrkContainer->end(), [trackId](const SimTrack& track) {
        return static_cast<decltype(trackId)>(track.trackId()) == trackId;
      });

      if (simTrack != G4TrkContainer->end()) {
        // Get the particle type that caused the hit
        int particleType = simTrack->type();

        // Get the momentum of the particle that caused the hit
        auto momentum = simTrack->momentum();

        // Get the primary vertex position
        auto vertexId = simTrack->vertIndex();
        GlobalPoint primaryVertex(0, 0, 0);
        if (vertexId >= 0 && vertexId < static_cast<int>(G4VtxContainer->size())) {
          const SimVertex& vertex = G4VtxContainer->at(vertexId);
          primaryVertex = GlobalPoint(vertex.position().x(), vertex.position().y(), vertex.position().z());
        } else {
          edm::LogWarning("TrackerHitAnalyzer::analyze") << "Invalid vertex index: " << vertexId;
        }

        // Log the information
        csv << evtcount << "," << energyDeposit << "," << 0 << "," << x << "," << y << "," << z << "," << r << "," << particleType << ',' << trackId << "," << momentum.E() << ',' << momentum.Px() << ',' << momentum.Py() << ',' << momentum.Pz() << ',' << primaryVertex.x() << ',' << primaryVertex.y() << ',' << primaryVertex.z() << '\n';
      }
    } catch (const cms::Exception& e) {
      edm::LogError("TrackerHitAnalyzer::analyze") << "Invalid DetID: " << e.what();
      continue;
    }
    const GeomDetUnit *det = (const GeomDetUnit *)tkGeometry->idToDetUnit(detId);
    GlobalPoint globalPosition = det->toGlobal(simHit->localPosition());
    double x = globalPosition.x();
    double y = globalPosition.y();
    double z = globalPosition.z();
    double r = sqrt(globalPosition.x() * globalPosition.x() + globalPosition.y() * globalPosition.y());

    // Find the corresponding SimTrack
    auto trackId = simHit->trackId();
    auto simTrack = std::find_if(G4TrkContainer->begin(), G4TrkContainer->end(), [trackId](const SimTrack& track) {
      return static_cast<decltype(trackId)>(track.trackId()) == trackId;
    });

    if (simTrack != G4TrkContainer->end()) {
      // Get the particle type that caused the hit
      int particleType = simTrack->type();

      // Get the momentum of the particle that caused the hit
      auto momentum = simTrack->momentum();

      // Get the primary vertex position
        auto vertexId = simTrack->vertIndex();
        GlobalPoint primaryVertex(0, 0, 0);
        if (vertexId >= 0 && vertexId < static_cast<int>(G4VtxContainer->size())) {
          const SimVertex& vertex = G4VtxContainer->at(vertexId);
          primaryVertex = GlobalPoint(vertex.position().x(), vertex.position().y(), vertex.position().z());
        } else {
          edm::LogWarning("TrackerHitAnalyzer::analyze") << "Invalid vertex index: " << vertexId;
        }

      // Log the information
      csv << evtcount << "," << energyDeposit << "," << 0 << "," << x << "," << y << "," << z << "," << r << "," << particleType << ',' << trackId << "," << momentum.E() << ',' << momentum.Px() << ',' << momentum.Py() << ',' << momentum.Pz() << ',' << primaryVertex.x() << ',' << primaryVertex.y() << ',' << primaryVertex.z() << '\n';
    }
  }

  // Begin loop over calo hits
  for (auto caloHit = G4CaloHitContainer.begin(); caloHit != G4CaloHitContainer.end(); ++caloHit) {
    // Get the energy deposited
    double energyDeposit = caloHit->energy();

    // Get the location (r and z) of the hit
    DetId detId = DetId(caloHit->id());
    const GlobalPoint globalPosition = caloGeometry->getPosition(detId);
    double x = globalPosition.x();
    double y = globalPosition.y();
    double z = globalPosition.z();
    double r = sqrt(globalPosition.x() * globalPosition.x() + globalPosition.y() * globalPosition.y());

    // Find the corresponding SimTrack
    auto trackId = caloHit->geantTrackId();
    auto simTrack = std::find_if(G4TrkContainer->begin(), G4TrkContainer->end(), [trackId](const SimTrack& track) {
      return static_cast<decltype(trackId)>(track.trackId()) == trackId;
    });

    if (simTrack != G4TrkContainer->end()) {
      // Get the particle type that caused the hit
      int particleType = simTrack->type();

      // Get the momentum of the particle that caused the hit
      auto momentum = simTrack->momentum();

      // Get the primary vertex position
        auto vertexId = simTrack->vertIndex();
        GlobalPoint primaryVertex(0, 0, 0);
        if (vertexId >= 0 && vertexId < static_cast<int>(G4VtxContainer->size())) {
          const SimVertex& vertex = G4VtxContainer->at(vertexId);
          primaryVertex = GlobalPoint(vertex.position().x(), vertex.position().y(), vertex.position().z());
        } else {
          edm::LogWarning("TrackerHitAnalyzer::analyze") << "Invalid vertex index: " << vertexId;
        }

      // Log the information
      csv << evtcount << "," << energyDeposit << "," << 0 << "," << x << "," << y << "," << z << "," << r << "," << particleType << ',' << trackId << "," << momentum.E() << ',' << momentum.Px() << ',' << momentum.Py() << ',' << momentum.Pz() << ',' << primaryVertex.x() << ',' << primaryVertex.y() << ',' << primaryVertex.z() << '\n';
    }
  }

  
  // Begin loop over HCAL hits
  for (auto caloHit = HcalContainer->begin(); caloHit != HcalContainer->end(); ++caloHit) {
    // Get the energy deposited
    double energyDeposit = caloHit->energy();

    // Find the corresponding SimTrack
    auto trackId = caloHit->geantTrackId();
    auto simTrack = std::find_if(G4TrkContainer->begin(), G4TrkContainer->end(), [trackId](const SimTrack& track) {
      return static_cast<decltype(trackId)>(track.trackId()) == trackId;
    });

    if (simTrack != G4TrkContainer->end()) {
      // Get the particle type that caused the hit
      int particleType = simTrack->type();

      // Get the momentum of the particle that caused the hit
      auto momentum = simTrack->momentum();

      // Get the primary vertex position
        auto vertexId = simTrack->vertIndex();
        GlobalPoint primaryVertex(0, 0, 0);
        if (vertexId >= 0 && vertexId < static_cast<int>(G4VtxContainer->size())) {
          const SimVertex& vertex = G4VtxContainer->at(vertexId);
          primaryVertex = GlobalPoint(vertex.position().x(), vertex.position().y(), vertex.position().z());
        } else {
          edm::LogWarning("TrackerHitAnalyzer::analyze") << "Invalid vertex index: " << vertexId;
        }

      // Log the information
      csv << evtcount << "," << energyDeposit << "," << 1 << "," << 0 << "," << 0 << "," << 0 << "," << 0 << "," << particleType << ',' << trackId << "," << momentum.E() << ',' << momentum.Px() << ',' << momentum.Py() << ',' << momentum.Pz() << ',' << primaryVertex.x() << ',' << primaryVertex.y() << ',' << primaryVertex.z() << '\n';
    }
  }
  

  // Begin loop over muon chamber sim hits
  for (auto muonHit = G4MuonContainer.begin(); muonHit != G4MuonContainer.end(); ++muonHit) {
    // Get the energy deposited
    double energyDeposit = muonHit->energyLoss();

    // Get the location (r and z) of the hit
    DetId detId = DetId(muonHit->detUnitId());
    GlobalPoint globalPosition;

    // Determine the subdetector and get the global position
    if (detId.subdetId() == MuonSubdetId::CSC) {
      const GeomDetUnit *det = (const GeomDetUnit *)cscGeometry->idToDetUnit(detId);
      globalPosition = det->toGlobal(muonHit->localPosition());
    } else if (detId.subdetId() == MuonSubdetId::DT) {
      const GeomDetUnit *det = (const GeomDetUnit *)dtGeometry->idToDetUnit(detId);
      globalPosition = det->toGlobal(muonHit->localPosition());
    } else if (detId.subdetId() == MuonSubdetId::RPC) {
      const GeomDetUnit *det = (const GeomDetUnit *)rpcGeometry->idToDetUnit(detId);
      globalPosition = det->toGlobal(muonHit->localPosition());
    } else if (detId.subdetId() == MuonSubdetId::GEM) {
      try {
        const GeomDetUnit *det = (const GeomDetUnit *)gemGeometry->idToDetUnit(detId);
        globalPosition = det->toGlobal(muonHit->localPosition());
      } 
      catch (const cms::Exception& e) {
        edm::LogError("MuonHitAnalyzer::analyze") << "Invalid DetID: " << e.what();
        continue;
      }
      const GeomDetUnit *det = (const GeomDetUnit *)gemGeometry->idToDetUnit(detId);
      globalPosition = det->toGlobal(muonHit->localPosition());
    } else continue;

    double x = globalPosition.x();
    double y = globalPosition.y();
    double z = globalPosition.z();
    double r = sqrt(globalPosition.x() * globalPosition.x() + globalPosition.y() * globalPosition.y());

    // Find the corresponding SimTrack
    auto trackId = muonHit->trackId();
    auto simTrack = std::find_if(G4TrkContainer->begin(), G4TrkContainer->end(), [trackId](const SimTrack& track) {
      return static_cast<decltype(trackId)>(track.trackId()) == trackId;
    });

    if (simTrack != G4TrkContainer->end()) {
      // Get the particle type that caused the hit
      int particleType = simTrack->type();

      // Get the momentum of the particle that caused the hit
      auto momentum = simTrack->momentum();

      // Get the primary vertex position
        auto vertexId = simTrack->vertIndex();
        GlobalPoint primaryVertex(0, 0, 0);
        if (vertexId >= 0 && vertexId < static_cast<int>(G4VtxContainer->size())) {
          const SimVertex& vertex = G4VtxContainer->at(vertexId);
          primaryVertex = GlobalPoint(vertex.position().x(), vertex.position().y(), vertex.position().z());
        } else {
          edm::LogWarning("TrackerHitAnalyzer::analyze") << "Invalid vertex index: " << vertexId;
        }

      // Log the information
      csv << evtcount << "," << energyDeposit << "," << 0 << "," << x << "," << y << "," << z << "," << r << "," << particleType << ',' << trackId << "," << momentum.E() << ',' << momentum.Px() << ',' << momentum.Py() << ',' << momentum.Pz() << ',' << primaryVertex.x() << ',' << primaryVertex.y() << ',' << primaryVertex.z() << '\n';
    }
  }
}
//define this as a plug-in
DEFINE_FWK_MODULE(SpikedRHadronAnalyzer);