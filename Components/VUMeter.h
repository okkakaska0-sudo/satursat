#pragma once

#include <JuceHeader.h>
#include "../DSP/SaturationProcessor.h"
#include "../DSP/LoudnessCompensator.h"
#include "../LookAndFeel/CustomLookAndFeel.h"

class VUMeter : public juce::Component, public juce::Timer
{
public:
    enum MeterType
    {
        Input,
        Output
    };

    VUMeter(MeterType type, SaturationProcessor* satProcessor = nullptr, LoudnessCompensator* compensator = nullptr);
    ~VUMeter() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    void setLevels(float rmsLeft, float rmsRight, float peakLeft, float peakRight);

private:
    void paintChannel(juce::Graphics& g, juce::Rectangle<int> bounds, float rmsLevel, float peakLevel, const juce::String& label);
    void paintScale(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    MeterType meterType;
    SaturationProcessor* saturationProcessor;
    LoudnessCompensator* loudnessCompensator;
    
    float rmsLevels[2] = { 0.0f, 0.0f };
    float peakLevels[2] = { 0.0f, 0.0f };
    float peakHold[2] = { 0.0f, 0.0f };
    int peakHoldTimer[2] = { 0, 0 };
    
    static constexpr float smoothingFactor = 0.8f;
    static constexpr int updateRate = 60; // Hz
    static constexpr int peakHoldTime = 90; // frames
    static constexpr float minDb = -60.0f;
    static constexpr float maxDb = 6.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
