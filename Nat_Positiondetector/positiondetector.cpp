#include "positiondetector.h"

#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <atomic>

#include "icm20948_i2c.hpp"

// Number of stable samples required for upright confirmation
static int stabilitySamplesRequired = 20;

namespace PositionDetectorName {

void PositionDetector::CheckPosition(float X, float Y, float Z)
{
    bool isUpright =
    (Z < -8.5f) &&
    (std::fabs(X) < 3.0f) &&
    (std::fabs(Y) < 3.0f);

    std::cout << "X=" << X
              << " Y=" << Y
              << " Z=" << Z
              << " | isUpright=" << isUpright
              << " | Counter=" << Counter
              << " | Confirmed=" << UprightConfirmed
              << std::endl;

    if (isUpright) {
        Counter++;

        if (Counter >= stabilitySamplesRequired && !UprightConfirmed) {
            UprightConfirmed = true;

            ::std::cout << "[EVENT] Upright position detected!" << ::std::endl;

            if (callback) {
                callback->OnUprightDetected();
            }
        }
    } else {
        Counter = 0;
        UprightConfirmed = false;
    }
}

bool PositionDetector::RunFromIMU(unsigned bus,
                                  unsigned addr,
                                  ::std::atomic_bool& runFlag,
                                  int samplePeriodMs)
{
    icm20948::ICM20948_I2C imu(bus, addr);

    if (!imu.init()) {
        ::std::cerr << "ICM20948 init() failed (bus=" << bus
                    << ", addr=0x" << ::std::hex << addr << ::std::dec << ")\n";
        return false;
    }

    const int targetConfirmMs = 200;
    stabilitySamplesRequired = ::std::max(1, targetConfirmMs / ::std::max(1, samplePeriodMs));

    ::std::cout << "PositionDetector streaming IMU (Ctrl+C handled by caller)\n";
    ::std::cout << "Upright confirm: " << stabilitySamplesRequired
                << " samples (~" << (stabilitySamplesRequired * samplePeriodMs) << " ms)\n";

    while (runFlag.load()) {
        const bool ok = imu.read_accel_gyro();
        if (ok) {
            CheckPosition(imu.accel[0], imu.accel[1], imu.accel[2]);
            
        } else {
            Counter = 0;
            UprightConfirmed = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(samplePeriodMs));
    }

    return true;
}

} // namespace PositionDetectorName
