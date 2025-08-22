#pragma once

#include <JuceHeader.h>
#include "../DSP/SaturationProcessor.h"
#include "../LookAndFeel/CustomLookAndFeel.h"

class SaturationVisualization : public juce::Component, public juce::Timer
{
public:
    SaturationVisualization(SaturationProcessor& processor, juce::AudioProcessorValueTreeState& vts);
    ~SaturationVisualization() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    void drawSaturationCurve(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawGrid(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawLabels(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawInputOutputLine(juce::Graphics& g, juce::Rectangle<int> bounds);
    
    SaturationProcessor& saturationProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;
    
    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* satTypeParameter = nullptr;
    
    static constexpr int curveResolution = 300;
    static constexpr float inputRange = 2.0f; // ±2.0 for better visualization

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturationVisualization)
};
