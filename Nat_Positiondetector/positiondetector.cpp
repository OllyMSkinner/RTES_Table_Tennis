#include "Positiondetector.h"

#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>

#include "icm20948_i2c.hpp"   // <-- uses your ICM20948 driver

// Number of stable samples required for upright confirmation
static int stabilitySamplesRequired = 20;

namespace PositionDetectorName {

    // Existing: consumes accel values already provided
    void PositionDetector::CheckPosition(float X, float Y, float Z)
    {
        // Upright detection using gravity (units assumed: m/s^2)
        const bool isUpright =
            (std::fabs(X) < 1.0f) &&
            (std::fabs(Y) < 1.0f) &&
            (Z > 8.5f);

        if (isUpright) {
            Counter++;

            if (Counter >= stabilitySamplesRequired && !UprightConfirmed) {
                UprightConfirmed = true;

                // Print once per upright episode
                std::cout << "[EVENT] Upright position detected!" << std::endl;

                // Fire callback once per upright episode
                if (callback) {
                    callback->OnUprightDetected();
                }
            }
        } else {
            Counter = 0;
            UprightConfirmed = false;
        }
    }

    // NEW: read IMU internally and feed accel into CheckPosition()
    //
    // This mirrors your main.cpp loop, but lives inside PositionDetector.
    // You will need the matching declaration in Positiondetector.h (see below).
    bool PositionDetector::RunFromIMU(unsigned bus,
                                     unsigned addr,
                                     std::atomic_bool& runFlag,
                                     int samplePeriodMs)
    {
        icm20948::ICM20948_I2C imu(bus, addr);

        if (!imu.init()) {
            std::cerr << "ICM20948 init() failed (bus=" << bus << ", addr=0x"
                      << std::hex << addr << std::dec << ")\n";
            return false;
        }

        // Optional: align stability sample count with loop rate
        // (keeps ~200ms confirmation if you change samplePeriodMs)
        // 200ms / samplePeriodMs = required samples
        const int targetConfirmMs = 200;
        stabilitySamplesRequired = std::max(1, targetConfirmMs / std::max(1, samplePeriodMs));

        std::cout << "PositionDetector streaming IMU (Ctrl+C handled by caller)\n";
        std::cout << "Upright confirm: " << stabilitySamplesRequired
                  << " samples (~" << (stabilitySamplesRequired * samplePeriodMs) << " ms)\n";

        while (runFlag.load()) {
            const bool ok = imu.read_accel_gyro();
            if (ok) {
                // accel[] is in m/s^2 per your main.cpp printout
                CheckPosition(imu.accel[0], imu.accel[1], imu.accel[2]);
            } else {
                // If reads fail, treat as not upright (forces reset)
                Counter = 0;
                UprightConfirmed = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(samplePeriodMs));
        }

        return true;
    }

} // namespace PositionDetectorName
