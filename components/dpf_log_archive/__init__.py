import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["logger", "web_server", "psram"]
ns = cg.esphome_ns.namespace("dpf_log_archive")
LogArchive = ns.class_("LogArchive", cg.Component)
EventRecorder = ns.class_("EventRecorder", cg.Component)
CONFIG_SCHEMA = cv.Schema({cv.GenerateID(): cv.declare_id(LogArchive), cv.GenerateID("event_recorder_id"): cv.declare_id(EventRecorder)}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    event = cg.new_Pvariable(config["event_recorder_id"])
    await cg.register_component(event, config)
    cg.add_define("USE_LOG_LISTENERS")
