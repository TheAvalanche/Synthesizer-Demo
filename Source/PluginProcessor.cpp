#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SineWave.h"
#include "SquareWave.h"
#include "ADSR.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter();

class MixedWaveSound : public SynthesiserSound {
public:
	MixedWaveSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
	bool appliesToChannel(int /*midiChannel*/) override { return true; }
};

class MixedWaveVoice : public SynthesiserVoice {
public:
	MixedWaveVoice(AudioParameterFloat *mixParam, AudioParameterFloat *attackParam, AudioParameterFloat *decayParam, AudioParameterFloat *sustainParam, AudioParameterFloat *releaseParam): angleDelta(0.0), tailOff(0.0) {
		localMixParam = mixParam;
		localAdsr.setADSRParams(attackParam, decayParam, sustainParam, releaseParam);
		localAdsr.setSampleRate(getSampleRate());
	}

	bool canPlaySound(SynthesiserSound* sound) override {
		return dynamic_cast<MixedWaveSound*> (sound) != nullptr;
	}

	void startNote(int midiNoteNumber, float velocity,
		SynthesiserSound* /*sound*/,
		int /*currentPitchWheelPosition*/) override {
		currentAngle = 0.0;
		level = velocity * 0.15;
		tailOff = 0.0;

		double cyclesPerSecond = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
		double cyclesPerSample = cyclesPerSecond / getSampleRate();

		angleDelta = cyclesPerSample * 2.0 * double_Pi;
		
		localAdsr.trigger();
	}

	void stopNote(float /*velocity*/, bool allowTailOff) override {

	}

	void pitchWheelMoved(int /*newValue*/) override {
	}

	void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {
	}

	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
		processBlock(outputBuffer, startSample, numSamples);
	}

	void renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples) override {
		processBlock(outputBuffer, startSample, numSamples);
	}

private:

	template <typename FloatType>
	void processBlock(AudioBuffer<FloatType>& outputBuffer, int startSample, int numSamples) {
		const float mixValue = *localMixParam;
		if (localAdsr.isActive()) {

				while (--numSamples >= 0) {
					const FloatType sinSample = static_cast<FloatType> (sineWave.nextSample(currentAngle));
					const FloatType squareSample = static_cast<FloatType> (squareWave.nextSample(currentAngle));
					double adsrLevel = localAdsr.getLevel(isKeyDown());
					const FloatType currentSample = static_cast<FloatType> ((((1 - mixValue) * sinSample) + (mixValue * squareSample)) * level * adsrLevel);

					for (int i = outputBuffer.getNumChannels(); --i >= 0;)
						outputBuffer.addSample(i, startSample, currentSample);

					currentAngle += angleDelta;
					++startSample;
				}
		} else {
			clearCurrentNote();
		}
	}

	SineWave sineWave;
	SquareWave squareWave;
	AudioParameterFloat* localMixParam;
	AudioParameterFloat* localAttackParam;
	AudioParameterFloat* localDecayParam;
	AudioParameterFloat* localSustainParam;
	AudioParameterFloat* localReleaseParam;
	ADSR localAdsr;
	double currentAngle, angleDelta, level, tailOff;
};

