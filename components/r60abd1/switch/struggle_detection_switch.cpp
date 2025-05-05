#include "struggle_detection_switch.h"

namespace esphome {
namespace r60abd1 {

void StruggleDetectionSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_struggle_detection(state);
}

}  // namespace r60abd1
}  // namespace esphome