#include "IMUPublisher.h"
#include <fcntl.h>      // For open()
#include <sys/ioctl.h>  // For ioctl()
#include <linux/i2c-dev.h> // For I2C_SLAVE
#include <unistd.h>     // For close()
#include <stdexcept>    // For runtime_error
#include <chrono>       // For milliseconds
#include <gpiod.hpp>    // Required: sudo apt-get install libgpiod-dev

// The constructor initializes the hardware communication [4]
IMUPublisher::IMUPublisher(int pinNum, int i2cAddr) 
    : pin(pinNum), addr(i2cAddr), running(false) {
    
    // 1. Open the primary I2C bus on the Raspberry Pi [4]
    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        throw std::runtime_error("Failed to open the I2C bus.");
    }

    // 2. Set the I2C Slave Address for the ICM-20948 [5]
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        close(fd);
        throw std::runtime_error("Failed to connect to the IMU.");
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
    // 1. Configure the GPIO chip and line using modern libgpiod (v2 style) [6, 7]
    auto chip = std::make_shared<gpiod::chip>("/dev/gpiochip0");
    auto line_cfg = gpiod::line_config();
    line_cfg.add_line_settings(pin, gpiod::line_settings()
        .set_direction(gpiod::line::direction::INPUT)
        .set_edge_detection(gpiod::line::edge::RISING));

    auto request_builder = chip->prepare_request();
    request_builder.set_line_config(line_cfg);
    auto request = request_builder.do_request();

    while (running) {
        // 2. REMOVING POLLING: Block the thread until a GPIO edge occurs [1]
        if (request.wait_edge_events(std::chrono::milliseconds(100))) {
            // Clear the edge event buffer [1]
            gpiod::edge_event_buffer event_buf;
            request.read_edge_events(event_buf, 1);

            // 3. Hardware specific read sequence [5, 8]
            uint8_t reg = 0x2D; // ICM-20948 Accelerometer X High register
            if (write(fd, &reg, 1) != 1) continue;

            // Fix: We need 6 bytes for X, Y, Z (2 bytes each) [9]
            uint8_t buffer[10]; 
            if (read(fd, buffer, 6) != 6) continue;

            // 4. Data Reconstruction [9]
            // High byte is transmitted first. Correct indices: X(0,1), Y(2,3), Z(4,5)
            IMUSample s;
            s.x = (int16_t)((buffer << 8) | buffer[11]) * 0.0006f; 
            s.y = (int16_t)((buffer[12] << 8) | buffer[13]) * 0.0006f;
            s.z = (int16_t)((buffer[14] << 8) | buffer[15]) * 0.0006f;

            // 5. Trigger the callback by reference for efficiency [9, 16]
            if (callback) {
                callback->hasSample(s);
            }
        }
    }
}
