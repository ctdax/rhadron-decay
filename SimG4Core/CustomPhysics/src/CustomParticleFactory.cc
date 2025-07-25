#include "SimG4Core/CustomPhysics/interface/CustomParticleFactory.h"
#include "SimG4Core/CustomPhysics/interface/CustomPDGParser.h"
#include "SimG4Core/CustomPhysics/interface/CustomParticle.h"

#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "G4ParticleTable.hh"
#include "G4DecayTable.hh"
#include "G4PhaseSpaceDecayChannel.hh"
#include "G4ProcessManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

#include "SimG4Core/CustomPhysics/interface/CMSSIMP.h"
#include "SimG4Core/CustomPhysics/interface/CMSAntiSIMP.h"

#include <iomanip>
#include <iostream>
#include <sstream>

bool CustomParticleFactory::loaded = false;
std::vector<G4ParticleDefinition *> CustomParticleFactory::m_particles;

#ifdef G4MULTITHREADED
G4Mutex CustomParticleFactory::customParticleFactoryMutex = G4MUTEX_INITIALIZER;
#endif

CustomParticleFactory::CustomParticleFactory() {}

void CustomParticleFactory::loadCustomParticles(const std::string &filePath) {
  if (loaded) {
    return;
  }
#ifdef G4MULTITHREADED
  G4MUTEXLOCK(&customParticleFactoryMutex);
  if (loaded) {
    return;
  }
#endif

  // loading once
  loaded = true;
  std::ifstream configFile(filePath.c_str());

  std::string line;
  edm::LogVerbatim("SimG4CoreCustomPhysics")
      << "CustomParticleFactory: Reading Custom Particle and G4DecayTable from \n"
      << filePath;
  // This should be compatible IMO to SLHA
  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();
  G4double gluinoLifetime = -1.0; // Default value for the lifetime of the gluino, will be set if a gluino decay is found in the file
  G4double stopLifetime = -1.0; // Default value for the lifetime of the stop, will be set if a stop decay is found in the file
  while (getline(configFile, line)) {
    line.erase(0, line.find_first_not_of(" \t"));  // Remove leading whitespace.
    if (line.length() == 0 || line.at(0) == '#') continue; // Skip blank lines and comments.
    // The mass table begins with a line containing "BLOCK MASS"
    if (ToLower(line).find("block") < line.npos && ToLower(line).find("mass") < line.npos) {
      edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomParticleFactory: Retrieving mass table.";
      getMassTable(&configFile);
    }
    if (line.find("DECAY") < line.npos) {
      int pdgId;
      double width;
      std::string tmpString;
      std::stringstream lineStream(line);
      lineStream >> tmpString >> pdgId >> width;
      // assume SLHA format, e.g.: DECAY  1000021  5.50675438E+00   # gluino decays
      edm::LogVerbatim("SimG4CoreCustomPhysics")
          << "CustomParticleFactory: entry to G4DecayTable: pdgID, width " << pdgId << ",  " << width;
      G4ParticleDefinition *aParticle = theParticleTable->FindParticle(pdgId);
      if (nullptr == aParticle || width == 0.0) { // Skip if particle is stable or not found
        continue;
      }
      if (pdgId == 1000021) { // Set the gluino RhadronLifetime to the gluino lifetime. Skip the decay table of the gluino. The decay table will be passed to the RhadronPythiaDecayer
        gluinoLifetime = 1.0 / (width * CLHEP::GeV) * 6.582122e-22 * CLHEP::MeV * CLHEP::s;
        continue;
      }
      if (pdgId == 1000006) { // Set the stop RhadronLifetime to the stop lifetime. Skip the decay table of the stop. The decay table will be passed to the RhadronPythiaDecayer
        stopLifetime = 1.0 / (width * CLHEP::GeV) * 6.582122e-22 * CLHEP::MeV * CLHEP::s;
        continue;
      }

      G4DecayTable *aDecayTable = getDecayTable(&configFile, pdgId);
      aParticle->SetDecayTable(aDecayTable);
      aParticle->SetPDGStable(false);
      aParticle->SetPDGLifeTime(1.0 / (width * CLHEP::GeV) * 6.582122e-22 * CLHEP::MeV * CLHEP::s);

      if (pdgId < 999999 || pdgId > 2000015) { // Do not handle anti decay tables for SUSY particles
        G4ParticleDefinition *aAntiParticle = theParticleTable->FindAntiParticle(pdgId);
        if (nullptr != aAntiParticle && aAntiParticle->GetPDGEncoding() != pdgId) {
          aAntiParticle->SetDecayTable(getAntiDecayTable(pdgId, aDecayTable));
          aAntiParticle->SetPDGStable(false);
          aAntiParticle->SetPDGLifeTime(1.0 / (width * CLHEP::GeV) * 6.582122e-22 * CLHEP::MeV * CLHEP::s);
        }
      }
    }
  }

  // If the gluinoLifetime is set, set it for all gluino Rhadrons
  if (gluinoLifetime != -1.0) {
    for (auto &particle : m_particles) {
      if (CustomPDGParser::s_isgluinoHadron(particle->GetPDGEncoding())) {
        particle->SetPDGStable(false);
        particle->SetPDGLifeTime(gluinoLifetime);
        edm::LogVerbatim("SimG4CoreCustomPhysics")
            << "CustomParticleFactory: Setting lifetime for " << particle->GetParticleName() << " equal to lifetime of the gluino. This is "
            << gluinoLifetime << "s.";
      }
    }
  }

  // If the stopLifetime is set, set it for all stop Rhadrons
  if (stopLifetime != -1.0) {
    for (auto &particle : m_particles) {
      if (CustomPDGParser::s_isstopHadron(particle->GetPDGEncoding())) {
        particle->SetPDGStable(false);
        particle->SetPDGLifeTime(stopLifetime);
        edm::LogVerbatim("SimG4CoreCustomPhysics")
            << "CustomParticleFactory: Setting lifetime for " << particle->GetParticleName() << " equal to lifetime of the stop. This is "
            << stopLifetime << "s.";
      }
    }
  }
#ifdef G4MULTITHREADED
  G4MUTEXUNLOCK(&customParticleFactoryMutex);
#endif
}

