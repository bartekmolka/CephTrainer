#pragma once

#include "PointTypes.h"
#include "DataManager.h"
#include "PatientLoader.h"
#include "CephalometricAnalyzer.h"
#include "Renderer.h"

#include <opencv2/highgui.hpp>
#include <string>
#include <vector>

// CephTrainer jest wyłącznie kontrolerem:
//   - przechowuje aktualny stan aplikacji
//   - reaguje na eventy myszy i deleguje pracę do specjalistycznych klas
//   - wywołuje Renderer::* do wyświetlania

class CephTrainer
{
public:
    CephTrainer();
    void initWindow();

private:
    DataManager m_data;
    PatientLoader m_loader;

    ProgramState m_state;
    std::string m_windowName;

    std::string m_currentScanName;
    std::vector<std::string> m_availableScans;

    cv::Mat m_originalImage;
    cv::Mat m_displayImage;

    std::vector<PointPair> m_points;
    std::vector<Landmark> m_expertLandmarks;
    size_t m_currentPointIndex;

    float m_pixelToMmScale;
    int m_cvmStage;

    // Wynik analizy – wypełniany po kliknięciu ostatniego punktu
    AnalysisResult m_analysisResult;

    void loadScan(const std::string &scanName);
    void handleLandmarkClick(int x, int y);
    void finalizeSession();
    void updateScreen();

    static void onMouse(int event, int x, int y, int flags, void *userdata);
    void handleMouseEvent(int event, int x, int y);
};