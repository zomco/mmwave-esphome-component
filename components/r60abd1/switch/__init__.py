import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ICON,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
)

# Import the namespace and hub class from __init__.py
from .. import r60abd1_ns, R60ABD1Component, CONF_R60ABD1_ID

# Define keys for YAML configuration for each switch
CONF_PRESENCE_DETECTION = "presence_detection"
CONF_HEART_RATE_DETECTION = "heart_rate_detection"
CONF_RESPIRATION_DETECTION = "respiration_detection"
CONF_SLEEP_MONITORING = "sleep_monitoring"
CONF_HEART_RATE_WAVEFORMFORM = "heart_rate_waveform"
CONF_RESPIRATION_WAVEFORM = "respiration_waveform"
CONF_STRUGGLE_DETECTION = "struggle_detection"
CONF_UNATTENDED_DETECTION = "unattended_detection"

# Configuration schema for the switches
# Users will define switches under the 'switch:' platform in their YAML
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(R60ABD1Component),  # Reference the hub ID
        # Define schema for each switch type
        cv.Optional(CONF_PRESENCE_DETECTION): switch.switch_schema(
            r60abd1_ns.class_("PresenceDetectionSwitch", switch.Switch),
            icon="mdi:account-search",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_HEART_RATE_DETECTION): switch.switch_schema(
            r60abd1_ns.class_("HeartRateDetectionSwitch", switch.Switch),
            icon="mdi:heart-pulse",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_RESPIRATION_DETECTION): switch.switch_schema(
            r60abd1_ns.class_("RespirationDetectionSwitch", switch.Switch),
            icon="mdi:lungs",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_SLEEP_MONITORING): switch.switch_schema(
            r60abd1_ns.class_("SleepMonitoringSwitch", switch.Switch),
            icon="mdi:bed",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_HEART_RATE_WAVEFORMFORM): switch.switch_schema(
            r60abd1_ns.class_("HeartRateWaveformSwitch", switch.Switch),
            icon="mdi:chart-bell-curve-cumulative",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_RESPIRATION_WAVEFORM): switch.switch_schema(
            r60abd1_ns.class_("RespirationWaveformSwitch", switch.Switch),
            icon="mdi:chart-gantt",  # Using gantt as a wave-like icon
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_STRUGGLE_DETECTION): switch.switch_schema(
            r60abd1_ns.class_("StruggleDetectionSwitch", switch.Switch),
            icon="mdi:human-handsup",  # Icon representing struggle
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_UNATTENDED_DETECTION): switch.switch_schema(
            r60abd1_ns.class_("UnattendedDetectionSwitch", switch.Switch),
            icon="mdi:account-clock",  # Icon representing timed absence
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


# Function to generate C++ code for switch setup
async def to_code(config):
    r60abd1_component = await cg.get_variable(config[CONF_R60ABD1_ID])  # Get the hub instance

    if presence_detection := config.get(CONF_PRESENCE_DETECTION):
        s = await switch.new_switch(presence_detection)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_presence_detection_switch(s))
    
    if heart_rate_detection := config.get(CONF_HEART_RATE_DETECTION):
        s = await switch.new_switch(heart_rate_detection)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_heart_rate_detection_switch(s))

    if respiration_detection := config.get(CONF_RESPIRATION_DETECTION):
        s = await switch.new_switch(respiration_detection)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_respiration_detection_switch(s))

    if sleep_monitoring := config.get(CONF_SLEEP_MONITORING):
        s = await switch.new_switch(sleep_monitoring)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_sleep_monitoring_switch(s))

    if heart_rate_waveform := config.get(CONF_HEART_RATE_WAVEFORMFORM):
        s = await switch.new_switch(heart_rate_waveform)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_heart_rate_waveform_switch(s))

    if respiration_waveform := config.get(CONF_RESPIRATION_WAVEFORM):
        s = await switch.new_switch(respiration_waveform)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_respiration_waveform_switch(s))

    if struggle_detection := config.get(CONF_STRUGGLE_DETECTION):
        s = await switch.new_switch(struggle_detection)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_struggle_detection_switch(s))

    if unattended_detection := config.get(CONF_UNATTENDED_DETECTION):
        s = await switch.new_switch(unattended_detection)
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_unattended_detection_switch(s))
