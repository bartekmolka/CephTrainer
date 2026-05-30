#pragma once

#include <string>
#include <opencv2/core.hpp>

struct PointPair
{
    std::string name;
    cv::Point2f userPoint;
    cv::Point2f groundTruthPoint;
};

struct Landmark
{
    std::string symbol;
    int x;
    int y;
};

enum ProgramState
{
    STATE_WAITING_FOR_FILE,
    STATE_CALIBRATING,
    STATE_CLINICAL_TEST,
    STATE_RESULTS
};