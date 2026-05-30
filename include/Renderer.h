#pragma once

#include "PointTypes.h"
#include "CephalometricAnalyzer.h"

#include <opencv2/core.hpp>
#include <string>
#include <vector>

// Odpowiada wyłącznie za rysowanie na cv::Mat.

class Renderer
{
public:
    static void drawStartMenu(cv::Mat &canvas, const std::vector<std::string> &availableScans);

    // ── widok podczas klikania (kalibracja + badanie) ─────────────────────────
    static void drawWorkspace(cv::Mat &canvas, ProgramState state, const std::string &currentPointName, float pixelToMmScale);

    static cv::Mat buildResultsView(const cv::Mat &baseImage, const std::vector<Landmark> &expertLandmarks, const std::vector<std::string> &reportLines, bool diagnosticMatch);
    static void drawUserPoint(cv::Mat &img, cv::Point2f pos, const std::string &label);
    static void drawErrorLines(cv::Mat &img, const std::vector<PointPair> &points);
    static void drawTransparentCircle(cv::Mat &img, cv::Point center, int radius, cv::Scalar color, double alpha);

private:
    static void drawRuler(cv::Mat &canvas, float pixelToMmScale);
    static void drawCalibrationPattern(cv::Mat &canvas);
    static void drawReportPanel(cv::Mat &finalCanvas, int panelStartX, const std::vector<std::string> &reportLines, bool diagnosticMatch);
};