SynthDemoPluginAudioProcessor::SynthDemoPluginAudioProcessor()
	: AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
		.withOutput("Output", AudioChannelSet::stereo(), true)),
	lastUIWidth(400),
	lastUIHeight(200),
	attackParam(nullptr),
	decayParam(nullptr),
	sustainParam(nullptr),
	releaseParam(nullptr),
	gainParam(nullptr),
	delayParam(nullptr),
	mixParam(nullptr),
	lowPassParam(nullptr),
	highPassParam(nullptr),
	delayPosition(0),
	bufLow0(0.0),
	bufLow1(0.0),
	bufHigh0(0.0), 
	bufHigh1(0.0) {
	lastPosInfo.resetToDefault();

	addParameter(attackParam = new AudioParameterFloat("attack", "Attack", 0.0f, 1.0f, 0.5f));
	addParameter(decayParam = new AudioParameterFloat("decay", "Decay", 0.0f, 1.0f, 0.5f));
	addParameter(sustainParam = new AudioParameterFloat("sustain", "Sustain", 0.0f, 1.0f, 1.0f));
	addParameter(releaseParam = new AudioParameterFloat("release", "Release", 0.0f, 1.0f, 0.5f));
	addParameter(gainParam = new AudioParameterFloat("gain", "Gain", 0.0f, 1.0f, 0.5f));
	addParameter(delayParam = new AudioParameterFloat("delay", "Delay Feedback", 0.0f, 1.0f, 0.5f));
	addParameter(mixParam = new AudioParameterFloat("mix", "Mix", 0.0f, 1.0f, 0.5f));
	addParameter(lowPassParam = new AudioParameterFloat("lowPass", "LowPass", 0.01f, 1.0f, 1.0f));
	addParameter(highPassParam = new AudioParameterFloat("highPass", "HighPass", 0.00f, 0.99f, 0.0f));

	initialiseSynth();
}

SynthDemoPluginAudioProcessor::~SynthDemoPluginAudioProcessor() {
}

void SynthDemoPluginAudioProcessor::initialiseSynth() {
	const int numVoices = 8;

	for (int i = 0; i < numVoices; i++) {
		synth.addVoice(new MixedWaveVoice(mixParam, attackParam, decayParam, sustainParam, releaseParam));
	}
	

	synth.addSound(new MixedWaveSound());
}


bool SynthDemoPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {

	const AudioChannelSet& mainInput = layouts.getMainInputChannelSet();
	const AudioChannelSet& mainOutput = layouts.getMainOutputChannelSet();


	if (mainInput != mainOutput) return false;

	if (mainInput.isDisabled()) return false;

	if (mainInput.size() > 2) return false;

	return true;
}

void SynthDemoPluginAudioProcessor::prepareToPlay(double newSampleRate, int /*samplesPerBlock*/) {

	synth.setCurrentPlaybackSampleRate(newSampleRate);
	keyboardState.reset();

	if (isUsingDoublePrecision()) {
		delayBufferDouble.setSize(2, 12000);
		delayBufferFloat.setSize(1, 1);
	}
	else {
		delayBufferFloat.setSize(2, 12000);
		delayBufferDouble.setSize(1, 1);
	}

	reset();
}

void SynthDemoPluginAudioProcessor::releaseResources() {
	keyboardState.reset();
}

void SynthDemoPluginAudioProcessor::reset() {
	delayBufferFloat.clear();
	delayBufferDouble.clear();
}

template <typename FloatType>
void SynthDemoPluginAudioProcessor::process(AudioBuffer<FloatType>& buffer,
	MidiBuffer& midiMessages,
	AudioBuffer<FloatType>& delayBuffer) {
	const int numSamples = buffer.getNumSamples();

	//applyGain(buffer, delayBuffer);

	keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);

	synth.renderNextBlock(buffer, midiMessages, 0, numSamples);

	applyLowPassFilter(buffer, delayBuffer);
	applyHighPassFilter(buffer, delayBuffer);
	
	applyDelay(buffer, delayBuffer);
	for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
		buffer.clear(i, 0, numSamples);
}

template <typename FloatType>
void SynthDemoPluginAudioProcessor::applyGain(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer) {
	ignoreUnused(delayBuffer);
	const float gainLevel = *gainParam;

	for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
		buffer.applyGain(channel, 0, buffer.getNumSamples(), gainLevel);
}

