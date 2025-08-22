#include "KnobComponent.h"

KnobComponent::KnobComponent(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts, const juce::String& parameterID)
{
    // Setup slider
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, textBoxHeight);
    slider.setLookAndFeel(&customLookAndFeel);
    slider.setMouseDragSensitivity(150); // More precise control
    addAndMakeVisible(slider);
    
    // Setup label
    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(12.0f, juce::Font::bold));
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
    g.fillAll(CustomLookAndFeel::backgroundColor.withAlpha(0.0f));
}

void KnobComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Scale components based on knobSize
    int knobHeight = static_cast<int>(bounds.getHeight() * 0.7f * knobSize);
    int labelHeight = 18;
    int spacing = 5;
    
    label.setBounds(bounds.removeFromTop(labelHeight));
    bounds.removeFromTop(spacing);
    
    // Center the slider
    auto sliderBounds = bounds.removeFromTop(knobHeight);
    int sliderSize = juce::jmin(sliderBounds.getWidth(), sliderBounds.getHeight());
    sliderBounds = juce::Rectangle<int>(
        sliderBounds.getCentreX() - sliderSize / 2,
        sliderBounds.getCentreY() - sliderSize / 2,
        sliderSize, sliderSize);
    
    slider.setBounds(sliderBounds);
}

void KnobComponent::setKnobSize(float size)
{
    knobSize = juce::jlimit(0.5f, 2.0f, size);
    resized();
}

void KnobComponent::setTextBoxHeight(int height)
{
    textBoxHeight = juce::jmax(15, height);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, textBoxHeight);
}
