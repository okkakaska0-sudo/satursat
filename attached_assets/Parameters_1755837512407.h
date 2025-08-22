#pragma once

#include <JuceHeader.h>

namespace ParameterIDs
{
    const juce::String drive { "drive" };
    const juce::String mix { "mix" };
    const juce::String outputGain { "outputGain" };
    const juce::String satType { "satType" };
}

namespace ParameterDefaults
{
    constexpr float drive = 0.0f;
    constexpr float mix = 100.0f;
    constexpr float outputGain = 0.0f;
    constexpr int satType = 0;
}

class ParameterLayout
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::drive,
            "Drive",
            juce::NormalisableRange<float>(0.0f, 20.0f, 0.1f),
            ParameterDefaults::drive,
            "dB"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::mix,
            "Mix",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            ParameterDefaults::mix,
            "%"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::outputGain,
            "Output",
            juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f),
            ParameterDefaults::outputGain,
            "dB"));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            ParameterIDs::satType,
            "Saturation Type",
            juce::StringArray { "Soft", "Hard", "Tube", "Tape" },
            ParameterDefaults::satType));

        return layout;
    }
};
