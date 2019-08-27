/*
 * Copyright (C) 2006-2016  Music Technology Group - Universitat Pompeu Fabra
 *
 * This file is part of Essentia
 *
 * Essentia is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 */

#include "MinMax.h"
#include "essentiamath.h"
#include <sstream>

using namespace essentia;
using namespace standard;

const char* MinMax::name = "MinMax";
const char* MinMax::category = "Standard";
const char* MinMax::description = DOC("This algorithm calculates the minimum or maximum value of an array.");

MinMax::OpType MinMax::typeFromString(const std::string& name) const {
  if (name == "min") return MIN;
  if (name == "max") return MAX;

  throw EssentiaException("MinMax: Unknown operation: ", name);
}

void MinMax::compute() {

  const std::vector<Real>& input = _input.get();

  switch (_type) {

  case MIN:
    {
      if (input.empty())
        throw EssentiaException("trying to get min of empty array");
      _index.get() = argmin(input);
      _value.get() = input[_index.get()];
      break;
    }

  case MAX:
    {
      if (input.empty())
        throw EssentiaException("trying to get max of empty array");
      _index.get() = argmax(input);
      _value.get() = input[_index.get()];
      break;
    }

  default:
    throw EssentiaException("MinMax: Unknown operation");
  }

  return;
}
