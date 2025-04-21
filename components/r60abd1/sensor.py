import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_DISTANCE,
    CONF_HEART_RATE,
    CONF_POSITION_X,
    CONF_POSITION_Y,
    CONF_POSITION_Z,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_EMPTY,  # Use empty for things like 'score' or 'count'
    DEVICE_CLASS_SPEED,  # Use for respiration rate (breaths/min)
    ICON_HEART_PULSE,
    ICON_MOTION_SENSOR,
    ICON_RULER,
    ICON_SCOREBOARD,  # For sleep score
    ICON_LUNGS,
    ICON_AXIS_X_ARROW,
    ICON_AXIS_Y_ARROW,
    ICON_AXIS_Z_ARROW,
    STATE_CLASS_MEASUREMENT,
    UNIT_CENTIMETER,
    UNIT_BEATS_PER_MINUTE,
    UNIT_EMPTY,  # For score/count
    UNIT_PERCENT,  # For body movement?
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1

# Define keys for YAML configuration specific to this component's sensors
CONF_MOTION_STATE = "motion_state"
CONF_BODY_MOVEMENT = "body_movement"
CONF_RESPIRATION_RATE = "respiration_rate"
CONF_SLEEP_SCORE = "sleep_score"

# Define sensor types provided by this component
TYPES = [
    CONF_DISTANCE,
    CONF_MOTION_STATE,
    CONF_BODY_MOVEMENT,
    CONF_HEART_RATE,
    CONF_RESPIRATION_RATE,
    CONF_SLEEP_SCORE,
    CONF_POSITION_X,
    CONF_POSITION_Y,
    CONF_POSITION_Z,
]

# Configuration schema for individual sensors
# Users will define sensors under the 'sensor:' platform in their YAML
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(
            R60ABD1
        ),  # Reference the main hub component ID
        cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CENTIMETER,
            icon=ICON_RULER,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_MOTION_STATE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,  # It's a state code (0, 1, 2)
            icon=ICON_MOTION_SENSOR,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,  # Not a standard device class
            state_class=STATE_CLASS_MEASUREMENT,  # Or maybe none if just informational?
        ),
        cv.Optional(CONF_BODY_MOVEMENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,  # Assuming 0-100 is like a percentage
            icon="mdi:walk",  # Or mdi:human-male
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HEART_RATE): sensor.sensor_schema(
            unit_of_measurement=UNIT_BEATS_PER_MINUTE,
            icon=ICON_HEART_PULSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,  # No specific device class for BPM
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RESPIRATION_RATE): sensor.sensor_schema(
            unit_of_measurement="rpm",  # breaths per minute
            icon=ICON_LUNGS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_SPEED,  # Using speed as it's a rate
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SLEEP_SCORE): sensor.sensor_schema(
            unit_of_measurement="分",  # Score unit
            icon=ICON_SCOREBOARD,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POSITION_X): sensor.sensor_schema(
            unit_of_measurement=UNIT_CENTIMETER,
            icon=ICON_AXIS_X_ARROW,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POSITION_Y): sensor.sensor_schema(
            unit_of_measurement=UNIT_CENTIMETER,
            icon=ICON_AXIS_Y_ARROW,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POSITION_Z): sensor.sensor_schema(
            unit_of_measurement=UNIT_CENTIMETER,
            icon=ICON_AXIS_Z_ARROW,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


# Function to generate C++ code for sensor setup
async def to_code(config):
    hub = await cg.get_variable(config[CONF_ID])  # Get the hub instance

    # Loop through defined sensor types and register them with the hub
    for key in TYPES:
        if key in config:
            conf = config[key]
            # Create a new sensor object
            sens = await sensor.new_sensor(conf)
            # Call the corresponding setter method on the C++ hub object
            if key == CONF_DISTANCE:
                cg.add(hub.set_distance_sensor(sens))
            elif key == CONF_MOTION_STATE:
                cg.add(hub.set_motion_sensor(sens))
            elif key == CONF_BODY_MOVEMENT:
                cg.add(hub.set_body_movement_sensor(sens))
            elif key == CONF_HEART_RATE:
                cg.add(hub.set_heart_rate_sensor(sens))
            elif key == CONF_RESPIRATION_RATE:
                cg.add(hub.set_respiration_rate_sensor(sens))
            elif key == CONF_SLEEP_SCORE:
                cg.add(hub.set_sleep_score_sensor(sens))
            elif key == CONF_POSITION_X:
                cg.add(hub.set_position_x_sensor(sens))
            elif key == CONF_POSITION_Y:
                cg.add(hub.set_position_y_sensor(sens))
            elif key == CONF_POSITION_Z:
                cg.add(hub.set_position_z_sensor(sens))
