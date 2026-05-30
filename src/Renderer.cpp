#include "Renderer.h"

#include <opencv2/imgproc.hpp>

namespace Style
{
    constexpr int FONT = cv::FONT_HERSHEY_SIMPLEX;
    const cv::Scalar CYAN = {0, 255, 255};
    const cv::Scalar WHITE = {255, 255, 255};
    const cv::Scalar RED = {0, 0, 255};
    const cv::Scalar GREEN = {0, 255, 0};
    const cv::Scalar DARK_GRAY = {22, 22, 22};
    const cv::Scalar MID_GRAY = {50, 50, 50};
    const cv::Scalar LIGHT_GRAY = {150, 150, 150};
}

void Renderer::drawStartMenu(cv::Mat &canvas, const std::vector<std::string> &availableScans)
{
    cv::putText(canvas, "CEPHALOMETRIC WORKSTATION v2.0", cv::Point(180, 60), Style::FONT, 0.8, Style::CYAN, 2);

    if (availableScans.empty())
    {
        cv::putText(canvas, "NO PATIENT FILES FOUND IN 'data/Cephalograms/' !", cv::Point(120, 200), Style::FONT, 0.6, Style::RED, 2);
        cv::putText(canvas, "Please check your folders or CEPH_DATA_DIR env.", cv::Point(140, 240), Style::FONT, 0.5, Style::LIGHT_GRAY, 1);
        return;
    }

    cv::putText(canvas, "Select patient scan from the detected list to begin:", cv::Point(180, 110), Style::FONT, 0.5, {200, 200, 200}, 1);

    int y = 160;
    for (size_t i = 0; i < availableScans.size() && i < 7; ++i)
    {
        cv::rectangle(canvas, cv::Point(150, y), cv::Point(650, y + 50), Style::MID_GRAY, -1);
        cv::rectangle(canvas, cv::Point(150, y), cv::Point(650, y + 50), Style::GREEN, 2);

        const std::string label = "LOAD: Patient Scan -> [" + availableScans[i] + "]";
        cv::putText(canvas, label, cv::Point(240, y + 32), Style::FONT, 0.55, Style::WHITE, 1, cv::LINE_AA);
        y += 70;
    }
}

void Renderer::drawWorkspace(cv::Mat &canvas, ProgramState state, const std::string &currentPointName, float pixelToMmScale)
{
    if (state == STATE_CALIBRATING)
        drawCalibrationPattern(canvas);

    drawRuler(canvas, pixelToMmScale);

    if (state == STATE_CALIBRATING || state == STATE_CLINICAL_TEST)
    {
        const std::string instruction = "TARGET: " + currentPointName;
        cv::rectangle(canvas, cv::Point(10, 10), cv::Point(450, 55), {0, 0, 0}, -1);
        cv::putText(canvas, instruction, cv::Point(20, 40), Style::FONT, 0.75, Style::CYAN, 2);
    }
}

cv::Mat Renderer::buildResultsView(const cv::Mat &baseImage, const std::vector<Landmark> &expertLandmarks, const std::vector<std::string> &reportLines, bool diagnosticMatch)
{
    cv::Mat canvas = baseImage.clone();

    for (const auto &pt : expertLandmarks)
    {
        cv::circle(canvas, cv::Point(pt.x, pt.y), 3, Style::CYAN, -1);
        cv::putText(canvas, pt.symbol, cv::Point(pt.x + 5, pt.y - 5), Style::FONT, 0.4, Style::WHITE, 1, cv::LINE_AA);
    }

    cv::Mat finalCanvas;
    cv::copyMakeBorder(canvas, finalCanvas, 0, 0, 0, 700, cv::BORDER_CONSTANT, Style::DARK_GRAY);

    drawReportPanel(finalCanvas, canvas.cols + 25, reportLines, diagnosticMatch);
    return finalCanvas;
}

void Renderer::drawUserPoint(cv::Mat &img, cv::Point2f pos, const std::string &label)
{
    drawTransparentCircle(img, cv::Point(pos), 8, Style::RED, 0.5);
    cv::putText(img, label, cv::Point(static_cast<int>(pos.x) + 15, static_cast<int>(pos.y) + 5), Style::FONT, 0.6, Style::RED, 2);
}

