import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_MIN_VALUE,
    CONF_MAX_VALUE,
    CONF_STEP,
    CONF_MODE,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ICON,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
    UNIT_MINUTE,
)

# Import the namespace and hub class from __init__.py
from .. import r60abd1_ns, R60ABD1Component, CONF_R60ABD1_ID

# Define keys for YAML configuration
CONF_RESPIRATION_LOW_THRESHOLD = "respiration_low_threshold"
CONF_UNATTENDED_TIME = "unattended_time"
CONF_SLEEP_END_TIME = "sleep_end_time"


# Configuration schema for the numbers
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(R60ABD1Component),  # Reference the hub ID
        # Define schema for each number type
        cv.Optional(CONF_RESPIRATION_LOW_THRESHOLD): number.number_schema(
            r60abd1_ns.class_("RespirationLowThresholdNumber", number.Number),
            icon="mdi:transfer-down",  # Icon indicating a lower threshold
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement="rpm",
        ),
        cv.Optional(CONF_UNATTENDED_TIME): number.number_schema(
            r60abd1_ns.class_("UnattendedTimeNumber", number.Number),
            icon="mdi:clock-alert-outline",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_MINUTE,
        ),
        cv.Optional(CONF_SLEEP_END_TIME): number.number_schema(
            r60abd1_ns.class_("SleepEndTimeNumber", number.Number),
            icon="mdi:sleep-off",  # Icon indicating end of sleep period
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_MINUTE,
        ),
    }
)


# Function to generate C++ code for number setup
async def to_code(config):
    r60abd1_component = await cg.get_variable(config[CONF_R60ABD1_ID])  # Get the hub instance

    if respiration_low_threshold := config.get(CONF_RESPIRATION_LOW_THRESHOLD):
        n = await number.new_number(
            respiration_low_threshold, min_value=10, max_value=20, step=1
        )
        await cg.register_parented(n, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_respiration_low_threshold(n))

    if unattended_time := config.get(CONF_UNATTENDED_TIME):
        n = await number.new_number(
            unattended_time, min_value=30, max_value=180, step=10
        )
        await cg.register_parented(n, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_unattended_time(n))

    if sleep_end_time := config.get(CONF_SLEEP_END_TIME):
        n = await number.new_number(
            sleep_end_time, min_value=5, max_value=120, step=1
        )
        await cg.register_parented(n, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_sleep_end_time(n))