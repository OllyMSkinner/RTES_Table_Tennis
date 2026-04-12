#pragma once

#include "SwingCalibrator.hpp"
#include "positiondetector.hpp"

#include <chrono>

// Processes every IMU sample: calibrates gravity bias, detects start position,
// prints linear acceleration magnitude and cumulative rotation angle.
//
// Usage in main.cpp:
//   SwingProcessor processor(ACCEL_DEADBAND, CALIB_SAMPLES, RECAL_SAMPLES);
//   reader.setCallback([&](float ax, float ay, float az,
//                          float gx, float gy, float gz,
//                          float mx, float my) {
//       processor(ax, ay, az, gx, gy, gz, mx, my);
//   });

class SwingProcessor {
public:
    explicit SwingProcessor(float accel_deadband = 0.05f,
                            int   calib_samples  = 200,
                            int   recal_samples  = 50);

    // Called by IMUReader on every sample.
    void operator()(float ax, float ay, float az,
                    float gx, float gy, float gz,
                    float mx, float my);

private:
    float            accel_deadband_;
    float            gyro_deadband_;   // deg/s – zero out gyro noise below this
    float            angle_x_;         // integrated rotation per axis (deg)
    float            angle_y_;
    float            angle_z_;
    SwingCalibrator  calibrator_;
    PositionDetector detector_;

    using Clock = std::chrono::steady_clock;
    Clock::time_point last_time_;
    bool              first_sample_;
};
