#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <csignal>

#include "icm20948_i2c.hpp"

static volatile std::sig_atomic_t g_run = 1;
static void on_sigint(int) { g_run = 0; }

int main(int argc, char** argv)
{
    std::signal(SIGINT, on_sigint);

    unsigned bus = 1;
    unsigned addr = ICM20948_I2C_ADDR; // from icm20948_defs.hpp (typically 0x68)

    // Optional CLI: ./imu_coords [bus] [addr_hex]
    if (argc >= 2) bus = static_cast<unsigned>(std::stoul(argv[1]));
    if (argc >= 3) addr = static_cast<unsigned>(std::stoul(argv[2], nullptr, 16));

    icm20948::ICM20948_I2C imu(bus, addr);

    if (!imu.init()) {
        std::cerr << "ICM20948 init() failed (bus=" << bus << ", addr=0x"
                  << std::hex << addr << std::dec << ")\n";
        return 1;
    }

    std::cout << "Streaming ICM20948 (Ctrl+C to stop)\n";
    std::cout << "Units: accel m/s^2 | gyro rad/s | magn uT\n";

    std::cout << std::fixed << std::setprecision(3);

    while (g_run) {
        const bool ok_ag = imu.read_accel_gyro();
        const bool ok_m  = imu.read_magn();   // if magnetometer isn’t enabled, this may return false

        if (ok_ag) {
            std::cout
                << "ACC ["
                << imu.accel[0] << ", " << imu.accel[1] << ", " << imu.accel[2] << "]  "
                << "GYR ["
                << imu.gyro[0]  << ", " << imu.gyro[1]  << ", " << imu.gyro[2]  << "]  ";
        } else {
            std::cout << "ACC/GYR [read failed]  ";
        }

        if (ok_m) {
            std::cout
                << "MAG ["
                << imu.magn[0]  << ", " << imu.magn[1]  << ", " << imu.magn[2]  << "]";
        } else {
            std::cout << "MAG [read failed]";
        }

        std::cout << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}