#include <iostream>
#include <iomanip>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <string>
#include <mutex>
#include <chrono>

#include <fcntl.h>
#include <unistd.h>

#include "IMU_processing/IMUPublisher/IMUPublisher.hpp"

static volatile std::sig_atomic_t g_run = 1;
static void on_sigint(int) { g_run = 0; }

static int open_with_retry(const char* path, int flags, int retries = 50, int ms = 100)
{
    for (int i = 0; i < retries; ++i) {
        int fd = ::open(path, flags);
        if (fd >= 0) {
            return fd;
        }
        usleep(ms * 1000);
    }
    return -1;
}

class SampleWriter
{
public:
    SampleWriter(int btFd, int csvFd)
        : bt_fd(btFd),
          csv_fd(csvFd),
          t0(std::chrono::steady_clock::now())
    {
    }

    void WriteSample(const icm20948::IMUSample& sample)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto now = std::chrono::steady_clock::now();
        const uint64_t ts_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();

        char line[256];
        const int n = std::snprintf(
            line, sizeof(line),
            "%llu,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
            static_cast<unsigned long long>(ts_ms),
            sample.ax, sample.ay, sample.az,
            sample.gx, sample.gy, sample.gz,
            sample.mx, sample.my, sample.mz
        );

        if (n <= 0) {
            return;
        }

        if (bt_fd >= 0) {
            if (::write(bt_fd, line, static_cast<size_t>(n)) < 0) {
                std::cerr << "Bluetooth write failed\n";
            }
        }

        if (csv_fd >= 0) {
            ::write(csv_fd, line, static_cast<size_t>(n));
        }
    }

private:
    int bt_fd;
    int csv_fd;
    std::mutex mtx;
    std::chrono::steady_clock::time_point t0;
};

int main(int argc, char** argv)
{
    std::signal(SIGINT, on_sigint);

    unsigned bus = 1;
    unsigned addr = ICM20948_I2C_ADDR;
    const char* out_dev = "/dev/rfcomm0";
    const char* csv_path = "pi3_imu_log.csv";
    const char* gpiochip = "/dev/gpiochip0";
    unsigned rdy_line = 17;

    if (argc >= 2) bus = static_cast<unsigned>(std::stoul(argv[1]));
    if (argc >= 3) addr = static_cast<unsigned>(std::stoul(argv[2], nullptr, 16));
    if (argc >= 4) out_dev = argv[3];
    if (argc >= 5) csv_path = argv[4];
    if (argc >= 6) gpiochip = argv[5];
    if (argc >= 7) rdy_line = static_cast<unsigned>(std::stoul(argv[6]));

    std::cout << "Initialising Pi 3 IMU sender\n";
    std::cout << "I2C bus=" << bus
              << " addr=0x" << std::hex << addr << std::dec
              << " BT=" << out_dev
              << " CSV=" << csv_path
              << " RDY GPIO=" << rdy_line
              << "\n";

    int bt_fd = open_with_retry(out_dev, O_WRONLY | O_NOCTTY);
    if (bt_fd < 0) {
        std::cerr << "Failed to open " << out_dev << "\n";
        return 1;
    }

    std::cout << "Bluetooth connected: " << out_dev << "\n";

    int csv_fd = ::open(csv_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (csv_fd < 0) {
        std::cerr << "Warning: failed to open CSV file " << csv_path << "\n";
    }

    const char* hdr = "ts_ms,ax,ay,az,gx,gy,gz,mx,my,mz\n";
    ::write(bt_fd, hdr, std::strlen(hdr));
    if (csv_fd >= 0) {
        ::write(csv_fd, hdr, std::strlen(hdr));
    }

    SampleWriter writer(bt_fd, csv_fd);

    icm20948::IMUPublisher imuPublisher(bus, addr, gpiochip, rdy_line);

    if (!imuPublisher.init()) {
        std::cerr << "ICM20948 init() failed\n";
        if (csv_fd >= 0) ::close(csv_fd);
        ::close(bt_fd);
        return 1;
    }

    std::cout << "IMU init OK\n";

    imuPublisher.registerCallback(
        [&writer](const icm20948::IMUSample& sample) {
            writer.WriteSample(sample);
        }
    );

    if (!imuPublisher.start()) {
        std::cerr << "Failed to start IMU publisher\n";
        if (csv_fd >= 0) ::close(csv_fd);
        ::close(bt_fd);
        return 1;
    }

    std::cout << "Streaming IMU data from Pi 3 via Bluetooth...\n";
    std::cout << "Press Ctrl+C to stop\n";

    while (g_run) {
        pause();
    }

    std::cout << "\nStopping sender...\n";

    imuPublisher.stop();

    if (csv_fd >= 0) {
        ::close(csv_fd);
    }
    ::close(bt_fd);

    std::cout << "Sender stopped\n";
    return 0;
}