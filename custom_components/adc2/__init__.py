import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_INPUT

from esphome.core import CORE
from esphome.components.esp32 import get_esp32_variant
from esphome.components.esp32.const import (
    VARIANT_ESP32,
    VARIANT_ESP32C3,
    VARIANT_ESP32H2,
    VARIANT_ESP32S2,
    VARIANT_ESP32S3,
)

CODEOWNERS = ["@esphome/core"]

ATTENUATION_MODES = {
    "0db": cg.global_ns.ADC_ATTEN_DB_0,
    "2.5db": cg.global_ns.ADC_ATTEN_DB_2_5,
    "6db": cg.global_ns.ADC_ATTEN_DB_6,
    "11db": cg.global_ns.ADC_ATTEN_DB_11,
    "auto": "auto",
}

adc2_channel_t = cg.global_ns.enum("adc2_channel_t")

# From https://github.com/espressif/esp-idf/blob/master/components/driver/include/driver/adc_common.h
# pin to adc1 channel mapping
ESP32_VARIANT_ADC2_PIN_TO_CHANNEL = {
    VARIANT_ESP32S3: {
        11: adc2_channel_t.ADC2_CHANNEL_0,
        12: adc2_channel_t.ADC2_CHANNEL_1,
        13: adc2_channel_t.ADC2_CHANNEL_2,
        14: adc2_channel_t.ADC2_CHANNEL_3,
        15: adc2_channel_t.ADC2_CHANNEL_4,
        16: adc2_channel_t.ADC2_CHANNEL_5,
        17: adc2_channel_t.ADC2_CHANNEL_6,
        18: adc2_channel_t.ADC2_CHANNEL_7,
        19: adc2_channel_t.ADC2_CHANNEL_8,
        20: adc2_channel_t.ADC2_CHANNEL_9,
    },
}


def validate_adc_pin(value):
    if CORE.is_esp32:
        value = pins.internal_gpio_input_pin_number(value)
        variant = get_esp32_variant()
        if variant not in ESP32_VARIANT_ADC2_PIN_TO_CHANNEL:
            raise cv.Invalid(f"This ESP32 variant ({variant}) is not supported")

        if value not in ESP32_VARIANT_ADC2_PIN_TO_CHANNEL[variant]:
            raise cv.Invalid(f"{variant} doesn't support ADC on this pin")
        return pins.internal_gpio_input_pin_schema(value)

    raise NotImplementedError