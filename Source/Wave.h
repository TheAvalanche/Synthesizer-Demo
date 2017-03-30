#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class Wave {
public:
	Wave() {};
	~Wave() {};

	double nextSample(double currentAngle) {
		return sample(currentAngle);
	};
protected:
	virtual double sample(double currentAngle) = 0;
};