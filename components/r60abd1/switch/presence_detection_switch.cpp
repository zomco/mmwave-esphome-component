#include "presence_detection_switch.h"

namespace esphome {
namespace r60abd1 {

void PresenceDetectionSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_presence_detection(state);
}

}  // namespace r60abd1
}  // namespace esphome