#pragma once

#include "ann/layers/dense.h"

namespace Optimizers {
// Base optimizer class - only to be inherited, doesn't contain any logic
class Optimizer {
public:
  virtual void preUpdate() = 0;

  virtual void updateParams(Layer::Dense &layer) const = 0;

  virtual void postUpdate() = 0;

  virtual float learningRate() const = 0;
};
} // namespace Optimizers
