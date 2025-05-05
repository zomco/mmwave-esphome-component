import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_EMPTY,  # Use empty for things like 'score' or 'count'
    DEVICE_CLASS_SPEED,  # Use for respiration rate (breaths/min)
    ICON_HEART_PULSE,
    ICON_MOTION_SENSOR,
    ICON_RULER,
    STATE_CLASS_MEASUREMENT,
    UNIT_CENTIMETER,
    UNIT_BEATS_PER_MINUTE,
    UNIT_EMPTY,  # For score/count
    UNIT_PERCENT,  # For body movement?
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1Component, CONF_R60ABD1_ID

# Define keys for YAML configuration specific to this component's sensors
CONF_DISTANCE = "distance"
CONF_HEART_RATE = "heart_rate"
CONF_MOTION = "motion"
CONF_BODY_MOVEMENT = "body_movement"
CONF_RESPIRATION_RATE = "respiration_rate"
CONF_SLEEP_SCORE = "sleep_score"
CONF_POSITION_X = "position_x"
CONF_POSITION_Y = "position_y"
CONF_POSITION_Z = "position_z"
CONF_HEART_RATE_WAVE_0 = "heart_rate_wave_0"
CONF_HEART_RATE_WAVE_1 = "heart_rate_wave_1"
CONF_HEART_RATE_WAVE_2 = "heart_rate_wave_2"
CONF_HEART_RATE_WAVE_3 = "heart_rate_wave_3"
CONF_HEART_RATE_WAVE_4 = "heart_rate_wave_4"
CONF_RESPIRATION_RATE_WAVE_0 = "respiration_rate_wave_0"
CONF_RESPIRATION_RATE_WAVE_1 = "respiration_rate_wave_1"
CONF_RESPIRATION_RATE_WAVE_2 = "respiration_rate_wave_2"
CONF_RESPIRATION_RATE_WAVE_3 = "respiration_rate_wave_3"
CONF_RESPIRATION_RATE_WAVE_4 = "respiration_rate_wave_4"

ICON_SCOREBOARD = "mdi:scoreboard"
ICON_LUNGS = "mdi:lungs"
ICON_AXIS_X_ARROW = "mdi:axis-x-arrow"
ICON_AXIS_Y_ARROW = "mdi:axis-y-arrow"
ICON_AXIS_Z_ARROW = "mdi:axis-z-arrow"


# Configuration schema for individual sensors
# Users will define sensors under the 'sensor:' platform in their YAML
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(
            R60ABD1Component
        ),  # Reference the main hub component ID
        cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CENTIMETER,
            icon=ICON_RULER,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_DISTANCE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_MOTION): sensor.sensor_schema(
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
        cv.Optional(CONF_HEART_RATE_WAVE_0): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_HEART_PULSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HEART_RATE_WAVE_1): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_HEART_PULSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HEART_RATE_WAVE_2): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_HEART_PULSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HEART_RATE_WAVE_3): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_HEART_PULSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HEART_RATE_WAVE_4): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_HEART_PULSE,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RESPIRATION_RATE_WAVE_0): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_LUNGS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RESPIRATION_RATE_WAVE_1): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_LUNGS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RESPIRATION_RATE_WAVE_2): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_LUNGS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RESPIRATION_RATE_WAVE_3): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_LUNGS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_RESPIRATION_RATE_WAVE_4): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_LUNGS,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_EMPTY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)


