#pragma once

#include <JuceHeader.h>

namespace ParameterIDs
{
    const juce::String inputGain { "inputGain" };
    const juce::String drive { "drive" };
    const juce::String mix { "mix" };
    const juce::String outputGain { "outputGain" };
    const juce::String satType { "satType" };
    const juce::String soloSaturation { "soloSaturation" };
    
    // Linear Phase Filters
    const juce::String lowCutFreq { "lowCutFreq" };
    const juce::String highCutFreq { "highCutFreq" };
    const juce::String filterEnabled { "filterEnabled" };
    
    // Adaptive Equalizer
    const juce::String eqEnabled { "eqEnabled" };
    const juce::String eqTargetCurve { "eqTargetCurve" };
    const juce::String eqAdaptionStrength { "eqAdaptionStrength" };
    const juce::String eqReactionSpeed { "eqReactionSpeed" };
}

namespace ParameterDefaults
{
    constexpr float inputGain = 0.0f;
    constexpr float drive = 0.0f;
    constexpr float mix = 100.0f;
    constexpr float outputGain = 0.0f;
    constexpr int satType = 0;
    constexpr bool soloSaturation = false;
    
    constexpr float lowCutFreq = 20.0f;
    constexpr float highCutFreq = 20000.0f;
    constexpr bool filterEnabled = true;
    
    constexpr bool eqEnabled = false;
    constexpr int eqTargetCurve = 0; // Flat
    constexpr float eqAdaptionStrength = 50.0f;
    constexpr float eqReactionSpeed = 100.0f;
}

class ParameterLayout
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        // Input/Output gains
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::inputGain,
            "Input",
            juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f),
            ParameterDefaults::inputGain,
            "dB"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::drive,
            "Drive",
            juce::NormalisableRange<float>(0.0f, 30.0f, 0.1f),
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
            juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f),
            ParameterDefaults::outputGain,
            "dB"));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            ParameterIDs::satType,
            "Saturation Type",
            juce::StringArray { "Tube Warm", "Tape Classic", "Transistor Modern", "Diode Harsh", "Vintage Fuzz" },
            ParameterDefaults::satType));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            ParameterIDs::soloSaturation,
            "Solo Saturation",
            ParameterDefaults::soloSaturation));

        // Linear Phase Filters
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::lowCutFreq,
            "Low Cut",
            juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.3f),
            ParameterDefaults::lowCutFreq,
            "Hz"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::highCutFreq,
            "High Cut",
            juce::NormalisableRange<float>(5000.0f, 20000.0f, 1.0f, 0.3f),
            ParameterDefaults::highCutFreq,
            "Hz"));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            ParameterIDs::filterEnabled,
            "Filters Enable",
            ParameterDefaults::filterEnabled));

        // Adaptive Equalizer
        layout.add(std::make_unique<juce::AudioParameterBool>(
            ParameterIDs::eqEnabled,
            "EQ Enable",
            ParameterDefaults::eqEnabled));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            ParameterIDs::eqTargetCurve,
            "EQ Target",
            juce::StringArray { "Flat", "Musical", "Presence", "Warm", "Bright" },
            ParameterDefaults::eqTargetCurve));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::eqAdaptionStrength,
            "EQ Strength",
            juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
            ParameterDefaults::eqAdaptionStrength,
            "%"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::eqReactionSpeed,
            "EQ Speed",
            juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.3f),
            ParameterDefaults::eqReactionSpeed,
            "ms"));

        return layout;
    }
};