void CustomParticleFactory::addCustomParticle(int pdgCode, double mass, const std::string &name) {
  if (std::abs(pdgCode) % 100 < 14 && std::abs(pdgCode) / 1000000 == 0) {
    edm::LogError("CustomParticleFactory::addCustomParticle")
        << "Pdg code too low " << pdgCode << " " << std::abs(pdgCode) / 1000000;
    return;
  }

  if (CustomPDGParser::s_isSIMP(pdgCode)) {
    CMSSIMP *simp = CMSSIMP::Definition(mass * GeV);
    CMSAntiSIMP *antisimp = CMSAntiSIMP::Definition(mass * GeV);
    m_particles.push_back(simp);
    m_particles.push_back(antisimp);
    return;
  }

  /////////////////////// Check!!!!!!!!!!!!!
  G4String pType = "custom";
  G4String pSubType = "";
  G4double spectatormass = 0.0;
  G4ParticleDefinition *spectator = nullptr;
  //////////////////////
  if (CustomPDGParser::s_isgluinoHadron(pdgCode)) {
    pType = "rhadron";
  }
  if (CustomPDGParser::s_isSLepton(pdgCode)) {
    pType = "sLepton";
  }
  if (CustomPDGParser::s_isMesonino(pdgCode)) {
    pType = "mesonino";
  }
  if (CustomPDGParser::s_isSbaryon(pdgCode)) {
    pType = "sbaryon";
  }

  double massGeV = mass * CLHEP::GeV;
  double width = 0.0;
  double charge = CLHEP::eplus * CustomPDGParser::s_charge(pdgCode);
  if (name.compare(0, 4, "~HIP") == 0) {
    if ((name.compare(0, 7, "~HIPbar") == 0)) {
      std::string str = name.substr(7);
      charge = CLHEP::eplus * atoi(str.c_str()) / 3.;
    } else {
      std::string str = name.substr(4);
      charge = -CLHEP::eplus * atoi(str.c_str()) / 3.;
    }
  }
  if (name.compare(0, 9, "anti_~HIP") == 0) {
    if ((name.compare(0, 12, "anti_~HIPbar") == 0)) {
      std::string str = name.substr(12);
      charge = -CLHEP::eplus * atoi(str.c_str()) / 3.;
    } else {
      std::string str = name.substr(9);
      charge = CLHEP::eplus * atoi(str.c_str()) / 3.;
    }
  }
  int spin = (int)CustomPDGParser::s_spin(pdgCode) - 1;
  int parity = +1;
  int conjugation = 0;
  int isospin = 0;
  int isospinZ = 0;
  int gParity = 0;
  int lepton = 0;  //FIXME:
  int baryon = 1;  //FIXME:
  bool stable = true;
  double lifetime = -1;

  if (CustomPDGParser::s_isDphoton(pdgCode)) {
    pType = "darkpho";
    spin = 2;
    parity = -1;
    conjugation = -1;
    isospin = 0;
    isospinZ = 0;
    gParity = 0;
    lepton = 0;
    baryon = 0;
    stable = true;
    lifetime = -1;
  }

  G4DecayTable *decaytable = nullptr;
  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

  CustomParticle *particle = new CustomParticle(name,
                                                massGeV,
                                                width,
                                                charge,
                                                spin,
                                                parity,
                                                conjugation,
                                                isospin,
                                                isospinZ,
                                                gParity,
                                                pType,
                                                lepton,
                                                baryon,
                                                pdgCode,
                                                stable,
                                                lifetime,
                                                decaytable);

  if (pType == "rhadron" && name != "~g") {
    G4String cloudname = name + "cloud";
    G4String cloudtype = pType + "cloud";
    spectator = theParticleTable->FindParticle(1000021);
    spectatormass = spectator->GetPDGMass();
    G4double cloudmass = massGeV - spectatormass;
    CustomParticle *tmpParticle =
        new CustomParticle(cloudname, cloudmass, 0.0, 0, 0, +1, 0, 0, 0, 0, cloudtype, 0, +1, 0, true, -1.0, nullptr);
    particle->SetCloud(tmpParticle);
    particle->SetSpectator(spectator);

    edm::LogVerbatim("SimG4CoreCustomPhysics")
        << "CustomParticleFactory: " << name << " being assigned spectator" << spectator->GetParticleName()
        << " and cloud " << cloudname << "\n"
        << "                        Masses: " << mass << " Gev, " << spectatormass / CLHEP::GeV << " GeV and "
        << cloudmass / CLHEP::GeV << " GeV.";
  } else if (pType == "mesonino" || pType == "sbaryon") {
    int sign = 1;
    if (pdgCode < 0)
      sign = -1;

    G4String cloudname = name + "cloud";
    G4String cloudtype = pType + "cloud";
    if (CustomPDGParser::s_isstopHadron(pdgCode)) {
      spectator = theParticleTable->FindParticle(1000006 * sign);
    } else {
      if (CustomPDGParser::s_issbottomHadron(pdgCode)) {
        spectator = theParticleTable->FindParticle(1000005 * sign);
      } else {
        spectator = nullptr;
        edm::LogError("SimG4CoreCustomPhysics") << "CustomParticleFactory: Cannot find spectator parton";
      }
    }
    if (nullptr != spectator) {
      spectatormass = spectator->GetPDGMass();
    }
    G4double cloudmass = massGeV - spectatormass;
    CustomParticle *tmpParticle =
        new CustomParticle(cloudname, cloudmass, 0.0, 0, 0, +1, 0, 0, 0, 0, cloudtype, 0, +1, 0, true, -1.0, nullptr);
    particle->SetCloud(tmpParticle);
    particle->SetSpectator(spectator);

    if (nullptr != spectator)
      edm::LogVerbatim("SimG4CoreCustomPhysics")
          << "CustomParticleFactory: " << name << " being assigned spectator" << spectator->GetParticleName()
          << " and cloud " << cloudname << "\n"
          << "                        Masses: " << mass << " Gev, " << spectatormass / CLHEP::GeV << " GeV and "
          << cloudmass / CLHEP::GeV << " GeV.";
  } else {
    particle->SetCloud(nullptr);
    particle->SetSpectator(nullptr);
  }
  m_particles.push_back(particle);
}