# Function to generate C++ code for sensor setup
async def to_code(config):
    r60abd1_component = await cg.get_variable(config[CONF_R60ABD1_ID])  # Get the hub instance

    if distance := config.get(CONF_DISTANCE):
        sens = await sensor.new_sensor(distance)
        cg.add(r60abd1_component.set_distance_sensor(sens))

    if motion := config.get(CONF_MOTION):
        sens = await sensor.new_sensor(motion)
        cg.add(r60abd1_component.set_motion_sensor(sens))

    if body_movement := config.get(CONF_BODY_MOVEMENT):
        sens = await sensor.new_sensor(body_movement)
        cg.add(r60abd1_component.set_body_movement_sensor(sens))

    if heart_rate := config.get(CONF_HEART_RATE):
        sens = await sensor.new_sensor(heart_rate)
        cg.add(r60abd1_component.set_heart_rate_sensor(sens))

    if respiration_rate := config.get(CONF_RESPIRATION_RATE):
        sens = await sensor.new_sensor(respiration_rate)
        cg.add(r60abd1_component.set_respiration_rate_sensor(sens))

    if sleep_score := config.get(CONF_SLEEP_SCORE):
        sens = await sensor.new_sensor(sleep_score)
        cg.add(r60abd1_component.set_sleep_score_sensor(sens))

    if position_x := config.get(CONF_POSITION_X):
        sens = await sensor.new_sensor(position_x)
        cg.add(r60abd1_component.set_position_x_sensor(sens))

    if position_y := config.get(CONF_POSITION_Y):
        sens = await sensor.new_sensor(position_y)
        cg.add(r60abd1_component.set_position_y_sensor(sens))

    if position_z := config.get(CONF_POSITION_Z):
        sens = await sensor.new_sensor(position_z)
        cg.add(r60abd1_component.set_position_z_sensor(sens))

    if heart_rate_wave_0 := config.get(CONF_HEART_RATE_WAVE_0):
        sens = await sensor.new_sensor(heart_rate_wave_0)
        cg.add(r60abd1_component.set_heart_rate_wave_0_sensor(sens))

    if heart_rate_wave_1 := config.get(CONF_HEART_RATE_WAVE_1):
        sens = await sensor.new_sensor(heart_rate_wave_1)
        cg.add(r60abd1_component.set_heart_rate_wave_1_sensor(sens))

    if heart_rate_wave_2 := config.get(CONF_HEART_RATE_WAVE_2):
        sens = await sensor.new_sensor(heart_rate_wave_2)
        cg.add(r60abd1_component.set_heart_rate_wave_2_sensor(sens))

    if heart_rate_wave_3 := config.get(CONF_HEART_RATE_WAVE_3):
        sens = await sensor.new_sensor(heart_rate_wave_3)
        cg.add(r60abd1_component.set_heart_rate_wave_3_sensor(sens))

    if heart_rate_wave_4 := config.get(CONF_HEART_RATE_WAVE_4):
        sens = await sensor.new_sensor(heart_rate_wave_4)
        cg.add(r60abd1_component.set_heart_rate_wave_4_sensor(sens))

    if respiration_rate_wave_0 := config.get(CONF_RESPIRATION_RATE_WAVE_0):
        sens = await sensor.new_sensor(respiration_rate_wave_0)
        cg.add(r60abd1_component.set_respiration_rate_wave_0_sensor(sens))

    if respiration_rate_wave_1 := config.get(CONF_RESPIRATION_RATE_WAVE_1):
        sens = await sensor.new_sensor(respiration_rate_wave_1)
        cg.add(r60abd1_component.set_respiration_rate_wave_1_sensor(sens))

    if respiration_rate_wave_2 := config.get(CONF_RESPIRATION_RATE_WAVE_2):
        sens = await sensor.new_sensor(respiration_rate_wave_2)
        cg.add(r60abd1_component.set_respiration_rate_wave_2_sensor(sens))

    if respiration_rate_wave_3 := config.get(CONF_RESPIRATION_RATE_WAVE_3):
        sens = await sensor.new_sensor(respiration_rate_wave_3)
        cg.add(r60abd1_component.set_respiration_rate_wave_3_sensor(sens))

    if respiration_rate_wave_4 := config.get(CONF_RESPIRATION_RATE_WAVE_4):
        sens = await sensor.new_sensor(respiration_rate_wave_4)
        cg.add(r60abd1_component.set_respiration_rate_wave_4_sensor(sens))

