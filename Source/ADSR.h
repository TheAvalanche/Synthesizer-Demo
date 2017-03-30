#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ADSR {

public:


	ADSR() {

	}

	void setADSRParams(AudioParameterFloat *attackParam, AudioParameterFloat *decayParam, AudioParameterFloat *sustainParam, AudioParameterFloat *releaseParam) {
		localAttackParam = attackParam;
		localDecayParam = decayParam;
		localSustainParam = sustainParam;
		localReleaseParam = releaseParam;
	}

	void setSampleRate(float newRate) {
		sampleRate = newRate;
		convertSecondsToSamples();
	}

	void trigger() {
		convertSecondsToSamples();
		envelopeState = ATTACK;
		samplesSinceTrigger = 0;
		envLevel = 0;
		active = true;
	}

	bool isActive() {
		return active;
	}

	double getLevel(bool keyIsDown) {
		switch (envelopeState) {
			case ATTACK:
				if (attackSamples == 0) {
					envLevel = 1.0;
					envIncrement = 0.0;
					envelopeState = OFF;
				} else {
					if (samplesSinceTrigger == 0) {
						envIncrement = 1.0 / attackSamples;
					} else if (samplesSinceTrigger > attackSamples) {
						envelopeState = DECAY;
						envCoefficient = getSegmentCoefficient(envLevel, *localSustainParam, decaySamples);
					}

					if (!keyIsDown) {
						envelopeState = RELEASE;
						envCoefficient = getSegmentCoefficient(envLevel, 0.0, releaseSamples);
					}
					envLevel += envIncrement;
				}

				break;
			case DECAY:
				if (samplesSinceTrigger > attackSamples + decaySamples)
					envelopeState = SUSTAIN;
				else if (!keyIsDown) {
					envelopeState = RELEASE;
					envCoefficient = getSegmentCoefficient(envLevel, 0.0, releaseSamples);
				}
				else
					envLevel += envCoefficient * envLevel;
				break;
			case SUSTAIN:
				if (!keyIsDown) {
					envelopeState = RELEASE;
					envCoefficient = getSegmentCoefficient(envLevel, 0.0, releaseSamples);
				}
				break;
			case RELEASE:
				envLevel += envCoefficient * envLevel;
				if (envLevel < 0.001) {
					envelopeState = OFF;
					envLevel = 0.0;
					active = false;
					// caller will need to clear current note
				}
				break;
			case OFF:
				active = keyIsDown;
				break;
		}
		if (envLevel > 1.0)
			envLevel = 1.0;
		if (envLevel < 0.0)
			envLevel = 0.0;

		samplesSinceTrigger++;
		return envLevel;
	}

	int envelopeState = OFF;

	enum ADSRState {
		OFF, ATTACK, DECAY, SUSTAIN, RELEASE
	};

	AudioParameterFloat* localAttackParam;
	AudioParameterFloat* localDecayParam;
	AudioParameterFloat* localSustainParam;
	AudioParameterFloat* localReleaseParam;


private:

	inline double getSegmentCoefficient(double startLevel, double endLevel, int durationInSamples) const {
		// add a tiny fudge factor when calculating because it doesn't work when levels are exactly 0.0
		return (log((endLevel + 0.0001)) - log(startLevel + 0.0001)) / durationInSamples;
	}

	void convertSecondsToSamples() {
		const float attack = *localAttackParam;
		const float decay = *localDecayParam;
		const float release = *localReleaseParam;
		attackSamples = sampleRate * attack;
		decaySamples = sampleRate * decay;
		releaseSamples = sampleRate * release;
	}

	unsigned long samplesSinceTrigger = 0;
	double sampleRate = 44100.0;
	double envLevel = 0.0;
	float envCoefficient = 0.0;
	float envIncrement = 0.0;

	unsigned long attackSamples = 0;
	unsigned long decaySamples = 0;
	unsigned long releaseSamples = 0;

	bool active = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSR)
};