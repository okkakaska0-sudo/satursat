#pragma once

#include <JuceHeader.h>
#include "../DSP/AdaptiveEqualizer.h"
#include "../LookAndFeel/CustomLookAndFeel.h"

class EqualizerDisplay : public juce::Component, public juce::Timer
{
public:
    EqualizerDisplay(AdaptiveEqualizer& eq);
    ~EqualizerDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    void drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawFrequencyGrid(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSpectrum(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawTargetCurve(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    AdaptiveEqualizer& equalizer;
    
    std::vector<float> currentResponse;
    std::vector<float> currentSpectrum;
    std::vector<float> targetCurve;
    
    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float minGain = -15.0f;
    static constexpr float maxGain = 15.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerDisplay)
};
