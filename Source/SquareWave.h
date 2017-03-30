#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Wave.h"

class SquareWave : public Wave {
public:
	SquareWave() {};
	~SquareWave() {};

	double sample(double currentAngle) override {
		return std::copysign(1.0, std::sin(currentAngle));
	};


};