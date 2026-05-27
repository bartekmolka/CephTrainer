#pragma once

#include "PointTypes.h"

#include <opencv2/core.hpp>
#include <string>
#include <vector>

struct PatientData
{
    cv::Mat image;                 // Obraz RTG
    std::vector<PointPair> points; // Scale P1/P2 + S N A B z wyzerowanymi userPoint
    std::vector<Landmark> expertLandmarks;
    int cvmStage = 0; // 0 = niezaładowany
    bool valid = false;
};

// Odpowiada wyłącznie za I/O:
//   - wczytanie obrazu .png
//   - parsowanie JSON landmarks eksperta
//   - parsowanie JSON z fazą CVM

class PatientLoader
{
public:
    PatientData load(const std::string &imgPath, const std::string &landmarkJsonPath, const std::string &cvmJsonPath) const;

private:
    bool parseGroundTruth(const std::string &jsonPath, std::vector<PointPair> &points, std::vector<Landmark> &expertLandmarks) const;
    int parseCvmStage(const std::string &cvmJsonPath) const;
    static std::vector<PointPair> defaultPoints();
};