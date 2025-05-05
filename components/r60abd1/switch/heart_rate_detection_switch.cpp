#include "heart_rate_detection_switch.h"

namespace esphome {
namespace r60abd1 {

void HeartRateDetectionSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_heart_rate_detection(state);
}

}  // namespace r60abd1
}  // namespace esphome