#ifndef IMUPROCESSOR_H
#define IMUPROCESSOR_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <cmath>
#include "IMUCallback.h"

class IMUProcessor : public IMUCallback {
public:
    enum State { SLEEPING, RECORDING };

    void hasSample(IMUSample& sample) override {
        float motionMag = std::sqrt(sample.x*sample.x + sample.y*sample.y + sample.z*sample.z);

        if (state == SLEEPING) {
            // TRIGGER: Start if IMU reaches a specific coordinate position (e.g., tilted X > 5)
            if (sample.x > 5.0f) { 
                state = RECORDING;
                recordFile.open("imu_data.csv", std::ios::app);
                std::cout << "Trigger position reached! Recording started..." << std::endl;
            }
        } else {
            // RECORDING MODE
            recordFile << sample.x << "," << sample.y << "," << sample.z << "\n";
            updateVisuals(sample); // Hook for Qwt-plot [18]

            // REST DETECTION: If motion is below threshold (e.g., 0.1)
            if (std::abs(motionMag - lastMag) < 0.1f) {
                if (!isResting) {
                    restStartTime = std::chrono::steady_clock::now();
                    isResting = true;
                }
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - restStartTime).count();
                
                if (duration >= 30) {
                    std::cout << "30s rest detected. Returning to sleep mode." << std::endl;
                    state = SLEEPING;
                    recordFile.close();
                }
            } else {
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

    void updateVisuals(IMUSample& s) {
        // Here you would pipe data to the qwt-plot application [18]
        // Example: std::cout << "PLOT:" << s.x << "," << s.y << "," << s.z << std::endl;
    }
};

#endif