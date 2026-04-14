#include "led_controller.h"

#include <stdexcept>
#include <string>

LEDController::LEDController(LEDControllerSettings settings)
    : greenGpio_(settings.greenGpio)
{
    const std::string chipPath = "/dev/gpiochip" + std::to_string(settings.chipNumber);

    chip_ = std::make_shared<gpiod::chip>(chipPath);

    gpiod::line_config line_cfg;

    line_cfg.add_line_settings(
        greenGpio_,
        gpiod::line_settings()
            .set_direction(gpiod::line::direction::OUTPUT)
            .set_output_value(gpiod::line::value::INACTIVE));

    auto builder = chip_->prepare_request();
    builder.set_consumer("led_controller");
    builder.set_line_config(line_cfg);

    request_ = std::make_shared<gpiod::line_request>(builder.do_request());
}

LEDController::~LEDController()
{
    try
    {
        allOff();
        if (request_)
            request_->release();
        if (chip_)
            chip_->close();
    }
    catch (...)
    {
    }
}

void LEDController::greenOn()
{
    request_->set_value(greenGpio_, gpiod::line::value::ACTIVE);
    greenActive_ = true;
}

void LEDController::greenOff()
{
    request_->set_value(greenGpio_, gpiod::line::value::INACTIVE);
    greenActive_ = false;
}

void LEDController::allOff()
{
    greenOff();
}

void LEDController::flashGreen(int flashMs)
{
    greenOn();
    greenOffTime_ = std::chrono::steady_clock::now() + std::chrono::milliseconds(flashMs);
}

void LEDController::service()
{
    updateOutputs();
}

void LEDController::updateOutputs()
{
    const auto now = std::chrono::steady_clock::now();

    if (greenActive_ && now >= greenOffTime_)
    {
        greenOff();
    }
}