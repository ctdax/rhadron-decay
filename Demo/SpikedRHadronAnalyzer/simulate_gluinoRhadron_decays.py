# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: Configuration/GenProduction/python/EXO-RunIISummer20UL18GENSIM-00010-fragment.py --python_filename EXO-RunIISummer20UL18GENSIM-00010_1_cfg.py --eventcontent RAWSIM --customise SimG4Core/CustomPhysics/Exotica_HSCP_SIM_cfi.customise,Configuration/DataProcessing/Utils.addMonitoring --datatier GEN-SIM --fileout file:EXO-RunIISummer20UL18GENSIM-00010.root --conditions 106X_upgrade2018_realistic_v4 --beamspot Realistic25ns13TeVEarly2018Collision --step GEN,SIM --geometry DB:Extended --era Run2_2018 --no_exec --mc
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.Eras.Era_Run2_2018_cff import Run2_2018

options = VarParsing('analysis')
process = cms.Process('SIM',Run2_2018)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.GeometrySimDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedRealistic25ns13TeVEarly2018Collision_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

options.register('mass', 1800,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "Mass of the gluino"
)

options.parseArguments()
outputFile = options.outputFile
stringToReplace = "_numEvent{}".format(options.maxEvents)
outputFile = outputFile.replace(stringToReplace, '')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

# Handle seeding
process.load("IOMC.RandomEngine.IOMC_cff")
process.RandomNumberGeneratorService.generator.initialSeed = 123456789
process.RandomNumberGeneratorService.g4SimHits.initialSeed = 67890
process.RandomNumberGeneratorService.VtxSmeared.initialSeed = 345678
process.rndmStore = cms.EDProducer("RandomEngineStateProducer")

# Input source
process.source = cms.Source("EmptySource")

process.options = cms.untracked.PSet(

)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('Configuration/GenProduction/python/EXO-RunIISummer20UL18GENSIM-00010-fragment.py nevts:1'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.RAWSIMoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('generation_step')
    ),
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(1),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(20971520),
    fileName = cms.untracked.string(outputFile),
    outputCommands = process.RAWSIMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition

# Other statements
process.XMLFromDBSource.label = cms.string("Extended")
process.genstepfilter.triggerConditions=cms.vstring("generation_step")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '106X_upgrade2018_realistic_v4', '')

process.dirhadrongenfilter = cms.EDFilter("MCParticlePairFilter",
    MaxEta = cms.untracked.vdouble(100.0, 100.0),
    MinEta = cms.untracked.vdouble(-100, -100),
    MinP = cms.untracked.vdouble(0.0, 0.0),
    MinPt = cms.untracked.vdouble(0.0, 0.0),
    ParticleCharge = cms.untracked.int32(0),
    ParticleID1 = cms.untracked.vint32( 
        1000993, 1009213, 1009313, 1009323, 1009113, 
        1009223, 1009333, 1091114, 1092114, 1092214, 
        1092224, 1093114, 1093214, 1093224, 1093314, 
        1093324, 1093334
    ),
    ParticleID2 = cms.untracked.vint32(
        1000993, 1009213, 1009313, 1009323, 1009113, 
        1009223, 1009333, 1091114, 1092114, 1092214, 
        1092224, 1093114, 1093214, 1093224, 1093314, 
        1093324, 1093334
    ),
    Status = cms.untracked.vint32(1, 1)
)


