import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import (
    CONF_OPTIONS,
    CONF_ICON,
    CONF_ENTITY_CATEGORY,
    ENTITY_CATEGORY_CONFIG,
)

# Import the namespace and hub class from __init__.py
from .. import r60abd1_ns, R60ABD1Component, CONF_R60ABD1_ID

# Define key for YAML configuration
CONF_STRUGGLE_SENSITIVITY = "struggle_sensitivity"

# Define options for struggle sensitivity
STRUGGLE_SENSITIVITY_OPTIONS = ["Low", "Medium", "High"]  # Corresponds to 0, 1, 2

# Configuration schema for the select entities
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_R60ABD1_ID): cv.use_id(R60ABD1Component),  # Reference the hub ID
        # Define schema for struggle sensitivity
        cv.Optional(CONF_STRUGGLE_SENSITIVITY): select.select_schema(
            r60abd1_ns.class_("StruggleSensitivitySelect", select.Select),
            icon="mdi:tune-variant",  # Icon for sensitivity/tuning
            entity_category=ENTITY_CATEGORY_CONFIG,
            # options=STRUGGLE_SENSITIVITY_OPTIONS,  # Set the available options
        ),
    }
)

# Function to generate C++ code for select setup
async def to_code(config):
    r60abd1_component = await cg.get_variable(config[CONF_R60ABD1_ID])  # Get the hub instance

    if struggle_sensitivity := config.get(CONF_STRUGGLE_SENSITIVITY):
        s = await select.new_select(
                struggle_sensitivity,
                options=STRUGGLE_SENSITIVITY_OPTIONS,  # Pass options to the select object
            )
        await cg.register_parented(s, config[CONF_R60ABD1_ID])
        cg.add(r60abd1_component.set_struggle_sensitivity_select(s))
