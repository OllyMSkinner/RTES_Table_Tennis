#include "IMUPublisher.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <chrono>
#include <gpiod.hpp> // Required: sudo apt-get install libgpiod-dev

IMUPublisher::IMUPublisher(int pinNum, int i2cAddr) : pin(pinNum), addr(i2cAddr), running(false) {
    // Open the I2C bus #1 [6]
    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        throw std::runtime_error("Failed to open I2C bus");
    }
    // Set the I2C slave address [7]
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        throw std::runtime_error("Failed to connect to IMU via I2C");
    }
}

IMUPublisher::~IMUPublisher() { 
    stop(); 
    if (fd >= 0) close(fd); 
}

void IMUPublisher::registerCallback(IMUCallback* cb) { 
    callback = cb; 
}

void IMUPublisher::start() {
    running = true;
    workerThread = std::thread(&IMUPublisher::worker, this);
}

void IMUPublisher::stop() {
    running = false;
    if (workerThread.joinable()) workerThread.join();
}

void IMUPublisher::worker() {
    // 1. Configure the GPIO chip and line using modern libgpiod (v2 style) [5, 8]
    auto chip = std::make_shared<gpiod::chip>("/dev/gpiochip0");
    auto line_cfg = gpiod::line_config();
    line_cfg.add_line_settings(pin, gpiod::line_settings()
        .set_direction(gpiod::line::direction::INPUT)
        .set_edge_detection(gpiod::line::edge::RISING));

    auto request_builder = chip->prepare_request();
    request_builder.set_line_config(line_cfg);
    auto request = request_builder.do_request();

    while (running) {
        // 2. BLOCKING I/O: Thread sleeps until the IMU DRDY pin fires [4, 9]
        if (request.wait_edge_events(std::chrono::milliseconds(100))) {
            // Read events to clear the buffer
            gpiod::edge_event_buffer event_buf;
            request.read_edge_events(event_buf, 1);

            // 3. I2C Read Logic: Write register address, then read 6 bytes [7, 10]
            uint8_t reg = 0x3B; // Common starting register for Accel X_High
            write(fd, &reg, 1);
            
            uint8_t buffer[11]; // We need 6 bytes for X, Y, and Z (2 bytes each)
            if (read(fd, buffer, 6) != 6) continue;

            // 4. Reconstruction: Combine High and Low bytes into 16-bit integers
            // Indexing fix: buffer[12]=X, buffer[13, 14]=Y, buffer[15, 16]=Z
            IMUSample s;
            s.x = (int16_t)((buffer << 8) | buffer[12]) * 0.0006f;
            s.y = (int16_t)((buffer[13] << 8) | buffer[14]) * 0.0006f;
            s.z = (int16_t)((buffer[15] << 8) | buffer[16]) * 0.0006f;

            // 5. Trigger Callback: Pass by reference for efficiency [17, 18]
            if (callback) {
                callback->hasSample(s);
            }
        }
    }
}
