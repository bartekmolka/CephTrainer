#include "CephalometricAnalyzer.h"

#include <cmath>
#include <sstream>
#include <iomanip>

static std::string fmtFloat(float v, int precision)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << v;
    return ss.str();
}

float CephalometricAnalyzer::computePixelToMmScale(const std::vector<PointPair> &points, float knownDistanceMm)
{
    if (points.size() < 2)
        return 1.f;

    const float dist = static_cast<float>(
        cv::norm(points[0].userPoint - points[1].userPoint));

    return (dist > 0.f) ? (knownDistanceMm / dist) : 1.f;
}

AnalysisResult CephalometricAnalyzer::analyze(const std::vector<PointPair> &points, float pixelToMmScale, int cvmStage)
{
    AnalysisResult r;

    // Indeksy: 0=ScaleP1, 1=ScaleP2, 2=S, 3=N, 4=A, 5=B
    // Kąt ANB liczymy z N(3), A(4), B(5)
    r.userANB = calculateANB(points[3].userPoint, points[4].userPoint, points[5].userPoint);
    r.expertANB = calculateANB(points[3].groundTruthPoint, points[4].groundTruthPoint, points[5].groundTruthPoint);
    r.anbDelta = std::abs(r.userANB - r.expertANB);

    r.userSkeletalClass = classifySkeletalClass(r.userANB);
    r.expertSkeletalClass = classifySkeletalClass(r.expertANB);
    r.diagnosticMatch = (r.userSkeletalClass == r.expertSkeletalClass);

    auto &L = r.reportLines;

    L.push_back("--- LANDMARK DETECTION ERRORS ---");
    for (size_t i = 2; i < points.size(); ++i)
    {
        const float errorMm = static_cast<float>(cv::norm(points[i].userPoint - points[i].groundTruthPoint)) * pixelToMmScale;

        const std::string shortName = points[i].name.substr(0, points[i].name.find(' '));
        L.push_back("  " + shortName + " Dev: " + fmtFloat(errorMm, 2) + " mm");
    }

    L.push_back("");
    L.push_back("--- ANGULAR CEPHALOMETRIC METRICS ---");
    L.push_back("  Your ANB Angle:   " + fmtFloat(r.userANB, 2) + " deg");
    L.push_back("  Expert ANB Angle: " + fmtFloat(r.expertANB, 2) + " deg");
    L.push_back("  Angle Deviation:  " + fmtFloat(r.anbDelta, 2) + " deg");

    L.push_back("");
    L.push_back("--- DIAGNOSTIC SKELETAL CLASS ---");
    L.push_back("User Verdict:   " + skeletalClassName(r.userSkeletalClass, true));
    L.push_back("Expert Verdict: " + skeletalClassName(r.expertSkeletalClass, false));

    L.push_back("");
    appendTreatmentPlan(L, r.expertSkeletalClass, cvmStage);

    return r;
}

float CephalometricAnalyzer::calculateANB(cv::Point2f N, cv::Point2f A, cv::Point2f B)
{
    const cv::Point2f vecNA = A - N;
    const cv::Point2f vecNB = B - N;
    return (std::atan2(vecNB.y, vecNB.x) - std::atan2(vecNA.y, vecNA.x)) * 180.f / static_cast<float>(CV_PI);
}

int CephalometricAnalyzer::classifySkeletalClass(float anbAngle)
{
    if (anbAngle > 4.f)
        return 2;
    if (anbAngle < 2.f)
        return 3;
    return 1;
}

std::string CephalometricAnalyzer::skeletalClassName(int cls, bool isUser)
{
    if (isUser)
    {
        switch (cls)
        {
        case 1:
            return "Class I (Normal relation)";
        case 2:
            return "Class II (Retrognathism)";
        default:
            return "Class III (Prognathism)";
        }
    }
    else
    {
        switch (cls)
        {
        case 1:
            return "Class I (Normal)";
        case 2:
            return "Class II (Malocclusion)";
        default:
            return "Class III (Malocclusion)";
        }
    }
}

void CephalometricAnalyzer::appendTreatmentPlan(std::vector<std::string> &lines, int expertCls, int cvmStage)
{
    lines.push_back("--- POTENTIAL TREATMENT PLAN ---");

    if (cvmStage <= 0)
    {
        lines.push_back("CVM Stage: NOT LOADED OR INVALID");
        lines.push_back("Plan: Based strictly on permanent dentition state.");
        return;
    }

    lines.push_back("CVM Cervical Stage: CS" + std::to_string(cvmStage));

    if (cvmStage <= 2)
    {
        lines.push_back("Growth Status: Pre-peak skeletal growth period.");
        if (expertCls == 2)
            lines.push_back("Plan: Functional appliances (Twin Block / Herbst) recommended.");
        else if (expertCls == 3)
            lines.push_back("Plan: Facemask therapy / Orthopaedic expansion.");
        else
            lines.push_back("Plan: Preventive interceptive orthodontics / Monitoring.");
    }
    else if (cvmStage == 3 || cvmStage == 4)
    {
        lines.push_back("Growth Status: Peak growth window is currently OPEN.");
        if (expertCls == 2)
            lines.push_back("Plan: Active mandibular advancement (Herbst appliance).");
        else if (expertCls == 3)
            lines.push_back("Plan: Rapid maxillary expansion (RME) / Maxillary protraction.");
        else
            lines.push_back("Plan: Coordinate arches / Comprehensive fixed treatment.");
    }
    else
    {
        lines.push_back("Growth Status: Post-peak growth. Skeletal maturation completed.");
        if (expertCls == 2 || expertCls == 3)
            lines.push_back("Plan: Orthodontic Camouflage or Orthognathic Surgery (>18 yo).");
        else
            lines.push_back("Plan: Standard fixed mechanotherapy (brackets/aligners).");
    }
}