#include "VUMeter.h"

VUMeter::VUMeter(MeterType type, SaturationProcessor* satProcessor, LoudnessCompensator* compensator)
    : meterType(type), saturationProcessor(satProcessor), loudnessCompensator(compensator)
{
    startTimer(1000 / updateRate);
}

VUMeter::~VUMeter()
{
    stopTimer();
}

void VUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(CustomLookAndFeel::primaryColor.darker(0.3f));
    
    // Border
    g.setColour(CustomLookAndFeel::secondaryColor);
    g.drawRect(bounds, 1);
    
    bounds.reduce(3, 3);
    
    // Title
    g.setColour(CustomLookAndFeel::textColor);
    g.setFont(juce::FontOptions().withHeight(10.0f).withStyle("bold"));
    auto titleBounds = bounds.removeFromTop(15);
    juce::String title = (meterType == Input) ? "INPUT" : "OUTPUT";
    g.drawText(title, titleBounds, juce::Justification::centred);
    
    bounds.removeFromTop(5);
    
    // Scale
    auto scaleBounds = bounds.removeFromRight(25);
    paintScale(g, scaleBounds);
    
    bounds.removeFromRight(3);
    
    // Split into left and right channels
    auto leftBounds = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightBounds = bounds;
    
    leftBounds.removeFromRight(1);
    rightBounds.removeFromLeft(1);
    
    paintChannel(g, leftBounds, rmsLevels[0], peakLevels[0], "L");
    paintChannel(g, rightBounds, rmsLevels[1], peakLevels[1], "R");
}

void VUMeter::resized()
{
    // Nothing specific needed here
}

void VUMeter::timerCallback()
{
    if (meterType == Input && saturationProcessor)
    {
        float newRMS[2], newPeak[2];
        for (int channel = 0; channel < 2; ++channel)
        {
            newRMS[channel] = saturationProcessor->getRMSLevel(channel);
            newPeak[channel] = saturationProcessor->getPeakLevel(channel);
            
            // Smooth RMS
            rmsLevels[channel] = rmsLevels[channel] * smoothingFactor + newRMS[channel] * (1.0f - smoothingFactor);
            
            // Peak with hold
            if (newPeak[channel] > peakLevels[channel])
            {
                peakLevels[channel] = newPeak[channel];
                peakHold[channel] = newPeak[channel];
                peakHoldTimer[channel] = peakHoldTime;
            }
            else
            {
                peakLevels[channel] = peakLevels[channel] * 0.95f + newPeak[channel] * 0.05f;
                
                if (peakHoldTimer[channel] > 0)
                {
                    peakHoldTimer[channel]--;
                }
                else
                {
                    peakHold[channel] *= 0.99f;
                }
            }
        }
    }
    else if (meterType == Output && loudnessCompensator)
    {
        // For output meter, we could show compensated levels
        // This would need to be implemented in the main processor
    }
    
    repaint();
}

void VUMeter::setLevels(float rmsLeft, float rmsRight, float peakLeft, float peakRight)
{
    rmsLevels[0] = rmsLeft;
    rmsLevels[1] = rmsRight;
    peakLevels[0] = peakLeft;
    peakLevels[1] = peakRight;
}

