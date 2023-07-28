#include "adc2_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace adc {

static const char *const TAG = "adc2";

// 13bit for S2, and 12bit for all other esp32 variants
static const adc_bits_width_t ADC_WIDTH_MAX_SOC_BITS = static_cast<adc_bits_width_t>(ADC_WIDTH_MAX - 1);

#ifndef SOC_ADC_RTC_MAX_BITWIDTH
#if USE_ESP32_VARIANT_ESP32S2
static const int SOC_ADC_RTC_MAX_BITWIDTH = 13;
#else
static const int SOC_ADC_RTC_MAX_BITWIDTH = 12;
#endif
#endif

static const int ADC_MAX = (1 << SOC_ADC_RTC_MAX_BITWIDTH) - 1;    // 4095 (12 bit) or 8191 (13 bit)
static const int ADC_HALF = (1 << SOC_ADC_RTC_MAX_BITWIDTH) >> 1;  // 2048 (12 bit) or 4096 (13 bit)

void ADC2Sensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ADC '%s'...", this->get_name().c_str());
  pin_->setup();

  adc2_config_channel_atten(channel_, attenuation_);

  // load characteristics for each attenuation
  for (int i = 0; i < (int) ADC_ATTEN_MAX; i++) {
    auto cal_value = esp_adc_cal_characterize(ADC_UNIT_2, (adc_atten_t) i, ADC_WIDTH_MAX_SOC_BITS,
                                              1100,  // default vref
                                              &cal_characteristics_[i]);
    switch (cal_value) {
      case ESP_ADC_CAL_VAL_EFUSE_VREF:
        ESP_LOGV(TAG, "Using eFuse Vref for calibration");
        break;
      case ESP_ADC_CAL_VAL_EFUSE_TP:
        ESP_LOGV(TAG, "Using two-point eFuse Vref for calibration");
        break;
      case ESP_ADC_CAL_VAL_DEFAULT_VREF:        
      default:
        break;
    }
  }

  ESP_LOGCONFIG(TAG, "ADC '%s' setup finished!", this->get_name().c_str());
}

void ADC2Sensor::dump_config() {
  LOG_SENSOR("", "ADC Sensor", this);
  LOG_PIN("  Pin: ", pin_);
  if (this->output_raw_) {
    ESP_LOGCONFIG(TAG, "  Output raw ADC value");
  }
  if (this->oversampling_ > 1) {
    ESP_LOGCONFIG(TAG, "  Samples: %i", this->oversampling_);
  }
  switch (this->attenuation_) {
    case ADC_ATTEN_DB_0:
    ESP_LOGCONFIG(TAG, "  Attenuation: 0db");
    break;
    case ADC_ATTEN_DB_2_5:
    ESP_LOGCONFIG(TAG, "  Attenuation: 2.5db");
    break;
    case ADC_ATTEN_DB_6:
    ESP_LOGCONFIG(TAG, "  Attenuation: 6db");
    break;
    case ADC_ATTEN_DB_11:
    ESP_LOGCONFIG(TAG, "  Attenuation: 11db");
    break;
    default:  // This is to satisfy the unused ADC_ATTEN_MAX
    break;
  }
  LOG_UPDATE_INTERVAL(this);
}

float ADC2Sensor::get_setup_priority() const { return setup_priority::DATA; }
void ADC2Sensor::update() {
  this->setup();
  ESP_LOGD(TAG, "'%s': Got update request", this->get_name().c_str());
  float value_v = this->sample();
  ESP_LOGD(TAG, "'%s': Got voltage=%.4fV", this->get_name().c_str(), value_v);
  this->publish_state(value_v);
}

float ADC2Sensor::sample() {
  int sum=0;
  int error=0;
  esp_err_t result;
  int i;
  int max = -1;
  int min = ADC_MAX + 1;
  for(i=0;i<oversampling_&&error<oversampling_;)
  {
    int raw;
    result=adc2_get_raw(channel_,ADC_WIDTH_MAX_SOC_BITS,&raw);
    if (result!=ESP_OK) {
      error++;
      ESP_LOGE(TAG, "ADC2 get raw failed: %d", result);
    }
    else {
        if (raw>max) max=raw;
        if (raw<min) min=raw;
        sum+=raw;
        i++;
        ESP_LOGD(TAG, "ADC2 reading: %d %d %d", raw, i, sum);
    }
  }
  if (error==oversampling_) {
    ESP_LOGE(TAG, "ADC2 get raw failed: %d", result);
    return NAN;
  }
  if (i>2) {
    sum-=max;
    sum-=min;
    i-=2;
  }
  int output = sum/i;
  if (output_raw_) {
      return output;
  }
  uint32_t mv = esp_adc_cal_raw_to_voltage(output, &cal_characteristics_[(int) attenuation_]);
  return mv / 1000.0f;
}

}  // namespace adc
}  // namespace esphome