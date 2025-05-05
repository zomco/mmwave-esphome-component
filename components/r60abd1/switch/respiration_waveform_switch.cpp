#include "respiration_waveform_switch.h"

namespace esphome {
namespace r60abd1 {

void RespirationWaveformSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_respiration_waveform(state);
}

}  // namespace r60abd1
}  // namespace esphome