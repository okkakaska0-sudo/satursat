#include "KnobComponent.h"

KnobComponent::KnobComponent(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts, const juce::String& parameterID)
{
    // Setup slider
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(slider);
    
    // Setup label
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(14.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, CustomLookAndFeel::textColor);
    addAndMakeVisible(label);
    
    // Create attachment
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(vts, parameterID, slider);
}

KnobComponent::~KnobComponent()
{
    slider.setLookAndFeel(nullptr);
}

void KnobComponent::paint(juce::Graphics& g)
{
    g.fillAll(CustomLookAndFeel::backgroundColor);
}

void KnobComponent::resized()
{
    auto bounds = getLocalBounds();
    
    label.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(5); // spacing
    slider.setBounds(bounds);
}
