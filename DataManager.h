#pragma once

#include <string>
#include <vector>
#include <filesystem>

// Odpowiada wyłącznie za:
//   - resolving ścieżki root z .env / getenv / fallback
//   - skanowanie folderu Cephalograms i zwracanie listy nazw skanów
//   - budowanie ścieżek do konkretnych zasobów (obraz, JSON, CVM)

class DataManager
{
public:
    DataManager();

    void resolveDataRoot();

    std::vector<std::string> scanCephalograms(int maxCount = 10) const;

    std::string imagePath(const std::string &scanName) const;
    std::string landmarkPath(const std::string &scanName) const;
    std::string cvmPath(const std::string &scanName) const;

    const std::string &dataRoot() const { return m_dataRoot; }

private:
    std::string m_dataRoot;

    static std::string readEnvFile(const std::string &key);
};