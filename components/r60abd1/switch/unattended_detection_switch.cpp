#include "presence_detection.switch.h"

namespace esphome {
namespace r60abd1 {

void UnattendedDetectionSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_unattended_detection(state);
}

}  // namespace r60abd1
}  // namespace esphome