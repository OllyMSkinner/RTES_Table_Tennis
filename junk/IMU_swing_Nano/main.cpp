#include "IMU_swingLogic/SwingProcessor.hpp"
#include "IMU_swingLogic/SwingDetector.hpp"
#include "imureader.hpp"

#include <cstdio>
#include <cstdlib>

#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>

static constexpr unsigned I2C_BUS        = 1;
static constexpr int      CALIB_SAMPLES  = 200;
static constexpr int      RECAL_SAMPLES  = 50;
static constexpr float    ACCEL_DEADBAND = 0.05f;

int main()
{
    icm20948::settings imu_settings;
    imu_settings.magn.mode = icm20948::MAGN_SHUTDOWN;
    IMUReader reader(I2C_BUS, ICM20948_I2C_ADDR,
                     "/dev/gpiochip0", 27, imu_settings);

    if (!reader.init()) {
        std::fprintf(stderr, "ERROR: IMU init failed\n");
        return EXIT_FAILURE;
    }

    SwingDetector swing_detector;
    swing_detector.setCallback([](const char* level) {
        std::printf(">> %s\n", level);
    });

    SwingProcessor processor(ACCEL_DEADBAND, CALIB_SAMPLES, RECAL_SAMPLES);
    processor.setMagnitudeCallback([&swing_detector](float mag) {
        swing_detector.detect(mag);
    });
    std::printf("Calibrating — keep device still for ~2 s...\n");
    reader.setCallback([&processor](float ax, float ay, float az,
                                    float gx, float gy, float gz,
                                    float mx, float my) {
        processor(ax, ay, az, gx, gy, gz, mx, my);
    });
    reader.start();

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigprocmask(SIG_BLOCK, &mask, nullptr);

    int sfd = signalfd(-1, &mask, 0);

    signalfd_siginfo fdsi;
    while (true) {
        ssize_t s = ::read(sfd, &fdsi, sizeof(fdsi));
        if (s != static_cast<ssize_t>(sizeof(fdsi))) break;
        if (fdsi.ssi_signo == SIGINT || fdsi.ssi_signo == SIGHUP) break;
    }

    ::close(sfd);
    reader.stop();

    return EXIT_SUCCESS;
}
