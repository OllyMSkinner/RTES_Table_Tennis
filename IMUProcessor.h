#ifndef IMUPROCESSOR_H
#define IMUPROCESSOR_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include "IMUCallback.h"
#include "IMUSample.h"

// Subscriber class that implements the application logic
class IMUProcessor : public IMUCallback {
public:
    enum State { SLEEPING, RECORDING };

    // This method is called by the background thread as soon as data arrives [1, 2]
    void hasSample(IMUSample& sample) override {
        // Calculate motion magnitude to detect if the sensor is "at rest" [3]
        float motionMag = std::sqrt(sample.x*sample.x + sample.y*sample.y + sample.z*sample.z);

        if (state == SLEEPING) {
            // TRIGGER: Start recording if the IMU reaches a specific position (e.g., X > 5.0)
            if (sample.x > 5.0f) { 
                state = RECORDING;
                recordFile.open("imu_data.csv", std::ios::app);
                std::cout << "Target position detected! Recording started..." << std::endl;
            }
        } else {
            // RECORDING MODE: Log coordinates and update real-time visuals
            recordFile << sample.x << "," << sample.y << "," << sample.z << "\n";
            updateVisuals(sample); // Hook for plotting software [4]

            // REST DETECTION: Check if movement magnitude has stabilised [3, 5]
            if (std::abs(motionMag - lastMag) < 0.1f) {
                if (!isResting) {
                    restStartTime = std::chrono::steady_clock::now();
                    isResting = true;
                }

                // Check if the sensor has been at rest for 30 consecutive seconds
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - restStartTime).count();
                
                if (duration >= 30) {
                    std::cout << "30s of rest detected. Stopping recording and sleeping." << std::endl;
                    state = SLEEPING;
                    recordFile.close();
                }
            } else {
                // Motion detected; reset the rest timer
                isResting = false;
            }
        }
        lastMag = motionMag;
    }

private:
    State state = SLEEPING;
    std::ofstream recordFile;
    bool isResting = false;
    float lastMag = 0;
    std::chrono::steady_clock::time_point restStartTime;

    // Interface for real-time visual plotting applications
    void updateVisuals(IMUSample& s) {
        // In a full implementation, this would pipe data to the qwt-plot tool [4]
        // Example: std::cout << "PLOT:" << s.x << "," << s.y << "," << s.z << std::endl;
    }
};

#endif
