#pragma once

#include "PointTypes.h"

#include <opencv2/core.hpp>
#include <string>
#include <vector>

struct AnalysisResult
{
    float userANB = 0.f;
    float expertANB = 0.f;
    float anbDelta = 0.f;

    int userSkeletalClass = 0;
    int expertSkeletalClass = 0;

    bool diagnosticMatch = false;

    std::vector<std::string> reportLines;
};

// Odpowiada za matematykę i logikę kliniczną:
//   - obliczenie kąta ANB
//   - klasyfikacja szkieletowa (I/II/III)
//   - generowanie planu leczenia na podstawie klasy + CVM
//   - obliczenie dewiacji punktów w mm

class CephalometricAnalyzer
{
public:
    static float computePixelToMmScale(const std::vector<PointPair> &points, float knownDistanceMm = 10.f);
    static AnalysisResult analyze(const std::vector<PointPair> &points, float pixelToMmScale, int cvmStage);
    static float calculateANB(cv::Point2f N, cv::Point2f A, cv::Point2f B);

private:
    static int classifySkeletalClass(float anbAngle);
    static std::string skeletalClassName(int cls, bool isUser);
    static void appendTreatmentPlan(std::vector<std::string> &lines, int expertCls, int cvmStage);
};