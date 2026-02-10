#ifndef IMUPUBLISHER_H
#define IMUPUBLISHER_H

#include <thread>
#include <atomic>
#include <gpiod.hpp>
#include "IMUCallback.h"

class IMUPublisher {
public:
    IMUPublisher(int pinNum, int i2cAddr);
    ~IMUPublisher();
    void registerCallback(IMUCallback* cb);
    void start();
    void stop();

private:
    void worker();
    int pin, addr, fd;
    std::atomic<bool> running;
    std::thread workerThread;
    IMUCallback* callback = nullptr;
};

#endif