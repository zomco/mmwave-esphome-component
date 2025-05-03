#pragma once

#include "esphome/components/switch/switch.h"
#include "../r60abd1.h"

namespace esphome {
namespace r60abd1 {

class HeartRateDetectionSwitch : public switch_::Switch, public Parented<R60ABD1Component> {
 public:
  HeartRateDetectionSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace r60abd1
}  // namespace esphome