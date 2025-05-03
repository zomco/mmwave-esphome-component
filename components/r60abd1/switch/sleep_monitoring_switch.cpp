#include "presence_detection.switch.h"

namespace esphome {
namespace r60abd1 {

void SleepMonitoringSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_sleep_monitoring(state);
}

}  // namespace r60abd1
}  // namespace esphome