void VUMeter::paintChannel(juce::Graphics& g, juce::Rectangle<int> bounds, float rmsLevel, float peakLevel, const juce::String& label)
{
    auto meterBounds = bounds.reduced(2).toFloat();
    
    // Convert levels to dB
    float rmsDb = juce::Decibels::gainToDecibels(rmsLevel, minDb);
    float peakDb = juce::Decibels::gainToDecibels(peakLevel, minDb);
    
    // Normalize to 0-1 range
    float rmsNormalized = juce::jmap(rmsDb, minDb, maxDb, 0.0f, 1.0f);
    float peakNormalized = juce::jmap(peakDb, minDb, maxDb, 0.0f, 1.0f);
    
    rmsNormalized = juce::jlimit(0.0f, 1.0f, rmsNormalized);
    peakNormalized = juce::jlimit(0.0f, 1.0f, peakNormalized);
    
    // Background
    g.setColour(juce::Colours::black);
    g.fillRect(meterBounds);
    
    // RMS level with gradient
    if (rmsNormalized > 0.01f)
    {
        float rmsHeight = meterBounds.getHeight() * rmsNormalized;
        auto rmsRect = juce::Rectangle<float>(meterBounds.getX(), meterBounds.getBottom() - rmsHeight,
                                             meterBounds.getWidth() * 0.8f, rmsHeight);
        
        juce::ColourGradient gradient;
        
        // Green zone (0 to -18dB)
        if (rmsNormalized <= 0.7f)
        {
            gradient = juce::ColourGradient(juce::Colours::darkgreen, 0, meterBounds.getBottom(),
                                          juce::Colours::green, 0, meterBounds.getBottom() - meterBounds.getHeight() * 0.7f, false);
        }
        // Yellow zone (-18 to -6dB)
        else if (rmsNormalized <= 0.9f)
        {
            gradient = juce::ColourGradient(juce::Colours::green, 0, meterBounds.getBottom() - meterBounds.getHeight() * 0.7f,
                                          juce::Colours::yellow, 0, meterBounds.getBottom() - meterBounds.getHeight() * 0.9f, false);
        }
        // Red zone (-6dB and above)
        else
        {
            gradient = juce::ColourGradient(juce::Colours::yellow, 0, meterBounds.getBottom() - meterBounds.getHeight() * 0.9f,
                                          juce::Colours::red, 0, meterBounds.getY(), false);
        }
        
        g.setGradientFill(gradient);
        g.fillRect(rmsRect);
    }
    
    // Peak hold indicator
    int currentChannel = (label == "L") ? 0 : 1;
    float holdDb = juce::Decibels::gainToDecibels(peakHold[currentChannel], minDb);
    float holdNormalized = juce::jmap(holdDb, minDb, maxDb, 0.0f, 1.0f);
    holdNormalized = juce::jlimit(0.0f, 1.0f, holdNormalized);
    
    if (holdNormalized > 0.01f)
    {
        float holdY = meterBounds.getBottom() - (meterBounds.getHeight() * holdNormalized);
        auto holdRect = juce::Rectangle<float>(meterBounds.getRight() - meterBounds.getWidth() * 0.15f, holdY - 1,
                                              meterBounds.getWidth() * 0.15f, 2);
        
        juce::Colour holdColor = holdNormalized > 0.9f ? juce::Colours::red : 
                                holdNormalized > 0.7f ? juce::Colours::yellow : juce::Colours::green;
        
        g.setColour(holdColor);
        g.fillRect(holdRect);
    }
    
    // Channel label
    g.setColour(CustomLookAndFeel::textColor);
    g.setFont(juce::FontOptions().withHeight(9.0f).withStyle("bold"));
    g.drawText(label, bounds.removeFromBottom(12), juce::Justification::centred);
    
    // dB value display
    juce::String dbText = juce::String(rmsDb, 1) + "dB";
    g.setFont(juce::FontOptions().withHeight(8.0f));
    g.drawText(dbText, bounds.removeFromBottom(10), juce::Justification::centred);
}

void VUMeter::paintScale(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(CustomLookAndFeel::textColor.withAlpha(0.7f));
    g.setFont(juce::FontOptions().withHeight(7.0f));
    
    // Draw scale marks at key dB levels
    std::vector<float> scaleDb = { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -30.0f, -42.0f, -60.0f };
    
    for (float db : scaleDb)
    {
        float normalized = juce::jmap(db, minDb, maxDb, 0.0f, 1.0f);
        float y = bounds.getBottom() - (bounds.getHeight() * normalized);
        
        // Draw tick mark
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getX() + 6);
        
        // Draw label for major marks
        if (juce::approximatelyEqual(db, 0.0f) || juce::approximatelyEqual(db, -6.0f) || juce::approximatelyEqual(db, -12.0f) || juce::approximatelyEqual(db, -18.0f) || juce::approximatelyEqual(db, -30.0f))
        {
            juce::String label = juce::approximatelyEqual(db, 0.0f) ? "0" : juce::String(static_cast<int>(db));
            g.drawText(label, bounds.getX() + 8, static_cast<int>(y) - 4, bounds.getWidth() - 8, 8, juce::Justification::left);
        }
    }
}
