#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/defines.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"

#include "driver/adc.h"
#include <esp_adc_cal.h>

namespace esphome {
namespace adc {

class ADC2Sensor : public sensor::Sensor, public PollingComponent, public voltage_sampler::VoltageSampler {
 public:
  /// Set the attenuation for this pin. Only available on the ESP32.
  void set_attenuation(adc_atten_t attenuation) { attenuation_ = attenuation; }
  void set_channel(adc2_channel_t channel) { channel_ = channel; }
  void set_oversampling(int oversampling) { oversampling_ = oversampling; }

  /// Update adc values.
  void update() override;
  /// Setup ADc
  void setup() override;
  void dump_config() override;
  /// `HARDWARE_LATE` setup priority.
  float get_setup_priority() const override;
  void set_pin(InternalGPIOPin *pin) { this->pin_ = pin; }
  void set_output_raw(bool output_raw) { output_raw_ = output_raw; }
  float sample() override;

 protected:
  InternalGPIOPin *pin_;
  bool output_raw_{false};
  int oversampling_{1};

  adc_atten_t attenuation_{ADC_ATTEN_DB_0};
  adc2_channel_t channel_{};
  esp_adc_cal_characteristics_t cal_characteristics_[(int) ADC_ATTEN_MAX] = {};
};

}  // namespace adc
}  // namespace esphome