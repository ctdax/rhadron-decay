#include "SimG4Core/CustomPhysics/interface/RHadronPythiaDecayDataManager.h"

RHadronPythiaDecayDataManager& RHadronPythiaDecayDataManager::getInstance() {
    static RHadronPythiaDecayDataManager instance;
    return instance;
}

void RHadronPythiaDecayDataManager::addDecayInfo(int id, float x, float y, float z, float t, float px, float py, float pz, float e) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    storedDecayIDs_.push_back(id);
    storedDecayX_.push_back(x);
    storedDecayY_.push_back(y);
    storedDecayZ_.push_back(z);
    storedDecayTime_.push_back(t);
    storedDecayPx_.push_back(px);
    storedDecayPy_.push_back(py);
    storedDecayPz_.push_back(pz);
    storedDecayE_.push_back(e);
}

void RHadronPythiaDecayDataManager::getDecayInfo(std::vector<int>& ids, std::vector<float>& x, std::vector<float>& y, 
                                          std::vector<float>& z, std::vector<float>& t, std::vector<float>& px,
                                          std::vector<float>& py, std::vector<float>& pz, std::vector<float>& e) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    ids = storedDecayIDs_;
    x = storedDecayX_;
    y = storedDecayY_;
    z = storedDecayZ_;
    t = storedDecayTime_;
    px = storedDecayPx_;
    py = storedDecayPy_;
    pz = storedDecayPz_;
    e = storedDecayE_;
}

void RHadronPythiaDecayDataManager::clearDecayInfo() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    storedDecayIDs_.clear();
    storedDecayX_.clear();
    storedDecayY_.clear();
    storedDecayZ_.clear();
    storedDecayTime_.clear();
    storedDecayPx_.clear();
    storedDecayPy_.clear();
    storedDecayPz_.clear();
    storedDecayE_.clear();
}