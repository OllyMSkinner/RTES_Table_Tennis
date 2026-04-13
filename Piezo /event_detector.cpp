#include "event_detector.h"
#include "led_controller.h"
#include <iostream>

PiezoEventDetector::PiezoEventDetector(LEDController& ledRef,
                                       float dipThreshold,
                                       float peakThreshold,
                                       int flashMs)
    : led_(ledRef),
      dipThreshold_(dipThreshold),
      peakThreshold_(peakThreshold),
      flashMs_(flashMs),
      dipTriggered_(false),
      peakTriggered_(false)
{
    std::cout << "Detector initialised with dip=" << dipThreshold_
              << " peak=" << peakThreshold_ << std::endl;
}

void PiezoEventDetector::processSample(float v)
{
    if (!dipTriggered_ && v <= dipThreshold_)
    {
        std::cout << "GREEN" << std::endl;
        led_.flashGreen(flashMs_);
        dipTriggered_ = true;
        peakTriggered_ = false;
    }
    else if (dipTriggered_ && !peakTriggered_ && v >= peakThreshold_)
    {
        std::cout << "RED" << std::endl;
        led_.flashRed(flashMs_);
        peakTriggered_ = true;
    }
    else if (v > dipThreshold_ && v < peakThreshold_)
    {
        // Reset once signal comes back into the middle region
        dipTriggered_ = false;
        peakTriggered_ = false;
    }
}
