#include "PatientLoader.h"

#include <nlohmann/json.hpp>
#include <opencv2/imgcodecs.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

PatientData PatientLoader::load(const std::string &imgPath, const std::string &landmarkJsonPath, const std::string &cvmJsonPath) const
{
    PatientData result;
    result.points = defaultPoints();

    result.image = cv::imread(imgPath);
    if (result.image.empty())
    {
        std::cerr << "[PatientLoader] Cannot read image: " << imgPath << "\n";
        return result;
    }

    if (!parseGroundTruth(landmarkJsonPath, result.points, result.expertLandmarks))
    {
        std::cerr << "[PatientLoader] Cannot parse landmarks: " << landmarkJsonPath << "\n";
        return result;
    }

    result.cvmStage = parseCvmStage(cvmJsonPath);

    result.valid = true;
    return result;
}

bool PatientLoader::parseGroundTruth(const std::string &jsonPath, std::vector<PointPair> &points, std::vector<Landmark> &expertLandmarks) const
{
    std::ifstream file(jsonPath);
    if (!file.is_open())
        return false;

    try
    {
        json data;
        file >> data;

        if (!data.contains("landmarks"))
            return false;

        for (const auto &item : data["landmarks"])
        {
            const std::string symbol = item["symbol"];
            const float x = item["value"]["x"];
            const float y = item["value"]["y"];

            for (auto &p : points)
            {
                if (p.name.substr(0, p.name.find(' ')) == symbol)
                {
                    p.groundTruthPoint = cv::Point2f(x, y);
                    break;
                }
            }

            expertLandmarks.push_back({symbol, static_cast<int>(x), static_cast<int>(y)});
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[PatientLoader] JSON parse error: " << e.what() << "\n";
        return false;
    }

    return true;
}

int PatientLoader::parseCvmStage(const std::string &cvmJsonPath) const
{
    std::ifstream file(cvmJsonPath);
    if (!file.is_open())
    {
        std::cerr << "[PatientLoader] CVM file not found: " << cvmJsonPath << "\n";
        return 0;
    }

    try
    {
        json data;
        file >> data;

        if (data.contains("cvm_stage") && data["cvm_stage"].is_object())
        {
            const auto &sub = data["cvm_stage"];
            if (sub.contains("value"))
            {
                const int stage = sub["value"].get<int>();
                std::cout << "[PatientLoader] CVM stage: CS" << stage << "\n";
                return stage;
            }
        }
        std::cerr << "[PatientLoader] Invalid CVM JSON structure in: " << cvmJsonPath << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "[PatientLoader] CVM parse exception: " << e.what() << "\n";
    }
    return 0;
}

std::vector<PointPair> PatientLoader::defaultPoints()
{
    // Kolejność MUSI być zachowana: Scale P1, Scale P2, S, N, A, B
    return {
        {"Scale P1", {0, 0}, {0, 0}},
        {"Scale P2", {0, 0}, {0, 0}},
        {"S (Sella)", {0, 0}, {0, 0}},
        {"N (Nasion)", {0, 0}, {0, 0}},
        {"A (Subspinale)", {0, 0}, {0, 0}},
        {"B (Supramentale)", {0, 0}, {0, 0}},
    };
}