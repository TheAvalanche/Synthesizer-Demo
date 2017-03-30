#ifndef __PLUGINPROCESSOR_H_526ED7A9__
#define __PLUGINPROCESSOR_H_526ED7A9__

#include "../JuceLibraryCode/JuceHeader.h"


class SynthDemoPluginAudioProcessor : public AudioProcessor {
public:

	SynthDemoPluginAudioProcessor();
	~SynthDemoPluginAudioProcessor();

	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	void reset() override;

	void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override {
		jassert(!isUsingDoublePrecision());
		process(buffer, midiMessages, delayBufferFloat);
	}

	void processBlock(AudioBuffer<double>& buffer, MidiBuffer& midiMessages) override {
		jassert(isUsingDoublePrecision());
		process(buffer, midiMessages, delayBufferDouble);
	}

	bool hasEditor() const override { return true; }
	AudioProcessorEditor* createEditor() override;

	const String getName() const override { return JucePlugin_Name; }

	bool acceptsMidi() const override { return true; }
	bool producesMidi() const override { return true; }

	double getTailLengthSeconds() const override { return 0.0; }

	int getNumPrograms() override { return 0; }
	int getCurrentProgram() override { return 0; }
	void setCurrentProgram(int /*index*/) override {}
	const String getProgramName(int /*index*/) override { return String(); }
	void changeProgramName(int /*index*/, const String& /*name*/) override {}

	void getStateInformation(MemoryBlock&) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	MidiKeyboardState keyboardState;

	AudioPlayHead::CurrentPositionInfo lastPosInfo;

	int lastUIWidth, lastUIHeight;

	AudioParameterFloat* attackParam;
	AudioParameterFloat* decayParam;
	AudioParameterFloat* sustainParam;
	AudioParameterFloat* releaseParam;
	AudioParameterFloat* gainParam;
	AudioParameterFloat* delayParam;
	AudioParameterFloat* mixParam;
	AudioParameterFloat* lowPassParam;
	AudioParameterFloat* highPassParam;

private:
	template <typename FloatType>
	void process(AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioBuffer<FloatType>& delayBuffer);
	template <typename FloatType>
	void applyGain(AudioBuffer<FloatType>&, AudioBuffer<FloatType>& delayBuffer);
	template <typename FloatType>
	void applyLowPassFilter(AudioBuffer<FloatType>&, AudioBuffer<FloatType>& delayBuffer);
	template <typename FloatType>
	void applyHighPassFilter(AudioBuffer<FloatType>&, AudioBuffer<FloatType>& delayBuffer);
	template <typename FloatType>
	void applyDelay(AudioBuffer<FloatType>&, AudioBuffer<FloatType>& delayBuffer);

	AudioBuffer<float> delayBufferFloat;
	AudioBuffer<double> delayBufferDouble;
	int delayPosition;
	double bufLow0;
	double bufLow1;
	double bufHigh0;
	double bufHigh1;

	Synthesiser synth;

	void initialiseSynth();
	void updateCurrentTimeInfoFromHost();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthDemoPluginAudioProcessor)
};

#endif  // __PLUGINPROCESSOR_H_526ED7A9__
