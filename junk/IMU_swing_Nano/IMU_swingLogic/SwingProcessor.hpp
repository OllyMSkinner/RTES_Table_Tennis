#pragma once

#include "SwingCalibrator.hpp"
#include "positiondetector.hpp"

#include <functional>

// Processes every IMU sample: calibrates gravity bias, detects start position,
// prints linear acceleration magnitude and angular velocity magnitude.
// Also fires an optional magnitude callback so SwingDetector can be wired in
// from main.cpp.

class SwingProcessor {
public:
    using MagnitudeCallback = std::function<void(float accel_mag)>;

    explicit SwingProcessor(float accel_deadband = 0.05f,
                            int   calib_samples  = 200,
                            int   recal_samples  = 50);

    // Optional: called with the linear accel magnitude on each live sample.
    void setMagnitudeCallback(MagnitudeCallback cb);

    // Called by IMUReader on every sample.
    void operator()(float ax, float ay, float az,
                    float gx, float gy, float gz,
                    float mx, float my);

private:
    float              accel_deadband_;
    SwingCalibrator    calibrator_;
    PositionDetector   detector_;
    MagnitudeCallback  mag_callback_;
};
