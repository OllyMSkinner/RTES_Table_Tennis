#include <iostream>
#include <stdexcept>

#include "ads1115rpi.h"
#include "event_detector.h"
#include "led_controller.h"

int main()
{
    try
    {
        LEDController led;
        PiezoEventDetector detector(led);

        ADS1115rpi ads1115rpi;

        ads1115rpi.registerCallback([&](float v)
        {
            detector.processSample(v);
        });

        std::cout << "Starting ADS1115..." << std::endl;
        ads1115rpi.start(makeDefaultADS1115Settings());

        std::cout << "Running. Press Enter to stop." << std::endl;
        std::cin.get();

        ads1115rpi.stop();
        led.allOff();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}