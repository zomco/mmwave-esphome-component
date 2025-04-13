import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_OCCUPANCY,
    CONF_HAS_TARGET,
)
from . import CONF_R60ABD1_ID, R60ABD1Component

DEPENDENCIES = ["R60ABD1"]

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(R60ABD1Component),
    cv.Optional(CONF_HAS_TARGET): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_OCCUPANCY, icon="mdi:motion-sensor"
    ),
}


async def to_code(config):
    R60ABD1_component = await cg.get_variable(config[CONF_R60ABD1_ID])

    if has_target_config := config.get(CONF_HAS_TARGET):
        sens = await binary_sensor.new_binary_sensor(has_target_config)
        cg.add(R60ABD1_component.set_has_target_binary_sensor(sens))