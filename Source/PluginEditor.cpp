#include "PluginProcessor.h"
#include "PluginEditor.h"


class SynthDemoPluginAudioProcessorEditor::ParameterSlider : public Slider,
	private Timer {
public:
	ParameterSlider(AudioProcessorParameter& p)
		: Slider(p.getName(256)), param(p) {
		setRange(0.0, 1.0, 0.0);
		startTimerHz(30);
		updateSliderPos();
	}

	void valueChanged() override {
		if (isMouseButtonDown())
			param.setValueNotifyingHost((float)Slider::getValue());
		else
			param.setValue((float)Slider::getValue());
	}

	void timerCallback() override { updateSliderPos(); }

	void startedDragging() override { param.beginChangeGesture(); }
	void stoppedDragging() override { param.endChangeGesture(); }

	double getValueFromText(const String& text) override { return param.getValueForText(text); }
	String getTextFromValue(double value) override { return param.getText((float)value, 1024); }

	void updateSliderPos() {
		const float newValue = param.getValue();

		if (newValue != (float)Slider::getValue() && !isMouseButtonDown())
			Slider::setValue(newValue);
	}

	AudioProcessorParameter& param;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterSlider)
};

SynthDemoPluginAudioProcessorEditor::SynthDemoPluginAudioProcessorEditor(SynthDemoPluginAudioProcessor& owner)
	: AudioProcessorEditor(owner),
	midiKeyboard(owner.keyboardState, MidiKeyboardComponent::horizontalKeyboard),
	attackLabel(String(), "A"),
	decayLabel(String(), "D"),
	sustainLabel(String(), "S"),
	releaseLabel(String(), "R"),
	gainLabel(String(), "Throughput:"),
	delayLabel(String(), "Delay:"),
	mixLabel(String(), "Mix:"),
	lowPassLabel(String(), "LowPass"),
	highPassLabel(String(), "HighPass") {

	getLookAndFeel().setColour(Slider::rotarySliderFillColourId, Colours::red);
	getLookAndFeel().setColour(Slider::rotarySliderOutlineColourId, Colours::orange);
	getLookAndFeel().setColour(Slider::thumbColourId, Colours::red);
	getLookAndFeel().setColour(Slider::trackColourId, Colours::darkgrey);
	getLookAndFeel().setColour(Label::textColourId, Colours::orange);
	getLookAndFeel().setColour(Label::textWhenEditingColourId, Colours::white);
	getLookAndFeel().setColour(Slider::textBoxTextColourId, Colours::orange);
	getLookAndFeel().setColour(Slider::textBoxBackgroundColourId, Colours::darkgrey);
	getLookAndFeel().setColour(MidiKeyboardComponent::whiteNoteColourId, Colour(0x3C, 0x3F, 0x41));
	getLookAndFeel().setColour(MidiKeyboardComponent::mouseOverKeyOverlayColourId, Colours::orange);
	getLookAndFeel().setColour(MidiKeyboardComponent::keyDownOverlayColourId, Colours::red);
	getLookAndFeel().setColour(MidiKeyboardComponent::upDownButtonBackgroundColourId, Colours::orange);
	getLookAndFeel().setColour(MidiKeyboardComponent::upDownButtonArrowColourId, Colours::red);

	addAndMakeVisible(mixSlider = new ParameterSlider(*owner.mixParam));
	mixSlider->setSliderStyle(Slider::Rotary);
	mixSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(gainSlider = new ParameterSlider(*owner.gainParam));
	gainSlider->setSliderStyle(Slider::Rotary);
	gainSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(delaySlider = new ParameterSlider(*owner.delayParam));
	delaySlider->setSliderStyle(Slider::Rotary);
	delaySlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(lowPassSlider = new ParameterSlider(*owner.lowPassParam));
	lowPassSlider->setSliderStyle(Slider::Rotary);
	lowPassSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(highPassSlider = new ParameterSlider(*owner.highPassParam));
	highPassSlider->setSliderStyle(Slider::Rotary);
	highPassSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(attackSlider = new ParameterSlider(*owner.attackParam));
	attackSlider->setSliderStyle(Slider::LinearVertical);
	attackSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(decaySlider = new ParameterSlider(*owner.decayParam));
	decaySlider->setSliderStyle(Slider::LinearVertical);
	decaySlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(sustainSlider = new ParameterSlider(*owner.sustainParam));
	sustainSlider->setSliderStyle(Slider::LinearVertical);
	sustainSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	addAndMakeVisible(releaseSlider = new ParameterSlider(*owner.releaseParam));
	releaseSlider->setSliderStyle(Slider::LinearVertical);
	releaseSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 40, 15);

	gainLabel.attachToComponent(gainSlider, false);
	gainLabel.setFont(Font(13.0f));

	delayLabel.attachToComponent(delaySlider, false);
	delayLabel.setFont(Font(13.0f));

	mixLabel.attachToComponent(mixSlider, false);
	mixLabel.setFont(Font(13.0f));

	lowPassLabel.attachToComponent(lowPassSlider, false);
	lowPassLabel.setFont(Font(13.0f));

	highPassLabel.attachToComponent(highPassSlider, false);
	highPassLabel.setFont(Font(13.0f));

	attackLabel.attachToComponent(attackSlider, false);
	attackLabel.setFont(Font(13.0f));

	decayLabel.attachToComponent(decaySlider, false);
	decayLabel.setFont(Font(13.0f));

	sustainLabel.attachToComponent(sustainSlider, false);
	sustainLabel.setFont(Font(13.0f));

	releaseLabel.attachToComponent(releaseSlider, false);
	releaseLabel.setFont(Font(13.0f));

	addAndMakeVisible(midiKeyboard);

	setResizeLimits(416, 216, 416, 216);

	setSize(owner.lastUIWidth, owner.lastUIHeight);

}

