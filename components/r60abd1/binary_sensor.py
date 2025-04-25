import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    CONF_ID,
    CONF_PRESENCE,
    DEVICE_CLASS_PRESENCE,
    DEVICE_CLASS_OCCUPANCY,  # Good for bed status
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1Component

# Define keys for YAML configuration
CONF_BED_STATUS = "bed_status"

# Define binary sensor types
TYPES = [
    CONF_PRESENCE,
    CONF_BED_STATUS,
]

# Configuration schema for individual binary sensors
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(R60ABD1Component),  # Reference the hub
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
    hub = await cg.get_variable(config[CONF_ID])  # Get the hub instance

    for key in TYPES:
        if key in config:
            conf = config[key]
            sens = await binary_sensor.new_binary_sensor(conf)
            if key == CONF_PRESENCE:
                cg.add(hub.set_presence_sensor(sens))
            elif key == CONF_BED_STATUS:
                cg.add(hub.set_bed_status_sensor(sens))
