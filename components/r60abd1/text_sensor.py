import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_ID,
    ICON_MOTION_SENSOR,
    ICON_LUNGS,
    ICON_SLEEP,
    ICON_INFORMATION_OUTLINE,
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1Component

# Define keys for YAML configuration
CONF_MOTION_TEXT = "motion_text"
CONF_RESPIRATION_INFO = "respiration_info"
CONF_SLEEP_STAGE = "sleep_stage"
CONF_SLEEP_RATING = "sleep_rating"
CONF_FIRMWEAR_VERSION = "firmwear_version"

# Define text sensor types
TYPES = [
    CONF_MOTION_TEXT,
    CONF_RESPIRATION_INFO,
    CONF_SLEEP_STAGE,
]

# Configuration schema for individual text sensors
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(R60ABD1Component),  # Reference the hub
        cv.Optional(CONF_MOTION_TEXT): text_sensor.text_sensor_schema(
            icon=ICON_MOTION_SENSOR  # Or mdi:walk
        ),
        cv.Optional(CONF_RESPIRATION_INFO): text_sensor.text_sensor_schema(
            icon=ICON_INFORMATION_OUTLINE  # Or ICON_LUNGS
        ),
        cv.Optional(CONF_SLEEP_STAGE): text_sensor.text_sensor_schema(icon=ICON_SLEEP),
        cv.Optional(CONF_SLEEP_RATING): text_sensor.text_sensor_schema(icon=ICON_SLEEP),
        cv.Optional(CONF_FIRMWEAR_VERSION): text_sensor.text_sensor_schema(icon=ICON_SLEEP),
    }
)


# Function to generate C++ code
async def to_code(config):
    hub = await cg.get_variable(config[CONF_ID])  # Get the hub instance

    for key in TYPES:
        if key in config:
            conf = config[key]
            sens = await text_sensor.new_text_sensor(conf)
            if key == CONF_MOTION_TEXT:
                cg.add(hub.set_motion_text_sensor(sens))
            elif key == CONF_RESPIRATION_INFO:
                cg.add(hub.set_respiration_info_sensor(sens))
            elif key == CONF_SLEEP_STAGE:
                cg.add(hub.set_sleep_stage_sensor(sens))
            elif key == CONF_SLEEP_RATING:
                cg.add(hub.set_sleep_rating_sensor(sens))
            elif key == CONF_FIRMWEAR_VERSION:
                cg.add(hub.set_firmwear_version_sensor(sens))
