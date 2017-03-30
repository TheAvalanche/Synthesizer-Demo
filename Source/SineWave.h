#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Wave.h"

class SineWave : public Wave {
public:
	SineWave() {};
	~SineWave() {};

	double sample(double currentAngle) override {
		return sin(currentAngle);
	};

};