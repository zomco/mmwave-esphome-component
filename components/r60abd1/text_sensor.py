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
from . import micradar_r60abd1_ns, MicRadarR60ABD1

# Define keys for YAML configuration
CONF_MOTION_TEXT = "motion_text"
CONF_RESPIRATION_INFO = "respiration_info"
CONF_SLEEP_STAGE = "sleep_stage"

# Define text sensor types
TYPES = [
    CONF_MOTION_TEXT,
    CONF_RESPIRATION_INFO,
    CONF_SLEEP_STAGE,
]

# Configuration schema for individual text sensors
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(MicRadarR60ABD1), # Reference the hub
        cv.Optional(CONF_MOTION_TEXT): text_sensor.text_sensor_schema(
            icon=ICON_MOTION_SENSOR # Or mdi:walk
        ),
        cv.Optional(CONF_RESPIRATION_INFO): text_sensor.text_sensor_schema(
             icon=ICON_INFORMATION_OUTLINE # Or ICON_LUNGS
        ),
        cv.Optional(CONF_SLEEP_STAGE): text_sensor.text_sensor_schema(
             icon=ICON_SLEEP
        ),
    }
)

# Function to generate C++ code
async def to_code(config):
    hub = await cg.get_variable(config[CONF_ID]) # Get the hub instance

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