void CustomParticleFactory::getMassTable(std::ifstream *configFile) {
  int pdgId;
  double mass;
  std::string name, tmp;
  std::string line;
  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

  // This should be compatible IMO to SLHA
  while (getline(*configFile, line)) {
    line.erase(0, line.find_first_not_of(" \t"));  // remove leading whitespace
    if ((ToLower(line).find("block") < line.npos) || (line == "#")) {
      edm::LogInfo("SimG4CoreCustomPhysics") << "CustomParticleFactory: Finished the Mass Table ";
      break;
    }
    if (line.length() == 0 || line.at(0) == '#')
      continue;  // skip blank lines and comments
    std::stringstream sstr(line);
    sstr >> pdgId >> mass >> tmp >> name;  // Assume SLHA format, e.g.: 1000001 5.68441109E+02 # ~d_L

    mass = std::max(mass, 0.0);
    if (theParticleTable->FindParticle(pdgId)) {
      continue;
    }

    edm::LogVerbatim("SimG4CoreCustomPhysics")
        << "CustomParticleFactory: Calling addCustomParticle for pdgId: " << pdgId << ", mass " << mass << " GeV  "
        << name << ", isgluinoHadron: " << CustomPDGParser::s_isgluinoHadron(pdgId)
        << ", isstopHadron: " << CustomPDGParser::s_isstopHadron(pdgId)
        << ", isSIMP: " << CustomPDGParser::s_isSIMP(pdgId);
    addCustomParticle(pdgId, mass, name);

    ////Find SM particle partner and check for the antiparticle.
    int pdgIdPartner = pdgId % 100;
    G4ParticleDefinition *aParticle = theParticleTable->FindParticle(pdgIdPartner);
    //Add antiparticles for SUSY particles only, not for rHadrons.
    if (nullptr != aParticle) {
      edm::LogVerbatim("SimG4CoreCustomPhysics")
          << "CustomParticleFactory: Found partner particle for "
          << " pdgId= " << pdgId << ", pdgIdPartner= " << pdgIdPartner << " " << aParticle->GetParticleName();
    }

    if (nullptr != aParticle && !CustomPDGParser::s_isgluinoHadron(pdgId) && !CustomPDGParser::s_isstopHadron(pdgId) &&
        !CustomPDGParser::s_isSIMP(pdgId) && pdgId != 1000006 && pdgId != -1000006 && pdgId != 25 && pdgId != 35 &&
        pdgId != 36 && pdgId != 37) {
      int sign = aParticle->GetAntiPDGEncoding() / pdgIdPartner;
      edm::LogVerbatim("SimG4CoreCustomPhysics")
          << "CustomParticleFactory: For " << aParticle->GetParticleName() << " pdg= " << pdgIdPartner
          << ", GetAntiPDGEncoding() " << aParticle->GetAntiPDGEncoding() << " sign= " << sign;

      if (std::abs(sign) != 1) {
        edm::LogVerbatim("SimG4CoreCustomPhysics")
            << "CustomParticleFactory: sign= " << sign << " a: " << aParticle->GetAntiPDGEncoding()
            << " b: " << pdgIdPartner;
        aParticle->DumpTable();
      }
      if (sign == -1 && pdgId != 25 && pdgId != 35 && pdgId != 36 && pdgId != 37 && pdgId != 1000039) {
        tmp = "anti_" + name;
        if (theParticleTable->FindParticle(-pdgId) == nullptr) {
          edm::LogVerbatim("SimG4CoreCustomPhysics")
              << "CustomParticleFactory: Calling addCustomParticle for antiparticle with pdgId: " << -pdgId << ", mass "
              << mass << " GeV, name " << tmp;
          addCustomParticle(-pdgId, mass, tmp);
          theParticleTable->FindParticle(pdgId)->SetAntiPDGEncoding(-pdgId);
        }
      } else
        theParticleTable->FindParticle(pdgId)->SetAntiPDGEncoding(pdgId);
    }

    if (pdgId == 1000039)
      theParticleTable->FindParticle(pdgId)->SetAntiPDGEncoding(pdgId);  // gravitino
    if (pdgId == 1000024 || pdgId == 1000037 || pdgId == 37) {
      tmp = "anti_" + name;
      if (!theParticleTable->FindParticle(-pdgId)) {
        edm::LogVerbatim("SimG4CoreCustomPhysics")
            << "CustomParticleFactory: Calling addCustomParticle for antiparticle (2) with pdgId: " << -pdgId
            << ", mass " << mass << " GeV, name " << tmp;
        addCustomParticle(-pdgId, mass, tmp);
        theParticleTable->FindParticle(pdgId)->SetAntiPDGEncoding(-pdgId);
      }
    }
    if (pdgId == 9000006) {
      tmp = name + "bar";
      edm::LogVerbatim("CustomPhysics") << "Calling addCustomParticle for antiparticle with pdgId: " << -pdgId
                                        << ", mass " << mass << ", name " << tmp;
      addCustomParticle(-pdgId, mass, tmp);
      theParticleTable->FindParticle(pdgId)->SetAntiPDGEncoding(-pdgId);
    }
  }
}

