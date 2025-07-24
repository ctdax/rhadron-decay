#include <memory>

#include "SimG4Core/CustomPhysics/interface/CustomPhysicsList.h"
#include "SimG4Core/CustomPhysics/interface/CustomParticleFactory.h"
#include "SimG4Core/CustomPhysics/interface/CustomParticle.h"
#include "SimG4Core/CustomPhysics/interface/DummyChargeFlipProcess.h"
#include "SimG4Core/CustomPhysics/interface/G4ProcessHelper.h"
#include "SimG4Core/CustomPhysics/interface/CustomPDGParser.h"
#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayer.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "G4hMultipleScattering.hh"
#include "G4hIonisation.hh"
#include "G4ProcessManager.hh"
#include "G4Decay.hh"
#include "G4DecayTable.hh"

#include "SimG4Core/CustomPhysics/interface/FullModelHadronicProcess.h"
#include "SimG4Core/CustomPhysics/interface/CMSDarkPairProductionProcess.h"
#include "SimG4Core/CustomPhysics/interface/CMSQGSPSIMPBuilder.h"
#include "SimG4Core/CustomPhysics/interface/CMSSIMPInelasticProcess.h"

using namespace CLHEP;

G4ThreadLocal std::unique_ptr<G4ProcessHelper> CustomPhysicsList::myHelper;

CustomPhysicsList::CustomPhysicsList(const std::string& name, const edm::ParameterSet& p, bool apinew)
    : G4VPhysicsConstructor(name) {
  myConfig = p;
  if (apinew) {
    dfactor = p.getParameter<double>("DarkMPFactor");
    fHadronicInteraction = p.getParameter<bool>("RhadronPhysics");
  } else {
    // this is left for backward compatibility
    dfactor = p.getParameter<double>("dark_factor");
    fHadronicInteraction = p.getParameter<bool>("rhadronPhysics");
  }
  edm::FileInPath fp = p.getParameter<edm::FileInPath>("particlesDef");
  particleDefFilePath = fp.fullPath();
  fParticleFactory = std::make_unique<CustomParticleFactory>();
  myHelper.reset(nullptr);

  edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: Path for custom particle definition file: \n"
                                             << particleDefFilePath << "\n"
                                             << "      dark_factor= " << dfactor;
}

CustomPhysicsList::~CustomPhysicsList() {}

void CustomPhysicsList::ConstructParticle() {
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "===== CustomPhysicsList::ConstructParticle ";
  fParticleFactory.get()->loadCustomParticles(particleDefFilePath);
}

