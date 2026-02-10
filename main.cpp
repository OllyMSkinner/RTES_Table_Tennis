#include "IMUPublisher.h"
#include "IMUProcessor.h"

int main() {
    IMUPublisher driver(17, 0x68); // GPIO 17, I2C Addr 0x68
    IMUProcessor processor;
    driver.registerCallback(&processor);
    driver.start();
    std::cout << "System active. Waiting for trigger position..." << std::endl;
    std::cin.get();
    driver.stop();
    return 0;
}