G4DecayTable *CustomParticleFactory::getDecayTable(std::ifstream *configFile, int pdgId) {
  double br;
  int nDaughters;
  int pdg[4] = {0};
  std::string line;

  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

  std::string parentName = theParticleTable->FindParticle(pdgId)->GetParticleName();
  G4DecayTable *decaytable = new G4DecayTable();

  while (getline(*configFile, line)) {
    line.erase(0, line.find_first_not_of(" \t"));  // remove leading whitespace
    if (line.length() == 0)
      continue;  // skip blank lines
    if (line.at(0) == '#' && ToLower(line).find("br") < line.npos && ToLower(line).find("nda") < line.npos)
      continue;  // skip a comment of the form:  # BR  NDA  ID1  ID2
    if (line.at(0) == '#' || ToLower(line).find("block") < line.npos) {
      edm::LogInfo("SimG4CoreCustomPhysics") << "CustomParticleFactory: Finished the Decay Table ";
      break;
    }

    std::stringstream sstr(line);
    sstr >> br >> nDaughters;  // assume SLHA format, e.g.:  1.49435135E-01  2  -15  16  # BR(H+ -> tau+ nu_tau)
    LogDebug("SimG4CoreCustomPhysics") << "CustomParticleFactory: Branching Ratio: " << br
                                       << ", Number of Daughters: " << nDaughters;
    if (nDaughters > 4) {
      edm::LogError("SimG4CoreCustomPhysics")
          << "CustomParticleFactory: Number of daughters is too large (max = 4): " << nDaughters
          << " for pdgId: " << pdgId;
      break;
    }
    if (br <= 0.0) {
      edm::LogError("SimG4CoreCustomPhysics")
          << "CustomParticleFactory: Branching ratio is " << br << " for pdgId: " << pdgId;
      break;
    }
    std::string name[4] = {""};
    for (int i = 0; i < nDaughters; ++i) {
      sstr >> pdg[i];
      LogDebug("SimG4CoreCustomPhysics") << "CustomParticleFactory: Daughter ID " << pdg[i];
      const G4ParticleDefinition *part = theParticleTable->FindParticle(pdg[i]);
      if (!part) {
        edm::LogWarning("SimG4CoreCustomPhysics")
            << "CustomParticleFactory: particle with PDG code" << pdg[i] << " not found!";
        continue;
      }
      name[i] = part->GetParticleName();
    }
    ////Set the G4 decay
    G4PhaseSpaceDecayChannel *aDecayChannel =
        new G4PhaseSpaceDecayChannel(parentName, br, nDaughters, name[0], name[1], name[2], name[3]);
    decaytable->Insert(aDecayChannel);
    edm::LogVerbatim("SimG4CoreCustomPhysics")
        << "CustomParticleFactory: inserted decay channel "
        << " for pdgID= " << pdgId << "  " << parentName << " BR= " << br << " Daughters: " << name[0] << "  "
        << name[1] << "  " << name[2] << "  " << name[3];
  }
  return decaytable;
}

