#include "DataManager.h"

#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

DataManager::DataManager()
{
    resolveDataRoot();
}

void DataManager::resolveDataRoot()
{
    m_dataRoot = readEnvFile("CEPH_DATA_DIR");
    if (!m_dataRoot.empty())
    {
        std::cout << "[DataManager] Root from .env: " << m_dataRoot << "\n";
        return;
    }

    if (const char *envPath = std::getenv("CEPH_DATA_DIR"))
    {
        m_dataRoot = envPath;
        std::cout << "[DataManager] Root from getenv: " << m_dataRoot << "\n";
        return;
    }

    try
    {
        m_dataRoot = (fs::current_path().parent_path() / "data").string();
        std::cout << "[DataManager] Root fallback: " << m_dataRoot << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "[DataManager] Cannot determine CWD: " << e.what() << "\n";
    }
}

std::vector<std::string> DataManager::scanCephalograms(int maxCount) const
{
    std::vector<std::string> names;
    fs::path imagesPath = fs::path(m_dataRoot) / "Cephalograms";

    if (!fs::exists(imagesPath))
    {
        std::cerr << "[DataManager] Cephalograms dir not found: " << imagesPath << "\n";
        return names;
    }

    try
    {
        for (const auto &entry : fs::directory_iterator(imagesPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".png")
            {
                names.push_back(entry.path().stem().string());
                if (static_cast<int>(names.size()) >= maxCount)
                    break;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[DataManager] Directory iteration error: " << e.what() << "\n";
    }

    std::sort(names.begin(), names.end());
    return names;
}

std::string DataManager::imagePath(const std::string &scanName) const
{
    return (fs::path(m_dataRoot) / "Cephalograms" / (scanName + ".png")).string();
}

std::string DataManager::landmarkPath(const std::string &scanName) const
{
    return (fs::path(m_dataRoot) / "Senior_Orthodontists" / (scanName + ".json")).string();
}

std::string DataManager::cvmPath(const std::string &scanName) const
{
    return (fs::path(m_dataRoot) / "CVM_Stages" / (scanName + ".json")).string();
}

std::string DataManager::readEnvFile(const std::string &key)
{
    std::ifstream f(".env");
    if (!f.is_open())
        return {};

    std::string line;
    while (std::getline(f, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        const size_t delim = line.find('=');
        if (delim != std::string::npos && line.substr(0, delim) == key)
            return line.substr(delim + 1);
    }
    return {};
}
