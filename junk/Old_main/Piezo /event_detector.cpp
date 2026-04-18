#include "event_detector.h"
#include "led_controller.h"

#include <iostream>

PiezoEventDetector::PiezoEventDetector(LEDController& ledRef,
                                       PiezoEventDetectorSettings settings)
    : led_(ledRef),
      settings_(settings)
{
    if (settings_.enableDebugPrints)
    {
        std::cout << "Detector initialised with dip=" << settings_.dipThreshold
                  << " peak=" << settings_.peakThreshold << std::endl;
    }
}

void PiezoEventDetector::processSample(float v)
{
    led_.service();

    if (!dipTriggered_ && v <= settings_.dipThreshold)
    {
        if (settings_.enableDebugPrints)
        {
            std::cout << "GREEN" << std::endl;
        }

        led_.flashGreen(settings_.flashMs);
        dipTriggered_ = true;
        peakTriggered_ = false;
    }
    else if (dipTriggered_ && !peakTriggered_ && v >= settings_.peakThreshold)
    {
        if (settings_.enableDebugPrints)
        {
            std::cout << "RED" << std::endl;
        }

        led_.flashRed(settings_.flashMs);
        peakTriggered_ = true;
    }
    else if (v > settings_.dipThreshold && v < settings_.peakThreshold)
    {
        dipTriggered_ = false;
        peakTriggered_ = false;
    }
}