G4DecayTable *CustomParticleFactory::getAntiDecayTable(int pdgId, G4DecayTable *theDecayTable) {
  std::string name[4] = {""};
  G4ParticleTable *theParticleTable = G4ParticleTable::GetParticleTable();

  std::string parentName = theParticleTable->FindParticle(-pdgId)->GetParticleName();
  G4DecayTable *decaytable = new G4DecayTable();

  for (int i = 0; i < theDecayTable->entries(); ++i) {
    G4VDecayChannel *theDecayChannel = theDecayTable->GetDecayChannel(i);
    int nd = std::min(4, theDecayChannel->GetNumberOfDaughters());
    for (int j = 0; j < nd; ++j) {
      int id = theDecayChannel->GetDaughter(j)->GetAntiPDGEncoding();
      G4ParticleDefinition *part = theParticleTable->FindParticle(id);
      if (nullptr == part) {
        edm::LogWarning("SimG4CoreCustomPhysics")
            << "CustomParticleFactory: antiparticle with PDG code" << id << " not found!";
        continue;
      }
      name[j] = part->GetParticleName();
    }
    G4PhaseSpaceDecayChannel *aDecayChannel =
        new G4PhaseSpaceDecayChannel(parentName, theDecayChannel->GetBR(), nd, name[0], name[1], name[2], name[3]);
    decaytable->Insert(aDecayChannel);
    edm::LogVerbatim("SimG4CoreCustomPhysics")
        << "CustomParticleFactory: inserted decay channel "
        << " for pdgID= " << -pdgId << "  " << parentName << " BR= " << theDecayChannel->GetBR()
        << " Daughters: " << name[0] << "  " << name[1] << "  " << name[2] << "  " << name[3];
  }
  return decaytable;
}

std::string CustomParticleFactory::ToLower(std::string str) {
  std::locale loc;
  for (std::string::size_type i = 0; i < str.length(); ++i)
    str.at(i) = std::tolower(str.at(i), loc);
  return str;
}

const std::vector<G4ParticleDefinition *> &CustomParticleFactory::getCustomParticles() { return m_particles; }
