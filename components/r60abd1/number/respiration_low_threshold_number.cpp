#include "struggle_sensitivity_select.h"

namespace esphome {
namespace r60abd1 {

void RespirationLowThresholdNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_respiration_low_threshold(value);
}

}  // namespace r60abd1
}  // namespace esphome