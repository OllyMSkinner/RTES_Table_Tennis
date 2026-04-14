#ifndef EVENT_DETECTOR_H
#define EVENT_DETECTOR_H

class LEDController;

struct PiezoEventDetectorSettings
{
    float dipThreshold = 0.5f;
    float peakThreshold = 2.5f;
    int flashMs = 100;
    bool enableDebugPrints = false;
};

class PiezoEventDetector
{
public:
    PiezoEventDetector(LEDController& ledRef,
                       PiezoEventDetectorSettings settings = PiezoEventDetectorSettings());

    void processSample(float v);

private:
    LEDController& led_;
    PiezoEventDetectorSettings settings_;

    bool dipTriggered_ = false;
    bool peakTriggered_ = false;
};

#endif