process.generator = cms.EDFilter("Pythia8ConcurrentGeneratorFilter",
    PythiaParameters = cms.PSet(
        parameterSets = cms.vstring(
            'pythia8CommonSettings', 
            'pythia8CP5Settings', 
            'processParameters'
        ),
        processParameters = cms.vstring(
            'SUSY:all = off', 
            'SUSY:gg2gluinogluino = on', 
            'SUSY:qqbar2gluinogluino = on', 
            'RHadrons:allow = on', 
            'RHadrons:allowDecay = off', 
            'RHadrons:setMasses = on', 
            'RHadrons:probGluinoball = 0.1'
        ),
        pythia8CP5Settings = cms.vstring(
            'Tune:pp 14', 
            'Tune:ee 7', 
            'MultipartonInteractions:ecmPow=0.03344', 
            'MultipartonInteractions:bProfile=2', 
            'MultipartonInteractions:pT0Ref=1.41', 
            'MultipartonInteractions:coreRadius=0.7634', 
            'MultipartonInteractions:coreFraction=0.63', 
            'ColourReconnection:range=5.176', 
            'SigmaTotal:zeroAXB=off', 
            'SpaceShower:alphaSorder=2', 
            'SpaceShower:alphaSvalue=0.118', 
            'SigmaProcess:alphaSvalue=0.118', 
            'SigmaProcess:alphaSorder=2', 
            'MultipartonInteractions:alphaSvalue=0.118', 
            'MultipartonInteractions:alphaSorder=2', 
            'TimeShower:alphaSorder=2', 
            'TimeShower:alphaSvalue=0.118', 
            'SigmaTotal:mode = 0', 
            'SigmaTotal:sigmaEl = 21.89', 
            'SigmaTotal:sigmaTot = 100.309', 
            'PDF:pSet=LHAPDF6:NNPDF31_nnlo_as_0118'
        ),
        pythia8CommonSettings = cms.vstring(
            'Tune:preferLHAPDF = 2', 
            'Main:timesAllowErrors = 10000', 
            'Check:epTolErr = 0.01', 
            'Beams:setProductionScalesFromLHEF = off', 
            #'SLHA:keepSM = on', COMMENTED OUT DUE TO ERROR
            'SLHA:minMassSM = 1000.', 
            'ParticleDecays:limitTau0 = on', 
            'ParticleDecays:tau0Max = 10', 
            'ParticleDecays:allowPhotonRadiation = on',
            '1000021:mayDecay = off'
        )
    ),
    SLHAFileForPythia8 = cms.string('SimG4Core/CustomPhysics/data/TESTDECAY_GLUINO1800_STOP500.txt'),
    comEnergy = cms.double(13000.0),
    crossSection = cms.untracked.double(-1),
    hscpFlavor = cms.untracked.string('gluino'),
    massPoint = cms.untracked.int32(options.mass),
    maxEventsToPrint = cms.untracked.int32(0),
    #particleFile = cms.untracked.string('SimG4Core/CustomPhysics/data/TESTDECAY_GLUINO1800.txt'),
    #RhadronPythiaDecayerCommandFile = cms.untracked.string('SimG4Core/CustomPhysics/data/RhadronPythiaDecayerCommands.txt'),
    #pdtFile = cms.FileInPath('Configuration/Generator/data/hscppythiapdtgluino1800.tbl'),
    processFile = cms.untracked.string('SimG4Core/CustomPhysics/data/RhadronProcessList.txt'),
    pythiaHepMCVerbosity = cms.untracked.bool(False),
    pythiaPylistVerbosity = cms.untracked.int32(0),
    #slhaFile = cms.untracked.string('Configuration/Generator/data/TESTDECAY_GLUINO1800.txt'),
    useregge = cms.bool(False),
    initialSeed = cms.untracked.uint32(process.RandomNumberGeneratorService.generator.initialSeed.value()),
#    engineName = cms.untracked.string('TRandom3')
)

#ENABLE FOR DEBUGGING LOG
process.MessageLogger = cms.Service(
    "MessageLogger",
    destinations = cms.untracked.vstring(
        'detailedInfo',
         ),
    detailedInfo = cms.untracked.PSet(
        threshold  = cms.untracked.string('DEBUG')
        ),
    debugModules = cms.untracked.vstring('*')
    )

process.ProductionFilterSequence = cms.Sequence(process.generator+process.dirhadrongenfilter)

# Path and EndPath definitions
process.generation_step = cms.Path(process.pgen)
process.simulation_step = cms.Path(process.psim)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.RAWSIMoutput_step = cms.EndPath(process.RAWSIMoutput)

# Schedule definition
process.schedule = cms.Schedule(process.generation_step,process.genfiltersummary_step,process.simulation_step,process.endjob_step,process.RAWSIMoutput_step)
from PhysicsTools.PatAlgos.tools.helpers import associatePatAlgosToolsTask
associatePatAlgosToolsTask(process)
# filter all path with the production filter sequence
for path in process.paths:
	getattr(process,path).insert(0, process.ProductionFilterSequence)

# customisation of the process.

# Automatic addition of the customisation function from SimG4Core.CustomPhysics.Exotica_HSCP_SIM_cfi
from SimG4Core.CustomPhysics.Exotica_HSCP_SIM_cfi import customise 

#call to customisation function customise imported from SimG4Core.CustomPhysics.Exotica_HSCP_SIM_cfi
process = customise(process)

# Automatic addition of the customisation function from Configuration.DataProcessing.Utils
from Configuration.DataProcessing.Utils import addMonitoring 

#call to customisation function addMonitoring imported from Configuration.DataProcessing.Utils
process = addMonitoring(process)

# End of customisation functions

# Customisation from command line

# Add early deletion of temporary data products to reduce peak memory need
from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
process = customiseEarlyDelete(process)
# End adding early deletion
