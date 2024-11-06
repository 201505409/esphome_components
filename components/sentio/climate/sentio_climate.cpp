#include "sentio_climate.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace sentio {
static const char *TAG = "sentio.climate";

void SentioClimate::setup() {
  current_temp_sensor_->add_on_state_callback([this](float state) {
    // Set current temperature with 0.1 precision
    current_temperature = roundf(state * 10) / 10.0;
    publish_state();
  });
  temp_setpoint_number_->add_on_state_callback([this](float state) {
    // Set target temperature to match 0.5 precision for setpoint
    target_temperature = roundf(state * 2) / 2.0;
    publish_state();
  });

  mode_select_->add_on_state_callback([this](float state) {
    sentio_mode_to_climatemode(state);
    publish_state();
  });

  // Initialize with current states
  current_temperature = roundf(current_temp_sensor_->state * 10) / 10.0;
  target_temperature  = roundf(temp_setpoint_number_->state * 2) / 2.0;
  sentio_mode_to_climatemode(mode_select_->state); 
}

void SentioClimate::control(const climate::ClimateCall& call) {
  if (call.get_target_temperature().has_value())
  {
    this->target_temperature = roundf(*call.get_target_temperature() * 2) / 2.0;
    float target = target_temperature;
    ESP_LOGD(TAG, "Target temperature changed to: %f", target);
    temp_setpoint_number_->make_call().set_value(target).perform();
  }
}

climate::ClimateTraits SentioClimate::traits() {
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(true);
  traits.set_visual_temperature_step(0.5);  // 0.5 step for setpoint adjustment
  traits.set_visual_min_temperature(5);
  traits.set_visual_max_temperature(30);

  return traits;
}

void SentioClimate::dump_config() {
  LOG_CLIMATE("", "Sentio Climate", this);
}

void SentioClimate::sentio_mode_to_climatemode(const int state)
{
  switch (state) {
  case 1:
    this->mode = climate::CLIMATE_MODE_OFF;
    break;
  case 2:
    this->mode = climate::CLIMATE_MODE_HEAT;
    break;
  case 3:
    this->mode = climate::CLIMATE_MODE_COOL;
    break;
  default: 
    this->mode = climate::CLIMATE_MODE_OFF;
    break;
  }
}

} // namespace sentio
} // namespace esphome
