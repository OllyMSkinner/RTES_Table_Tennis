#include <atomic>

namespace PositionDetectorName {

class PositionDetector {
public:
    // ... your existing stuff ...

    bool RunFromIMU(unsigned bus,
                    unsigned addr,
                    std::atomic_bool& runFlag,
                    int samplePeriodMs = 50);

    // ...
};

}
