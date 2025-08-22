#pragma once

#include <JuceHeader.h>
#include "../LookAndFeel/CustomLookAndFeel.h"

class KnobComponent : public juce::Component
{
public:
    KnobComponent(const juce::String& labelText, juce::AudioProcessorValueTreeState& vts, const juce::String& parameterID);
    ~KnobComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    CustomLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobComponent)
};
