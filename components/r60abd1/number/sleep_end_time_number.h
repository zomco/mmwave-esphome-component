#pragma once

#include "esphome/components/select/select.h"
#include "../r60abd1.h"

namespace esphome {
namespace r60abd1 {

class SleepEndTimeNumber : public number::Number, public Parented<R60ABD1Component> {
 public:
  SleepEndTimeNumber() = default;

 protected:
  void control(float value) override;
};

}  // namespace r60abd1
}  // namespace esphome