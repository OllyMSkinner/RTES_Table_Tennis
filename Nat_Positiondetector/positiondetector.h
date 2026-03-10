#ifndef POSITIONDETECTOR_H
#define POSITIONDETECTOR_H

#include <atomic>
#include <functional>

namespace PositionDetectorName {

class UprightCallback {
public:
    virtual void OnUprightDetected() = 0;
};

class PositionDetector {
public:
    PositionDetector() : Counter(0), UprightConfirmed(false), callback(nullptr) {}

    // Check position using given accelerometer readings
    void CheckPosition(float X, float Y, float Z);

    // Stream IMU and detect upright internally
    bool RunFromIMU(unsigned bus,
                    unsigned addr,
                    ::std::atomic_bool& runFlag,
                    int samplePeriodMs = 50);

    // Callback to notify when upright detected
    UprightCallback* callback;

private:
    int Counter;
    bool UprightConfirmed;
};

} // namespace PositionDetectorName

#endif // POSITIONDETECTOR_H