template <typename FloatType>
void SynthDemoPluginAudioProcessor::applyLowPassFilter(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer) {
	ignoreUnused(delayBuffer);

	const int numSamples = buffer.getNumSamples();
	const float cutoff = *lowPassParam;

	double bufPos0 = bufLow0;
	double bufPos1 = bufLow1;

	for (int channel = 0; channel < getTotalNumInputChannels(); ++channel) {
		FloatType* const channelData = buffer.getWritePointer(channel);

		bufLow0 = bufPos0;
		bufLow1 = bufPos1;

		for (int i = 0; i < numSamples; ++i) {
			const FloatType in = channelData[i];
			bufLow0 += cutoff * (in - bufLow0);
			bufLow1 += cutoff * (bufLow0 - bufLow1);
			channelData[i] = bufLow1;
		}
	}
}

template <typename FloatType>
void SynthDemoPluginAudioProcessor::applyHighPassFilter(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer) {
	ignoreUnused(delayBuffer);

	const int numSamples = buffer.getNumSamples();
	const float cutoff = *highPassParam;

	double bufPos0 = bufHigh0;
	double bufPos1 = bufHigh1;

	for (int channel = 0; channel < getTotalNumInputChannels(); ++channel) {
		FloatType* const channelData = buffer.getWritePointer(channel);

		bufHigh0 = bufPos0;
		bufHigh1 = bufPos1;

		for (int i = 0; i < numSamples; ++i) {
			const FloatType in = channelData[i];
			bufHigh0 += cutoff * (in - bufHigh0);
			bufHigh1 += cutoff * (bufHigh0 - bufHigh1);
			channelData[i] = in - bufHigh1;
		}
	}
}

template <typename FloatType>
void SynthDemoPluginAudioProcessor::applyDelay(AudioBuffer<FloatType>& buffer, AudioBuffer<FloatType>& delayBuffer) {
	const int numSamples = buffer.getNumSamples();
	const float delayLevel = *delayParam;

	int delayPos = 0;

	for (int channel = 0; channel < getTotalNumInputChannels(); ++channel) {
		FloatType* const channelData = buffer.getWritePointer(channel);
		FloatType* const delayData = delayBuffer.getWritePointer(jmin(channel, delayBuffer.getNumChannels() - 1));
		delayPos = delayPosition;

		for (int i = 0; i < numSamples; ++i) {
			const FloatType in = channelData[i];
			channelData[i] += delayData[delayPos];
			delayData[delayPos] = (delayData[delayPos] + in) * delayLevel;

			if (++delayPos >= delayBuffer.getNumSamples())
				delayPos = 0;
		}
	}

	delayPosition = delayPos;
}

void SynthDemoPluginAudioProcessor::updateCurrentTimeInfoFromHost() {
	if (AudioPlayHead* ph = getPlayHead()) {
		AudioPlayHead::CurrentPositionInfo newTime;

		if (ph->getCurrentPosition(newTime)) {
			lastPosInfo = newTime;
			return;
		}
	}

	lastPosInfo.resetToDefault();
}

AudioProcessorEditor* SynthDemoPluginAudioProcessor::createEditor() {
	return new SynthDemoPluginAudioProcessorEditor(*this);
}

void SynthDemoPluginAudioProcessor::getStateInformation(MemoryBlock& destData) {

	XmlElement xml("MYPLUGINSETTINGS");


	xml.setAttribute("uiWidth", lastUIWidth);
	xml.setAttribute("uiHeight", lastUIHeight);

	for (int i = 0; i < getNumParameters(); ++i)
		if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
			xml.setAttribute(p->paramID, p->getValue());

	copyXmlToBinary(xml, destData);
}

void SynthDemoPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {

	ScopedPointer<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState != nullptr) {

		if (xmlState->hasTagName("MYPLUGINSETTINGS")) {

			lastUIWidth = jmax(xmlState->getIntAttribute("uiWidth", lastUIWidth), 400);
			lastUIHeight = jmax(xmlState->getIntAttribute("uiHeight", lastUIHeight), 200);

			for (int i = 0; i < getNumParameters(); ++i)
				if (AudioProcessorParameterWithID* p = dynamic_cast<AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
					p->setValue((float)xmlState->getDoubleAttribute(p->paramID, p->getValue()));
		}
	}
}


AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
	return new SynthDemoPluginAudioProcessor();
}
