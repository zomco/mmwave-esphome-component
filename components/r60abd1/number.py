import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ID,
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
from . import micradar_r60abd1_ns, MicRadarR60ABD1

# Define keys for YAML configuration
CONF_RESPIRATION_LOW_THRESHOLD = "respiration_low_threshold"
CONF_UNATTENDED_TIME = "unattended_time"
CONF_SLEEP_END_TIME = "sleep_end_time"

# List of number types
TYPES = [
    CONF_RESPIRATION_LOW_THRESHOLD,
    CONF_UNATTENDED_TIME,
    CONF_SLEEP_END_TIME,
]

# Configuration schema for the numbers
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(MicRadarR60ABD1),  # Reference the hub ID
        # Define schema for each number type
        cv.Optional(CONF_RESPIRATION_LOW_THRESHOLD): number.number_schema(
            icon="mdi:transfer-down",  # Icon indicating a lower threshold
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement="rpm",
            mode="box",  # Use box input mode
            min_value=10,  # Range 10-20 according to protocol
            max_value=20,
            step=1,
        ),
        cv.Optional(CONF_UNATTENDED_TIME): number.number_schema(
            icon="mdi:clock-alert-outline",
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_MINUTE,
            mode="box",
            min_value=30,  # Range 30-180 according to protocol
            max_value=180,
            step=10,  # Protocol implies steps of 10, enforce here
        ),
        cv.Optional(CONF_SLEEP_END_TIME): number.number_schema(
            icon="mdi:sleep-off",  # Icon indicating end of sleep period
            entity_category=ENTITY_CATEGORY_CONFIG,
            unit_of_measurement=UNIT_MINUTE,
            mode="box",
            min_value=5,  # Range 5-120 according to protocol
            max_value=120,
            step=1,  # Protocol doesn't specify step, use 1
        ),
    }
)


# Function to generate C++ code for number setup
async def to_code(config):
    hub = await cg.get_variable(config[CONF_ID])  # Get the hub instance

    for key in TYPES:
        if key in config:
            conf = config[key]
            # Create a new Number object
            var = await number.new_number(
                conf,
                min_value=conf[CONF_MIN_VALUE],
                max_value=conf[CONF_MAX_VALUE],
                step=conf[CONF_STEP],
            )

            # Generate C++ lambda code to call the hub's setter method
            # 'x' is the new value passed from Home Assistant
            if key == CONF_RESPIRATION_LOW_THRESHOLD:
                cg.add(
                    var.set_set_action(
                        cg.RawLambda(f"{hub}->set_respiration_low_threshold(x);")
                    )
                )
            elif key == CONF_UNATTENDED_TIME:
                cg.add(
                    var.set_set_action(cg.RawLambda(f"{hub}->set_unattended_time(x);"))
                )
            elif key == CONF_SLEEP_END_TIME:
                cg.add(
                    var.set_set_action(cg.RawLambda(f"{hub}->set_sleep_end_time(x);"))
                )
