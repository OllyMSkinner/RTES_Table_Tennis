#include "SwingProcessor.hpp"

#include <cmath>
#include <cstdio>

static constexpr float RAD_TO_DEG    = 180.0f / 3.14159265f;
static constexpr float GYRO_DEADBAND = 2.0f;   // deg/s – zero below this when still

SwingProcessor::SwingProcessor(float accel_deadband, int calib_samples, int recal_samples)
    : accel_deadband_(accel_deadband),
      gyro_deadband_(GYRO_DEADBAND),
      angle_x_(0.f), angle_y_(0.f), angle_z_(0.f),
      calibrator_(calib_samples, recal_samples),
      first_sample_(true)
{
    calibrator_.setCallback([](float bx, float by, float bz, bool initial) {
        if (initial)
            std::printf("Ready — return to start position between swings to recalibrate.\n");
        else
            std::printf("Bias updated (bx=%.4f by=%.4f bz=%.4f).\n", bx, by, bz);
    });

    detector_.setCallback([this](bool upright) {
        if (upright) {
            std::printf("Start position detected — recalibrating bias, resetting angle.\n");
            calibrator_.triggerRecal();
            angle_x_ = angle_y_ = angle_z_ = 0.f;  // reset on return to start
        }
    });
}

void SwingProcessor::operator()(float ax, float ay, float az,
                                float gx, float gy, float gz,
                                float /*mx*/, float /*my*/)
{
    // ── timing ────────────────────────────────────────────────────────────
    auto now = Clock::now();
    float dt = 0.f;
    if (!first_sample_)
        dt = std::chrono::duration<float>(now - last_time_).count();
    last_time_    = now;
    first_sample_ = false;

    // ── calibration ───────────────────────────────────────────────────────
    if (calibrator_.feed(ax, ay, az)) return;

    // ── position detection ────────────────────────────────────────────────
    detector_.onSample(ax, ay, az, 0.f, 0.f);

    // ── linear acceleration magnitude ─────────────────────────────────────
    float lin_ax = ax - calibrator_.biasX();
    float lin_ay = ay - calibrator_.biasY();
    float lin_az = az - calibrator_.biasZ();

    float lin_mag = std::sqrt(lin_ax*lin_ax + lin_ay*lin_ay + lin_az*lin_az);
    if (lin_mag < accel_deadband_) lin_mag = 0.f;

    // ── integrate per-axis gyro → angle vector relative to start pos ──────
    // Convert to deg/s first, then apply dead-band to suppress noise at rest.
    float gx_deg = gx * RAD_TO_DEG;
    float gy_deg = gy * RAD_TO_DEG;
    float gz_deg = gz * RAD_TO_DEG;

    float gyro_speed = std::sqrt(gx_deg*gx_deg + gy_deg*gy_deg + gz_deg*gz_deg);
    if (gyro_speed > gyro_deadband_) {
        angle_x_ += gx_deg * dt;
        angle_y_ += gy_deg * dt;
        angle_z_ += gz_deg * dt;
    }

    // Magnitude of the angle vector = total rotation from start position.
    // This can decrease when the paddle moves back toward start.
    float angle_mag = std::sqrt(angle_x_*angle_x_ + angle_y_*angle_y_ + angle_z_*angle_z_);

    std::printf("accel: %8.4f m/s²  |  gyro: %7.2f deg/s  |  angle: %7.2f deg\n",
                lin_mag, gyro_speed, angle_mag);
}
