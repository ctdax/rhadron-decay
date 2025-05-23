import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
#process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
#from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = "106X_mcRun3_2021_realistic_v3"

options = VarParsing('analysis')
options.parseArguments()

#process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(5000) )
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

outputFileName = options.outputFile.replace(".root", "")

process.demo = cms.EDAnalyzer("SpikedRHadronAnalyzer",

    outputFileName = cms.string(outputFileName),
    gen_info = cms.InputTag("genParticles","","SIM"),

    G4TrkSrc = cms.InputTag("g4SimHits"),
    G4VtxSrc = cms.InputTag("g4SimHits"),

    # Tracker
    TrackerHitsPixelBarrelLowTof = cms.InputTag("g4SimHits","TrackerHitsPixelBarrelLowTof"),
    TrackerHitsPixelBarrelHighTof = cms.InputTag("g4SimHits","TrackerHitsPixelBarrelHighTof"),
    TrackerHitsPixelEndcapLowTof = cms.InputTag("g4SimHits","TrackerHitsPixelEndcapLowTof"),
    TrackerHitsPixelEndcapHighTof = cms.InputTag("g4SimHits","TrackerHitsPixelEndcapHighTof"),
    TrackerHitsTIBLowTof = cms.InputTag("g4SimHits","TrackerHitsTIBLowTof"),
    TrackerHitsTIBHighTof = cms.InputTag("g4SimHits","TrackerHitsTIBHighTof"),
    TrackerHitsTOBLowTof = cms.InputTag("g4SimHits","TrackerHitsTOBLowTof"),
    TrackerHitsTOBHighTof = cms.InputTag("g4SimHits","TrackerHitsTOBHighTof"),
    TrackerHitsTECLowTof = cms.InputTag("g4SimHits","TrackerHitsTECLowTof"),
    TrackerHitsTECHighTof = cms.InputTag("g4SimHits","TrackerHitsTECHighTof"),
    TrackerHitsTIDLowTof = cms.InputTag("g4SimHits","TrackerHitsTIDLowTof"),
    TrackerHitsTIDHighTof = cms.InputTag("g4SimHits","TrackerHitsTIDHighTof"),

    # Calorimiter
    EcalHitsEB = cms.InputTag("g4SimHits","EcalHitsEB"),
    EcalHitsEE = cms.InputTag("g4SimHits","EcalHitsEE"),
    EcalHitsES = cms.InputTag("g4SimHits","EcalHitsES"),
    HcalHits = cms.InputTag("g4SimHits","HcalHits"),

    # Muon
    MuonCSCHits = cms.InputTag("g4SimHits","MuonCSCHits"),
    MuonDTHits = cms.InputTag("g4SimHits","MuonDTHits"),
    MuonRPCHits = cms.InputTag("g4SimHits","MuonRPCHits"),
    MuonGEMHits = cms.InputTag("g4SimHits","MuonGEMHits"),
)

process.p = cms.Path(process.demo)
