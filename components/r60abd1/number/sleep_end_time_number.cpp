#include "struggle_sensitivity_select.h"

namespace esphome {
namespace r60abd1 {

void SleepEndTimeNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_sleep_end_time();
}

}  // namespace r60abd1
}  // namespace esphome