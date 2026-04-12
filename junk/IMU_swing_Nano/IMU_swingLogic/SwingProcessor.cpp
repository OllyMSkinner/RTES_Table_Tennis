#include "SwingProcessor.hpp"

#include <cmath>
#include <cstdio>

static constexpr float RAD_TO_DEG = 180.0f / 3.14159265f;

void SwingProcessor::setMagnitudeCallback(MagnitudeCallback cb)
{
    mag_callback_ = std::move(cb);
}

SwingProcessor::SwingProcessor(float accel_deadband, int calib_samples, int recal_samples)
    : accel_deadband_(accel_deadband),
      calibrator_(calib_samples, recal_samples)
{
    calibrator_.setCallback([](float /*bx*/, float /*by*/, float /*bz*/, bool initial) {
        if (initial)
            std::printf("Ready — swing detection active.\n");
        else
            std::printf("Bias recalibrated.\n");
    });

    detector_.setCallback([this](bool upright) {
        if (upright) {
            std::printf("Start position detected — recalibrating bias...\n");
            calibrator_.triggerRecal();
        }
    });
}

void SwingProcessor::operator()(float ax, float ay, float az,
                                float gx, float gy, float gz,
                                float /*mx*/, float /*my*/)
{
    if (calibrator_.feed(ax, ay, az)) return;

    detector_.onSample(ax, ay, az, 0.f, 0.f);

    float lin_ax = ax - calibrator_.biasX();
    float lin_ay = ay - calibrator_.biasY();
    float lin_az = az - calibrator_.biasZ();

    float lin_mag = std::sqrt(lin_ax*lin_ax + lin_ay*lin_ay + lin_az*lin_az);
    if (lin_mag < accel_deadband_) lin_mag = 0.f;

    float gyro_deg_s = std::sqrt(gx*gx + gy*gy + gz*gz) * RAD_TO_DEG;

    std::printf("accel: %8.4f m/s²  |  gyro: %7.2f deg/s\n", lin_mag, gyro_deg_s);

    if (mag_callback_) mag_callback_(lin_mag);
}
