#include "CephTrainer.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

CephTrainer::CephTrainer()
    : m_state(STATE_WAITING_FOR_FILE),
      m_windowName("CephTrainer 2026"),
      m_currentPointIndex(0),
      m_pixelToMmScale(0.f),
      m_cvmStage(0)
{
    m_availableScans = m_data.scanCephalograms();
}

void CephTrainer::initWindow()
{
    cv::namedWindow(m_windowName, cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback(
        m_windowName,
        CephTrainer::onMouse,
        this);

    updateScreen();

    while (true)
    {
        int key = cv::waitKey(30);

        if (key == 27)
            break;

        if (key == 'r' || key == 'R')
        {
            m_state = STATE_WAITING_FOR_FILE;

            m_currentScanName.clear();

            m_originalImage.release();
            m_displayImage.release();

            m_points.clear();
            m_expertLandmarks.clear();

            m_currentPointIndex = 0;
            m_pixelToMmScale = 0.f;
            m_cvmStage = 0;

            m_analysisResult = {};

            updateScreen();
        }

        if (m_state == STATE_WAITING_FOR_FILE)
        {
            if (key >= '1' && key <= '9')
            {
                int idx = key - '1';

                if (idx >= 0 &&
                    idx < static_cast<int>(m_availableScans.size()))
                {
                    loadScan(m_availableScans[idx]);
                }
            }
        }
    }

    cv::destroyAllWindows();
}

void CephTrainer::loadScan(const std::string &scanName)
{
    m_currentScanName = scanName;

    PatientData patient = m_loader.load(
        m_data.imagePath(scanName),
        m_data.landmarkPath(scanName),
        m_data.cvmPath(scanName));

    if (!patient.valid)
    {
        std::cerr << "ERROR: Failed to load patient: " << scanName << std::endl;
        return;
    }

    m_originalImage = patient.image.clone();
    m_displayImage = patient.image.clone();

    m_points = patient.points;
    m_expertLandmarks = patient.expertLandmarks;

    m_cvmStage = patient.cvmStage;

    m_currentPointIndex = 0;
    m_pixelToMmScale = 0.f;

    m_analysisResult = {};

    m_state = STATE_CALIBRATING;

    updateScreen();
}

void CephTrainer::handleLandmarkClick(int x, int y)
{
    if (m_currentPointIndex >= m_points.size())
        return;

    PointPair &point =
        m_points[m_currentPointIndex];

    point.userPoint = cv::Point2f(
        static_cast<float>(x),
        static_cast<float>(y));

    Renderer::drawUserPoint(
        m_displayImage,
        point.userPoint,
        point.name);

    ++m_currentPointIndex;

    if (m_currentPointIndex == 2)
    {
        m_pixelToMmScale =
            CephalometricAnalyzer::computePixelToMmScale(
                m_points);

        m_state = STATE_CLINICAL_TEST;
    }

    if (m_currentPointIndex >= m_points.size())
    {
        finalizeSession();
        return;
    }

    updateScreen();
}

void CephTrainer::finalizeSession()
{
    m_analysisResult =
        CephalometricAnalyzer::analyze(
            m_points,
            m_pixelToMmScale,
            m_cvmStage);

    Renderer::drawErrorLines(
        m_displayImage,
        m_points);

    m_displayImage =
        Renderer::buildResultsView(
            m_displayImage,
            m_expertLandmarks,
            m_analysisResult.reportLines,
            m_analysisResult.diagnosticMatch);

    m_state = STATE_RESULTS;

    updateScreen();
}

void CephTrainer::updateScreen()
{
    cv::Mat canvas;

    if (m_state == STATE_WAITING_FOR_FILE)
    {
        canvas = cv::Mat(
            700,
            1000,
            CV_8UC3,
            cv::Scalar(30, 30, 30));

        Renderer::drawStartMenu(
            canvas,
            m_availableScans);

        cv::putText(
            canvas,
            "CLICK ON A PATIENT OR PRESS 1-9",
            cv::Point(240, 670),
            cv::FONT_HERSHEY_SIMPLEX,
            0.6,
            cv::Scalar(0, 255, 255),
            2);
    }
    else
    {
        canvas = m_displayImage.clone();

        std::string currentPointName;

        if (m_currentPointIndex < m_points.size())
        {
            currentPointName =
                m_points[m_currentPointIndex].name;
        }

        Renderer::drawWorkspace(
            canvas,
            m_state,
            currentPointName,
            m_pixelToMmScale);
    }

    cv::imshow(m_windowName, canvas);
}

void CephTrainer::onMouse(
    int event,
    int x,
    int y,
    int flags,
    void *userdata)
{
    CephTrainer *trainer =
        static_cast<CephTrainer *>(userdata);

    if (trainer)
    {
        trainer->handleMouseEvent(
            event,
            x,
            y);
    }
}

void CephTrainer::handleMouseEvent(
    int event,
    int x,
    int y)
{
    if (event != cv::EVENT_LBUTTONDOWN)
        return;

    if (m_state == STATE_WAITING_FOR_FILE)
    {
        int startY = 160;

        for (size_t i = 0;
             i < m_availableScans.size() && i < 7;
             ++i)
        {
            int top = startY + static_cast<int>(i) * 70;
            int bottom = top + 50;

            bool insideX =
                (x >= 150 && x <= 650);

            bool insideY =
                (y >= top && y <= bottom);

            if (insideX && insideY)
            {
                loadScan(m_availableScans[i]);
                return;
            }
        }
    }

    if (m_state == STATE_CALIBRATING ||
        m_state == STATE_CLINICAL_TEST)
    {
        handleLandmarkClick(x, y);
    }
}