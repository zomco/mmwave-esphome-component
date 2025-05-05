import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    ICON_MOTION_SENSOR,
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1Component, CONF_R60ABD1_ID

# Define keys for YAML configuration
CONF_MOTION_INFO = "motion_info"
CONF_RESPIRATION_INFO = "respiration_info"
CONF_SLEEP_STAGE = "sleep_stage"
CONF_SLEEP_RATING = "sleep_rating"
CONF_FIRMWARE_VERSION = "firmware_version"

ICON_LUNGS = "mdi:lungs"
ICON_SLEEP = "mdi:sleep"
ICON_INFORMATION_OUTLINE = "mdi:information-outline"


# Configuration schema for individual text sensors
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(R60ABD1Component),  # Reference the hub
        cv.Optional(CONF_MOTION_INFO): text_sensor.text_sensor_schema(
            icon=ICON_MOTION_SENSOR  # Or mdi:walk
        ),
        cv.Optional(CONF_RESPIRATION_INFO): text_sensor.text_sensor_schema(
            icon=ICON_INFORMATION_OUTLINE  # Or ICON_LUNGS
        ),
        cv.Optional(CONF_SLEEP_STAGE): text_sensor.text_sensor_schema(icon=ICON_SLEEP),
        cv.Optional(CONF_SLEEP_RATING): text_sensor.text_sensor_schema(icon=ICON_SLEEP),
        cv.Optional(CONF_FIRMWARE_VERSION): text_sensor.text_sensor_schema(icon=ICON_SLEEP),
    }
)


# Function to generate C++ code
async def to_code(config):
    r60abd1_component = await cg.get_variable(config[CONF_R60ABD1_ID])  # Get the hub instance

    if motion_info := config.get(CONF_MOTION_INFO):
        sens = await text_sensor.new_text_sensor(motion_info)
        cg.add(r60abd1_component.set_motion_info_text_sensor(sens))

    if respiration_info := config.get(CONF_RESPIRATION_INFO):
        sens = await text_sensor.new_text_sensor(respiration_info)
        cg.add(r60abd1_component.set_respiration_info_text_sensor(sens))

    if sleep_stage := config.get(CONF_SLEEP_STAGE):
        sens = await text_sensor.new_text_sensor(sleep_stage)
        cg.add(r60abd1_component.set_sleep_stage_text_sensor(sens))

    if sleep_rating := config.get(CONF_SLEEP_RATING):
        sens = await text_sensor.new_text_sensor(sleep_rating)
        cg.add(r60abd1_component.set_sleep_rating_text_sensor(sens))

    if firmware_version := config.get(CONF_FIRMWARE_VERSION):
        sens = await text_sensor.new_text_sensor(firmware_version)
        cg.add(r60abd1_component.set_firmware_version_text_sensor(sens))
