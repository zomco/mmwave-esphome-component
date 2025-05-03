#include "presence_detection.switch.h"

namespace esphome {
namespace r60abd1 {

void HeartRateWaveformSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_heart_rate_waveform(state);
}

}  // namespace r60abd1
}  // namespace esphome