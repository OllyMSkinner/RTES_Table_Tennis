#include <iostream>      // Added to support std::cout and std::cin
#include "IMUPublisher.h"
#include "IMUProcessor.h"

int main() {
    // 1. Setup the Driver
    // Pin 17 is used for the Data Ready signal [3, 4].
    // The ICM-20948 default I2C address is 0x69 (0x68 if jumpered) [1, 2].
    IMUPublisher driver(17, 0x69); 
    
    // 2. Setup the Subscriber (The logic for recording/rest is here)
    IMUProcessor processor;

    // 3. Register the callback
    // The driver hands over data to the processor via this interface [3, 5].
    driver.registerCallback(&processor);

    // 4. Start acquisition
    // This launches the background thread for real-time tracking [6, 7].
    driver.start();

    std::cout << "System active. Moving IMU to trigger position will start recording." << std::endl;
    std::cout << "Press Enter at any time to stop and exit." << std::endl;

    // 5. Keep the program running
    // The main thread blocks here while the worker thread handles events [8-10].
    std::cin.get();

    // 6. Graceful shutdown
    driver.stop();
    
    return 0;
}
