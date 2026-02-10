#include "IMUPublisher.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

IMUPublisher::IMUPublisher(int pinNum, int i2cAddr) : pin(pinNum), addr(i2cAddr), running(false) {
    fd = open("/dev/i2c-1", O_RDWR);
    ioctl(fd, I2C_SLAVE, addr);
}

IMUPublisher::~IMUPublisher() { stop(); close(fd); }

void IMUPublisher::registerCallback(IMUCallback* cb) { callback = cb; }

void IMUPublisher::start() {
    running = true;
    workerThread = std::thread(&IMUPublisher::worker, this);
}

void IMUPublisher::stop() {
    running = false;
    if (workerThread.joinable()) workerThread.join();
}

void IMUPublisher::worker() {
    auto chip = std::make_shared<gpiod::chip>("/dev/gpiochip0");
    auto line = chip->get_line(pin);
    line.request({"IMU_DRDY", gpiod::line_request::DIRECTION_INPUT, gpiod::line_request::EVENT_RISING_EDGE});

    while (running) {
        if (line.event_wait(std::chrono::milliseconds(100))) {
            line.event_read();
            uint8_t reg = 0x3B; 
            write(fd, &reg, 1);
            uint8_t buffer[12];
            read(fd, buffer, 6);

            IMUSample s;
            s.x = (int16_t)((buffer << 8) | buffer[13]) * 0.0006f;
            s.y = (int16_t)((buffer[14] << 8) | buffer[15]) * 0.0006f;
            s.z = (int16_t)((buffer[16] << 8) | buffer[17]) * 0.0006f;

            if (callback) callback->hasSample(s);
        }
    }
}