void CustomPhysicsList::ConstructProcess() {
  edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: adding CustomPhysics processes "
                                             << "for the list of particles";

  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

  // Set the pythia decayer for Rhadrons. RhadronPythiaDecayerCommandFile is an optional file that can be passed in the python config file to modify pythia settings for the Rhadron decays
  G4Decay* decay = new G4Decay(); // Used to check if decay is applicable for particles
  const std::string RhadronPythiaDecayerCommandFile = myConfig.getParameter<edm::FileInPath>("RhadronPythiaDecayerCommandFile").fullPath();
  G4Decay* pythiaDecayProcess = new RHadronPythiaDecayer(particleDefFilePath, RhadronPythiaDecayerCommandFile);
  G4VExtDecayer* extDecayer = dynamic_cast<G4VExtDecayer*>(pythiaDecayProcess);
  pythiaDecayProcess->SetExtDecayer(extDecayer); // Set the external decayer to itself. Seems redundant but is necessary as far as I can tell. Without doing this, RHadronPythiaDecayer::ImportDecayProducts() will not be called.

  for (auto particle : fParticleFactory.get()->getCustomParticles()) {
    if (particle->GetParticleType() == "simp") {
      G4ProcessManager* pmanager = particle->GetProcessManager();
      if (pmanager) {
        CMSSIMPInelasticProcess* simpInelPr = new CMSSIMPInelasticProcess();
        CMSQGSPSIMPBuilder* theQGSPSIMPB = new CMSQGSPSIMPBuilder();
        theQGSPSIMPB->Build(simpInelPr);
        pmanager->AddDiscreteProcess(simpInelPr);
      } else
        edm::LogVerbatim("CustomPhysics") << "   No pmanager";
    }

    CustomParticle* cp = dynamic_cast<CustomParticle*>(particle);
    if (cp) {
      G4ProcessManager* pmanager = particle->GetProcessManager();
      edm::LogVerbatim("SimG4CoreCustomPhysics")
          << "CustomPhysicsList: " << particle->GetParticleName() << "  PDGcode= " << particle->GetPDGEncoding()
          << "  Mass= " << particle->GetPDGMass() / GeV << " GeV.";
      if (pmanager) {

        if (particle->GetPDGCharge() != 0.0) {
          ph->RegisterProcess(new G4hMultipleScattering, particle);
          ph->RegisterProcess(new G4hIonisation, particle);
        }

        if (cp->GetCloud() && fHadronicInteraction &&
            (CustomPDGParser::s_isgluinoHadron(particle->GetPDGEncoding()) ||
             (CustomPDGParser::s_isstopHadron(particle->GetPDGEncoding())) ||
             (CustomPDGParser::s_issbottomHadron(particle->GetPDGEncoding())))) {
          edm::LogVerbatim("SimG4CoreCustomPhysics")
              << "CustomPhysicsList: " << particle->GetParticleName()
              << " CloudMass= " << cp->GetCloud()->GetPDGMass() / GeV
              << " GeV; SpectatorMass= " << cp->GetSpectator()->GetPDGMass() / GeV << " GeV.";

          if (!myHelper.get()) {
            myHelper = std::make_unique<G4ProcessHelper>(myConfig, fParticleFactory.get());
          }
          pmanager->AddDiscreteProcess(new FullModelHadronicProcess(myHelper.get()));
        }

        // Remove native G4 decay processes
        G4ProcessVector *fullProcessList = pmanager->GetProcessList();
        std::vector< G4VProcess * > existingDecayProcesses; existingDecayProcesses.reserve(2);
        for (unsigned int i=0; i<fullProcessList->size(); ++i) {
          G4VProcess* process = (*fullProcessList)[i];
          if (process->GetProcessType() == fDecay) {
            edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: Found a pre-existing decay process for " <<particle->GetParticleName() << ". Will remove in favour of using RHadronPythiaDecayer.";
            existingDecayProcesses.push_back(process);
          }
        }
        for (G4VProcess* process : existingDecayProcesses) {
          pmanager->RemoveProcess(process);
        }

        // Add R-hadron decay
        edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: Particle name = " << particle->GetParticleName()
                                                  << " Lifetime = " << particle->GetPDGLifeTime() / CLHEP::ns << " ns"
                                                  << " Mass = " << particle->GetPDGMass() / GeV << " GeV";
        if (decay->IsApplicable(*particle)) {
          edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: Adding decay for " << particle->GetParticleName();
          pmanager->AddProcess(pythiaDecayProcess);
          pmanager->SetProcessOrdering(pythiaDecayProcess, idxPostStep);
          pythiaDecayProcess->SetVerboseLevel(2);
        } else {
          edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: No decay allowed for " << particle->GetParticleName();
          if (!particle->GetPDGStable() && particle->GetPDGLifeTime() < 0.1 * CLHEP::ns) {
            edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: Gonna decay it anyway!!!";
            pmanager->AddProcess(pythiaDecayProcess);
            pmanager->SetProcessOrdering(pythiaDecayProcess, idxPostStep);
          }
        }
        
        if (particle->GetParticleType() == "darkpho") {
          CMSDarkPairProductionProcess* darkGamma = new CMSDarkPairProductionProcess(dfactor);
          pmanager->AddDiscreteProcess(darkGamma);
        }

        //Set the multiple scattering process second to last, transportation to last in the AlongStep order. Last is 9999
        G4ProcessVector* pv = pmanager->GetProcessList();
        for (size_t i = 0; i < pv->size(); ++i) {
          if ((*pv)[i]->GetProcessName() == "msc") {
            pmanager->SetProcessOrdering((*pv)[i], idxAlongStep, 9998);
            break;
          }
        }
        for (size_t i = 0; i < pv->size(); ++i) {
          if ((*pv)[i]->GetProcessName() == "Transportation") {
            pmanager->SetProcessOrderingToLast((*pv)[i], idxAlongStep);
            break;
          }
        }

        G4ProcessVector* alongStepVec = pmanager->GetAlongStepProcessVector(typeGPIL); // typeGPIL for physical interaction length
        for (size_t i = 0; i < alongStepVec->size(); ++i) {
          edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsList: AlongStep Process " << i << " for " << particle->GetParticleName()
                                                     << ": " << (*alongStepVec)[i]->GetProcessName() << "  . Order is: " << pmanager->GetProcessOrdering((*alongStepVec)[i], idxAlongStep);  
        }
      }
    }
  }
}