SynthDemoPluginAudioProcessorEditor::~SynthDemoPluginAudioProcessorEditor() {
}

void SynthDemoPluginAudioProcessorEditor::paint(Graphics& g) {
	g.setColour(Colour(0x2B, 0x2B, 0x2B));
	g.fillAll();
	g.setColour(Colours::red);
	g.drawRoundedRectangle(8, 8, 194, 122, 5, 1);
	g.drawRoundedRectangle(210, 8, 124, 122, 5, 1);
	g.drawRoundedRectangle(210, 8, 124, 122, 5, 1);
	g.drawRoundedRectangle(342, 8, 66, 122, 5, 1);
}

void SynthDemoPluginAudioProcessorEditor::resized() {

	Rectangle<int> r(getLocalBounds().reduced(8));

	midiKeyboard.setBounds(r.removeFromBottom(70));

	r.removeFromTop(30);
	Rectangle<int> sliderArea(r.removeFromTop(70));

	Rectangle<int> oscArea(sliderArea.removeFromLeft(194));

	oscArea.removeFromLeft(8);
	mixSlider->setBounds(oscArea.removeFromLeft(50));
	oscArea.removeFromLeft(8);
	attackSlider->setBounds(oscArea.removeFromLeft(30));
	decaySlider->setBounds(oscArea.removeFromLeft(30));
	sustainSlider->setBounds(oscArea.removeFromLeft(30));
	releaseSlider->setBounds(oscArea.removeFromLeft(30));

	sliderArea.removeFromLeft(8);
	sliderArea.removeFromLeft(8);
	highPassSlider->setBounds(sliderArea.removeFromLeft(50));
	sliderArea.removeFromLeft(8);
	lowPassSlider->setBounds(sliderArea.removeFromLeft(50));
	sliderArea.removeFromLeft(8);

	sliderArea.removeFromLeft(8);
	sliderArea.removeFromLeft(8);
	delaySlider->setBounds(sliderArea.removeFromLeft(50));

	getProcessor().lastUIWidth = getWidth();
	getProcessor().lastUIHeight = getHeight();
}