void Renderer::drawErrorLines(cv::Mat &img, const std::vector<PointPair> &points)
{
    for (size_t i = 2; i < points.size(); ++i)
    {
        cv::circle(img, points[i].groundTruthPoint, 6, Style::GREEN, -1);
        cv::line(img, points[i].userPoint, points[i].groundTruthPoint, cv::Scalar(255, 255, 0), 2);
    }
}

void Renderer::drawTransparentCircle(cv::Mat &img, cv::Point center, int radius, cv::Scalar color, double alpha)
{
    cv::Mat overlay = img.clone();
    cv::circle(overlay, center, radius, color, -1);
    cv::addWeighted(overlay, alpha, img, 1.0 - alpha, 0, img);
}

void Renderer::drawRuler(cv::Mat &canvas, float pixelToMmScale)
{
    const float scale = (pixelToMmScale > 0) ? pixelToMmScale : 0.1f;
    const int widthPx = static_cast<int>(20.f / scale);
    const int rX = 40;
    const int rY = canvas.rows - 40;
    const int halfW = widthPx / 2;

    cv::rectangle(canvas, cv::Point(rX - 10, rY - 25), cv::Point(rX + widthPx + 10, rY + 15), {0, 0, 0}, -1);

    cv::line(canvas, {rX, rY}, {rX + widthPx, rY}, Style::WHITE, 2);
    cv::line(canvas, {rX, rY - 8}, {rX, rY + 8}, Style::WHITE, 2);
    cv::line(canvas, {rX + halfW, rY - 5}, {rX + halfW, rY + 5}, Style::WHITE, 1);
    cv::line(canvas, {rX + widthPx, rY - 8}, {rX + widthPx, rY + 8}, Style::WHITE, 2);

    cv::putText(canvas, "20 mm", cv::Point(rX + halfW - 20, rY - 12), Style::FONT, 0.45, Style::CYAN, 1, cv::LINE_AA);
}

void Renderer::drawCalibrationPattern(cv::Mat &canvas)
{
    const int lineY = 100;
    const int startX = 50;
    const int endX = 150;

    cv::rectangle(canvas, cv::Point(startX - 10, lineY - 35), cv::Point(endX + 110, lineY + 20), {0, 0, 0}, -1);

    cv::line(canvas, {startX, lineY}, {endX, lineY}, Style::RED, 3);
    cv::circle(canvas, {startX, lineY}, 4, Style::CYAN, -1);
    cv::circle(canvas, {endX, lineY}, 4, Style::CYAN, -1);

    cv::putText(canvas, "<- CLICK HERE FOR P1  &  HERE FOR P2 ->", cv::Point(startX - 5, lineY - 15), Style::FONT, 0.4, Style::CYAN, 1, cv::LINE_AA);
    cv::putText(canvas, "CALIBRATION PATTERN (10mm)", cv::Point(startX, lineY + 15), Style::FONT, 0.4, Style::RED, 1, cv::LINE_AA);
}

void Renderer::drawReportPanel(cv::Mat &finalCanvas, int panelStartX, const std::vector<std::string> &reportLines, bool diagnosticMatch)
{
    int y = 45;
    for (const auto &line : reportLines)
    {
        cv::Scalar color = Style::WHITE;
        int thickness = 1;

        if (line.find("---") != std::string::npos ||
            line.find("===") != std::string::npos)
        {
            color = Style::CYAN;
            thickness = 2;
        }
        else if (line.find("Verdict:") != std::string::npos ||
                 line.find("Plan:") != std::string::npos ||
                 line.find("Growth Status:") != std::string::npos)
        {
            color = diagnosticMatch ? Style::GREEN : Style::RED;
        }

        cv::putText(finalCanvas, line, cv::Point(panelStartX, y),
                    Style::FONT, 0.62, color, thickness, cv::LINE_AA);
        y += 32;
    }
}