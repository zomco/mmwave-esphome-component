#include "unattended_time_number.h"

namespace esphome {
namespace r60abd1 {

void UnattendedTimeNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_unattended_time(value);
}

}  // namespace r60abd1
}  // namespace esphome