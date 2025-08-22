#pragma once

#include <JuceHeader.h>
#include "../SaturationProcessor.h"
#include "../LookAndFeel/CustomLookAndFeel.h"

class VUMeter : public juce::Component, public juce::Timer
{
public:
    VUMeter(SaturationProcessor& processor);
    ~VUMeter() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    void paintChannel(juce::Graphics& g, juce::Rectangle<int> bounds, float rmsLevel, float peakLevel);
    
    SaturationProcessor& saturationProcessor;
    
    float smoothedRMSLevels[2] = { 0.0f, 0.0f };
    float smoothedPeakLevels[2] = { 0.0f, 0.0f };
    
    static constexpr float smoothingFactor = 0.8f;
    static constexpr int updateRate = 60; // Hz

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeter)
};
