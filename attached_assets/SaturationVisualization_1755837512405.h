#pragma once

#include <JuceHeader.h>
#include "../SaturationProcessor.h"
#include "../LookAndFeel/CustomLookAndFeel.h"

class SaturationVisualization : public juce::Component, public juce::Timer
{
public:
    SaturationVisualization(SaturationProcessor& processor, juce::AudioProcessorValueTreeState& vts);
    ~SaturationVisualization() override
    {
        stopTimer();
    }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    void drawSaturationCurve(juce::Graphics& g, juce::Rectangle<int> bounds);
    float applySaturationForVisualization(float input, int type, float drive) const;
    
    SaturationProcessor& saturationProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    std::atomic<float>* driveParameter;
    std::atomic<float>* satTypeParameter;
    
    static constexpr int curveResolution = 200;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturationVisualization)
};
