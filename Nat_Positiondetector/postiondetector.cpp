#include "Positiondetector.h"

#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

// Number of stable samples required for upright confirmation
int stabilitySamplesRequired = 20;

namespace PositionDetectorName {

    void PositionDetector::CheckPosition(float X, float Y, float Z){

        // Upright detection using gravity
        bool isUpright =
            (std::fabs(X) < 1.0f) &&
            (std::fabs(Y) < 1.0f) &&
            (Z > 8.5f);

        if (isUpright){

            Counter++;

            if (Counter >= stabilitySamplesRequired && !UprightConfirmed){

                UprightConfirmed = true;

                if (callback){
                    callback->OnUprightDetected();
                }
            }

        } else {

            Counter = 0;
            UprightConfirmed = false;
        }
    }

} // namespace PositionDetectorName


// ===== TESTING MAIN =====
#ifdef POSITION_DETECTOR_TEST

using namespace PositionDetectorName;

int main() {

    PositionDetector detector;

    // Simple callback that prints event
    struct MyCallback : PositionDetector::Callback {
        void OnUprightDetected() override {
            std::cout << "[EVENT] Upright position detected!" << std::endl;
        }
    } callback;

    detector.RegisterCallback(&callback);

    std::cout << "Starting test loop (simulated IMU values)..." << std::endl;

    // Simulated IMU loop
    while (true) {

        // ===== Replace with real IMU readings later =====
        float X = 0.1f;  // Simulated X
        float Y = -0.2f; // Simulated Y
        float Z = 9.1f;  // Simulated Z (gravity)

        detector.CheckPosition(X, Y, Z);

        if (detector.IsUpright()) {
            std::cout << "Upright confirmed!" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 100 Hz sample rate
    }

    return 0;
}

#endif
