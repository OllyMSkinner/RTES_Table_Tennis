#include <iostream>
#include <stdexcept>

#include "ads1115rpi.h"
#include "event_detector.h"
#include "led_controller.h"

int main() 
{
    try
    {
        constexpr float DIP_THRESHOLD  = 0.5f;
        constexpr float PEAK_THRESHOLD = 3.0f;   // 4.5f will never be reached with current scaling
        constexpr int FLASH_MS = 100;

        // Change these GPIO numbers to match your wiring
        LEDController led(0, 17, 27); // chip 0, red GPIO 17, green GPIO 27

        PiezoEventDetector detector(led, DIP_THRESHOLD, PEAK_THRESHOLD, FLASH_MS);

        ADS1115rpi ads1115rpi;

        ads1115rpi.registerCallback([&](float v)
        {
            std::cout << v << std::endl;
            detector.processSample(v);
        });

        ADS1115settings s{};
        s.i2c_bus = 1;
        s.address = 0x48;
        s.samplingRate = ADS1115settings::FS128HZ;
        s.pgaGain = ADS1115settings::FSR2_048;
        s.channel = ADS1115settings::AIN0;

        std::cout << "Starting ADS1115..." << std::endl;
        ads1115rpi.start(s);

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
