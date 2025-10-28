#ifndef RHadronPythiaDecayDataManager_H
#define RHadronPythiaDecayDataManager_H

#include <vector>
#include <mutex>

class RHadronPythiaDecayDataManager {
public:
    static RHadronPythiaDecayDataManager& getInstance();
    
    void addDecayInfo(int id, float x, float y, float z, float t, float px, float py, float pz, float e);
    void getDecayInfo(std::vector<int>& ids, std::vector<float>& x, std::vector<float>& y, 
                     std::vector<float>& z, std::vector<float>& t, std::vector<float>& px,
                     std::vector<float>& py, std::vector<float>& pz, std::vector<float>& e);
    void clearDecayInfo();

private:
    RHadronPythiaDecayDataManager() {}
    std::mutex dataMutex_;
    std::vector<int> storedDecayIDs_;
    std::vector<float> storedDecayX_;
    std::vector<float> storedDecayY_;
    std::vector<float> storedDecayZ_;
    std::vector<float> storedDecayTime_;
    std::vector<float> storedDecayPx_;
    std::vector<float> storedDecayPy_;
    std::vector<float> storedDecayPz_;
    std::vector<float> storedDecayE_;
};

#endif