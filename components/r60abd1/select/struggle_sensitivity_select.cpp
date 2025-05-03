#include "struggle_sensitivity_select.h"

namespace esphome {
namespace r60abd1 {

void StruggleSensitivitySelect::control(const std::string &value) {
  this->publish_state(value);
  this->parent_->set_struggle_sensitivity(state);
}

}  // namespace r60abd1
}  // namespace esphome