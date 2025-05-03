import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    DEVICE_CLASS_PRESENCE,
    DEVICE_CLASS_OCCUPANCY,  # Good for bed status
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1Component, CONF_R60ABD1_ID

# Define keys for YAML configuration
CONF_PRESENCE = "presence"
CONF_BED_STATUS = "bed_status"

# Configuration schema for individual binary sensors
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(R60ABD1Component),  # Reference the hub
        cv.Optional(CONF_PRESENCE): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_PRESENCE
        ),
        cv.Optional(CONF_BED_STATUS): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_OCCUPANCY  # Or maybe 'sleeping' if preferred
        ),
    }
)


# Function to generate C++ code
async def to_code(config):
    r60abd1_component = await cg.get_variable(config[CONF_R60ABD1_ID])  # Get the hub instance

    if presence := config.get(CONF_PRESENCE):
        sens = await binary_sensor.new_binary_sensor(presence)
        cg.add(r60abd1_component.set_presence_binary_sensor(sens))

    if bed_status := config.get(CONF_BED_STATUS):
        sens = await binary_sensor.new_binary_sensor(bed_status)
        cg.add(r60abd1_component.set_bed_status_binary_sensor(sens))
