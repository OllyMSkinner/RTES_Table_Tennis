#include "positiondetector.hpp"
#include "imureader.hpp"
#include "rpi_pwm.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <sys/signalfd.h>
#include <thread>
#include <unistd.h>

// ── Dot product → duty cycle ──────────────────────────────────────────────────

static float dotToDutyCycle(float dot, float minDot)
{
    float absDot = std::abs(dot);
    if (absDot < minDot) return 0.0f;
    return (absDot - minDot) / (1.0f - minDot) * 100.0f;
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    // ── PWM ───────────────────────────────────────────────────────────────
    RPI_PWM pwm;
    if (!pwm.start()) {
        std::cerr << "Failed to start PWM\n";
        return EXIT_FAILURE;
    }

    // ── Startup buzz ──────────────────────────────────────────────────────
    std::cout << "Startup buzz — 10 seconds\n";
    pwm.setDutyCycle(100.0f);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    pwm.setDutyCycle(0.0f);
    std::cout << "Startup buzz done\n";

    // ── Position detector ─────────────────────────────────────────────────
    PositionDetector::Config cfg;
    // cfg.minDot = 0.90f;          // adjust if needed
    // cfg.stabilitySamples = 50;   // adjust if needed
    PositionDetector detector(cfg);

    detector.setCallback([&](bool inPosition) {
        if (inPosition) {
            std::cout << "IN position — buzzing\n";
        } else {
            pwm.setDutyCycle(0.0f);
            std::cout << "OUT of position — motor off\n";
        }
    });

    // ── IMU ───────────────────────────────────────────────────────────────
    IMUReader reader(1);

    if (!reader.init()) {
        std::cerr << "IMUReader init() failed\n";
        pwm.setDutyCycle(0.0f);
        return EXIT_FAILURE;
    }
    std::cout << "IMU init OK\n";

    // ── Event callback ────────────────────────────────────────────────────
    reader.setCallback([&](float ax, float ay, float az, float mx, float my) {

        float g = std::sqrt(ax * ax + ay * ay + az * az);

        float dot = 0.0f;
        bool accelInRange = (g >= cfg.minG && g <= cfg.maxG);

        if (accelInRange) {
            float nx = ax / g;
            float ny = ay / g;
            float nz = az / g;
            dot = nx * cfg.refX + ny * cfg.refY + nz * cfg.refZ;
        }

        detector.onSample(ax, ay, az, mx, my);

        float duty = dotToDutyCycle(dot, cfg.minDot);
        pwm.setDutyCycle(duty);

        std::cout << "dot=" << dot << "  duty=" << duty << "%\n";
    });

    reader.start();

    // ── Signal handling ───────────────────────────────────────────────────
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);

    if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
        reader.stop();
        pwm.setDutyCycle(0.0f);
        return EXIT_FAILURE;
    }

    int sfd = signalfd(-1, &mask, 0);
    if (sfd == -1) {
        reader.stop();
        pwm.setDutyCycle(0.0f);
        return EXIT_FAILURE;
    }

    std::cout << "Streaming — press Ctrl+C to stop\n";

    bool running = true;
    while (running) {
        signalfd_siginfo fdsi;
        ssize_t s = ::read(sfd, &fdsi, sizeof(fdsi));
        if (s != sizeof(fdsi)) break;
        if (fdsi.ssi_signo == SIGINT) running = false;
    }

    std::cout << "\nStopping...\n";
    ::close(sfd);
    reader.stop();
    pwm.setDutyCycle(0.0f);
    std::cout << "Done\n";

    return EXIT_SUCCESS;
}