"""Keep configured router credentials available while the radio is disabled."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_PASSWORD, CONF_SSID

DEPENDENCIES = ["wifi"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SSID): cv.All(cv.string_strict, cv.Length(min=1, max=32)),
        cv.Required(CONF_PASSWORD): cv.string_strict,
    }
)


async def to_code(config):
    control = cg.global_ns.namespace("dpf_wifi_control")
    cg.add(control.initialize_station_config(config[CONF_SSID], config[CONF_PASSWORD]))
