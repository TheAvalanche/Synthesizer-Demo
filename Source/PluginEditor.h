#ifndef __PLUGINEDITOR_H_4ACCBAA__
#define __PLUGINEDITOR_H_4ACCBAA__

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"



class SynthDemoPluginAudioProcessorEditor : public AudioProcessorEditor {
public:
	SynthDemoPluginAudioProcessorEditor(SynthDemoPluginAudioProcessor&);
	~SynthDemoPluginAudioProcessorEditor();

	void paint(Graphics&) override;
	void resized() override;

private:
	class ParameterSlider;

	MidiKeyboardComponent midiKeyboard;
	Label attackLabel, decayLabel, sustainLabel, releaseLabel, gainLabel, delayLabel, mixLabel, lowPassLabel, highPassLabel;
	ScopedPointer<ParameterSlider> attackSlider, decaySlider, sustainSlider, releaseSlider, gainSlider, delaySlider, mixSlider, lowPassSlider, highPassSlider;

	SynthDemoPluginAudioProcessor& getProcessor() const {
		return static_cast<SynthDemoPluginAudioProcessor&> (processor);
	}

};


#endif  // __PLUGINEDITOR_H_4ACCBAA__
