#include "VUMeter.h"

VUMeter::VUMeter(SaturationProcessor& processor)
    : saturationProcessor(processor)
{
    startTimer(1000 / updateRate);
}

void VUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.fillAll(CustomLookAndFeel::primaryColor);
    
    // Draw border
    g.setColour(CustomLookAndFeel::secondaryColor);
    g.drawRect(bounds, 1);
    
    bounds.reduce(2, 2);
    
    // Split into left and right channels
    auto leftBounds = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightBounds = bounds;
    
    leftBounds.removeFromRight(1); // spacing
    rightBounds.removeFromLeft(1); // spacing
    
    paintChannel(g, leftBounds, smoothedRMSLevels[0], smoothedPeakLevels[0]);
    paintChannel(g, rightBounds, smoothedRMSLevels[1], smoothedPeakLevels[1]);
    
    // Draw channel labels
    g.setColour(CustomLookAndFeel::textColor);
    g.setFont(10.0f);
    g.drawText("L", leftBounds.removeFromBottom(12), juce::Justification::centred);
    g.drawText("R", rightBounds.removeFromBottom(12), juce::Justification::centred);
}

void VUMeter::resized()
{
    // Nothing specific to do here
}

void VUMeter::timerCallback()
{
    for (int channel = 0; channel < 2; ++channel)
    {
        float currentRMS = saturationProcessor.getRMSLevel(channel);
        float currentPeak = saturationProcessor.getPeakLevel(channel);
        
        smoothedRMSLevels[channel] = smoothedRMSLevels[channel] * smoothingFactor + currentRMS * (1.0f - smoothingFactor);
        smoothedPeakLevels[channel] = smoothedPeakLevels[channel] * smoothingFactor + currentPeak * (1.0f - smoothingFactor);
    }
    
    repaint();
}

void VUMeter::paintChannel(juce::Graphics& g, juce::Rectangle<int> bounds, float rmsLevel, float peakLevel)
{
    auto meterBounds = bounds.reduced(1).toFloat();
    
    // Convert levels to dB and normalize to 0-1 range
    float rmsDb = juce::Decibels::gainToDecibels(rmsLevel, -60.0f);
    float peakDb = juce::Decibels::gainToDecibels(peakLevel, -60.0f);
    
    float rmsNormalized = juce::jmap(rmsDb, -60.0f, 0.0f, 0.0f, 1.0f);
    float peakNormalized = juce::jmap(peakDb, -60.0f, 0.0f, 0.0f, 1.0f);
    
    rmsNormalized = juce::jlimit(0.0f, 1.0f, rmsNormalized);
    peakNormalized = juce::jlimit(0.0f, 1.0f, peakNormalized);
    
    // Draw RMS level
    float rmsHeight = meterBounds.getHeight() * rmsNormalized;
    auto rmsRect = juce::Rectangle<float>(meterBounds.getX(), meterBounds.getBottom() - rmsHeight,
                                         meterBounds.getWidth() * 0.7f, rmsHeight);
    
    juce::ColourGradient rmsGradient(juce::Colours::green, 0, meterBounds.getBottom(),
                                    juce::Colours::red, 0, meterBounds.getY(), false);
    rmsGradient.addColour(0.7, juce::Colours::yellow);
    
    g.setGradientFill(rmsGradient);
    g.fillRect(rmsRect);
    
    // Draw peak level
    if (peakNormalized > 0.01f)
    {
        float peakY = meterBounds.getBottom() - (meterBounds.getHeight() * peakNormalized);
        auto peakRect = juce::Rectangle<float>(meterBounds.getRight() - meterBounds.getWidth() * 0.25f, peakY - 1,
                                              meterBounds.getWidth() * 0.25f, 2);
        
        juce::Colour peakColor = peakNormalized > 0.9f ? juce::Colours::red : 
                                peakNormalized > 0.7f ? juce::Colours::yellow : juce::Colours::green;
        
        g.setColour(peakColor);
        g.fillRect(peakRect);
    }
}
