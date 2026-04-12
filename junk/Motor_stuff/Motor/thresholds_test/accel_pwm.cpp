/**
 * accel_pwm.cpp
 *
 * Event-driven position → PWM motor buzz controller.
 * Uses PositionDetector to determine if the bat is in the correct position.
 *
 * Buzz intensity is proportional to how close the position is to perfect:
 *   - Perfect alignment (dot = 1.0) → 100% buzz
 *   - Close to threshold edge (dot = minDot) → low buzz
 *   - Outside threshold → no buzz
 */

#include "rpi_pwm.h"
#include "IMU_processing/IMUPublisher/IMUPublisher.hpp"
#include "positiondetector.hpp"

#include <atomic>
#include <cmath>
#include <csignal>
#include <iostream>
#include <unistd.h>

// ── Hardware configuration ────────────────────────────────────────────────────

static constexpr int   PWM_CHANNEL = 2;      // GPIO18
static constexpr float PWM_FREQ_HZ = 200.0f;

// ── Signal handling ───────────────────────────────────────────────────────────

static std::atomic<bool> g_running{true};
static void signalHandler(int) { g_running = false; }

// ── Dot product → duty cycle ──────────────────────────────────────────────────
/**
 * Maps the dot product linearly onto duty cycle.
 *
 * dot = 1.0  (perfect alignment) → 100%
 * dot = minDot (edge of threshold) → 0%
 * dot < minDot (outside threshold) → 0%
 */
static float dotToDutyCycle(float dot, float minDot)
{
    if (dot < minDot) return 0.0f;
    return (dot - minDot) / (1.0f - minDot) * 100.0f;
}

// ── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char** argv)
{
    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    unsigned    bus      = 1;
    unsigned    addr     = ICM20948_I2C_ADDR;
    const char* gpiochip = "/dev/gpiochip0";
    unsigned    rdy_line = 27;

    if (argc >= 2) bus      = static_cast<unsigned>(std::stoul(argv[1]));
    if (argc >= 3) addr     = static_cast<unsigned>(std::stoul(argv[2], nullptr, 16));
    if (argc >= 4) gpiochip = argv[3];
    if (argc >= 5) rdy_line = static_cast<unsigned>(std::stoul(argv[4]));

    // ── PWM ───────────────────────────────────────────────────────────────
    RPI_PWM pwm;
    pwm.start(PWM_CHANNEL, PWM_FREQ_HZ);
    pwm.setDutyCycle(0.0f);
    std::cout << "PWM started on channel " << PWM_CHANNEL << "\n";

    // ── Position detector ─────────────────────────────────────────────────
    PositionDetector::Config cfg;
    // cfg.minDot = 0.90f;          // adjust if needed
    // cfg.stabilitySamples = 50;   // adjust if needed
    PositionDetector detector(cfg);

    // Track the latest dot product so we can use it for proportional buzz
    std::atomic<float> latestDot{0.0f};

    // Callback fires only when confirmed state changes (in/out of position)
    detector.setCallback([&pwm, &latestDot](bool inPosition) {
        if (inPosition) {
            std::cout << "IN position — buzzing\n";
        } else {
            pwm.setDutyCycle(0.0f);
            std::cout << "OUT of position — motor off\n";
        }
    });

    // ── IMU ───────────────────────────────────────────────────────────────
    icm20948::IMUPublisher imuPublisher(bus, addr, gpiochip, rdy_line);

    if (!imuPublisher.init()) {
        std::cerr << "ICM20948 init() failed\n";
        pwm.setDutyCycle(0.0f);
        return 1;
    }
    std::cout << "IMU init OK\n";

    // ── Event callback ────────────────────────────────────────────────────
    imuPublisher.registerCallback(
        [&pwm, &detector, &cfg, &latestDot](const icm20948::IMUSample& sample) {

            // 1. Compute dot product manually to get proportional value
            float g = std::sqrt(sample.ax * sample.ax +
                                sample.ay * sample.ay +
                                sample.az * sample.az);

            float dot = 0.0f;
            bool accelInRange = (g >= cfg.minG && g <= cfg.maxG);

            if (accelInRange) {
                float nx = sample.ax / g;
                float ny = sample.ay / g;
                float nz = sample.az / g;
                dot = nx * cfg.refX + ny * cfg.refY + nz * cfg.refZ;
            }

            latestDot = dot;

            // 2. Feed into position detector (handles stability + heading check)
            detector.onSample(sample.ax, sample.ay, sample.az,
                              sample.mx, sample.my);

            // 3. Set duty cycle proportional to how close to perfect position
            float duty = dotToDutyCycle(dot, cfg.minDot);
            pwm.setDutyCycle(duty);

            std::cout << "dot=" << dot
                      << "  duty=" << duty << "%\n";
        }
    );

    if (!imuPublisher.start()) {
        std::cerr << "IMUPublisher start() failed\n";
        pwm.setDutyCycle(0.0f);
        return 1;
    }

    std::cout << "Streaming — press Ctrl+C to stop\n";

    while (g_running) {
        pause();
    }

    std::cout << "\nStopping...\n";
    imuPublisher.stop();
    pwm.setDutyCycle(0.0f);
    std::cout << "Done\n";

    return 0;
}
