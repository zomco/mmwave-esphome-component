import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
    CONF_ID,
    CONF_ICON,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
)

# Import the namespace and hub class from __init__.py
from . import micradar_r60abd1_ns, MicRadarR60ABD1

# Define keys for YAML configuration for each switch
CONF_PRESENCE_DETECTION = "presence_detection"
CONF_HEART_RATE_DETECTION = "heart_rate_detection"
CONF_RESPIRATION_DETECTION = "respiration_detection"
CONF_SLEEP_MONITORING = "sleep_monitoring"
CONF_HEART_RATE_WAVEFORM = "heart_rate_waveform"
CONF_RESPIRATION_WAVEFORM = "respiration_waveform"
CONF_STRUGGLE_DETECTION = "struggle_detection"
CONF_UNATTENDED_DETECTION = "unattended_detection"

# List of all switch types provided by this component
TYPES = [
    CONF_PRESENCE_DETECTION,
    CONF_HEART_RATE_DETECTION,
    CONF_RESPIRATION_DETECTION,
    CONF_SLEEP_MONITORING,
    CONF_HEART_RATE_WAVEFORM,
    CONF_RESPIRATION_WAVEFORM,
    CONF_STRUGGLE_DETECTION,
    CONF_UNATTENDED_DETECTION,
]

# Configuration schema for the switches
# Users will define switches under the 'switch:' platform in their YAML
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(MicRadarR60ABD1),  # Reference the hub ID
        # Define schema for each switch type
        cv.Optional(CONF_PRESENCE_DETECTION): switch.switch_schema(
            icon="mdi:account-search",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_HEART_RATE_DETECTION): switch.switch_schema(
            icon="mdi:heart-pulse",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_RESPIRATION_DETECTION): switch.switch_schema(
            icon="mdi:lungs",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_SLEEP_MONITORING): switch.switch_schema(
            icon="mdi:bed",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_HEART_RATE_WAVEFORM): switch.switch_schema(
            icon="mdi:chart-bell-curve-cumulative",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_RESPIRATION_WAVEFORM): switch.switch_schema(
            icon="mdi:chart-gantt",  # Using gantt as a wave-like icon
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_STRUGGLE_DETECTION): switch.switch_schema(
            icon="mdi:human-handsup",  # Icon representing struggle
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_UNATTENDED_DETECTION): switch.switch_schema(
            icon="mdi:account-clock",  # Icon representing timed absence
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


# Function to generate C++ code for switch setup
async def to_code(config):
    hub = await cg.get_variable(config[CONF_ID])  # Get the hub instance

    for key in TYPES:
        if key in config:
            conf = config[key]
            # Create a new Switch object
            # Use optimistic mode as we don't currently read back the state confirmation
            var = await switch.new_switch(conf, optimistic=True)

            # Generate C++ lambda code to call the hub's methods when the switch is toggled
            if key == CONF_PRESENCE_DETECTION:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_presence_detection(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(f"{hub}->set_presence_detection(false);")
                    )
                )
            elif key == CONF_HEART_RATE_DETECTION:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_heart_rate_detection(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(f"{hub}->set_heart_rate_detection(false);")
                    )
                )
            elif key == CONF_RESPIRATION_DETECTION:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_respiration_detection(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(f"{hub}->set_respiration_detection(false);")
                    )
                )
            elif key == CONF_SLEEP_MONITORING:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_sleep_monitoring(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(f"{hub}->set_sleep_monitoring(false);")
                    )
                )
            elif key == CONF_HEART_RATE_WAVEFORM:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_heart_rate_waveform_reporting(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(
                            f"{hub}->set_heart_rate_waveform_reporting(false);"
                        )
                    )
                )
            elif key == CONF_RESPIRATION_WAVEFORM:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(
                            f"{hub}->set_respiration_waveform_reporting(true);"
                        )
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(
                            f"{hub}->set_respiration_waveform_reporting(false);"
                        )
                    )
                )
            elif key == CONF_STRUGGLE_DETECTION:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_struggle_detection(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(f"{hub}->set_struggle_detection(false);")
                    )
                )
            elif key == CONF_UNATTENDED_DETECTION:
                cg.add(
                    var.set_turn_on_action(
                        cg.RawLambda(f"{hub}->set_unattended_detection(true);")
                    )
                )
                cg.add(
                    var.set_turn_off_action(
                        cg.RawLambda(f"{hub}->set_unattended_detection(false);")
                    )
                )
