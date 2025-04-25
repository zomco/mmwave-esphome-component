import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import (
    CONF_ID,
    CONF_OPTIONS,
    CONF_ICON,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
)

# Import the namespace and hub class from __init__.py
from . import r60abd1_ns, R60ABD1Component

# Define key for YAML configuration
CONF_STRUGGLE_SENSITIVITY = "struggle_sensitivity"

# List of select types
TYPES = [
    CONF_STRUGGLE_SENSITIVITY,
]

# Define options for struggle sensitivity
STRUGGLE_SENSITIVITY_OPTIONS = ["Low", "Medium", "High"]  # Corresponds to 0, 1, 2

# Configuration schema for the select entities
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.use_id(R60ABD1Component),  # Reference the hub ID
        # Define schema for struggle sensitivity
        cv.Optional(CONF_STRUGGLE_SENSITIVITY): select.select_schema(
            icon="mdi:tune-variant",  # Icon for sensitivity/tuning
            entity_category=ENTITY_CATEGORY_CONFIG,
            options=STRUGGLE_SENSITIVITY_OPTIONS,  # Set the available options
        ),
    }
)


# Function to generate C++ code for select setup
async def to_code(config):
    hub = await cg.get_variable(config[CONF_ID])  # Get the hub instance

    for key in TYPES:
        if key in config:
            conf = config[key]
            # Create a new Select object
            var = await select.new_select(
                conf,
                options=conf[CONF_OPTIONS],  # Pass options to the select object
            )

            # Generate C++ lambda code to call the hub's setter method
            # 'x' is the selected string option passed from Home Assistant
            if key == CONF_STRUGGLE_SENSITIVITY:
                # Map string option to integer value for the C++ function
                lambda_code = f"""
                    uint8_t level = 0; // Default to Low
                    if (x == "{STRUGGLE_SENSITIVITY_OPTIONS[1]}") {{ // Medium
                        level = 1;
                    }} else if (x == "{STRUGGLE_SENSITIVITY_OPTIONS[2]}") {{ // High
                        level = 2;
                    }}
                    {hub}->set_struggle_sensitivity(level);
                """
                cg.add(var.set_set_action(cg.RawLambda(lambda_code)))
