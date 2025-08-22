#include "SaturationVisualization.h"
#include "../Parameters.h"

SaturationVisualization::SaturationVisualization(SaturationProcessor& processor, juce::AudioProcessorValueTreeState& vts)
    : saturationProcessor(processor), valueTreeState(vts)
{
    driveParameter = valueTreeState.getRawParameterValue(ParameterIDs::drive);
    satTypeParameter = valueTreeState.getRawParameterValue(ParameterIDs::satType);
    
    // Verify parameters are valid before starting timer
    if (driveParameter && satTypeParameter)
        startTimer(30); // 30 FPS
}

void SaturationVisualization::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.fillAll(CustomLookAndFeel::backgroundColor);
    
    // Draw border
    g.setColour(CustomLookAndFeel::primaryColor);
    g.drawRect(bounds, 2);
    
    bounds.reduce(4, 4);
    
    // Draw grid
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.3f));
    
    // Vertical grid lines
    for (int i = 1; i < 4; ++i)
    {
        int x = bounds.getX() + (bounds.getWidth() * i) / 4;
        g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
    }
    
    // Horizontal grid lines
    for (int i = 1; i < 4; ++i)
    {
        int y = bounds.getY() + (bounds.getHeight() * i) / 4;
        g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
    }
    
    // Draw center lines
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.5f));
    g.drawHorizontalLine(bounds.getCentreY(), bounds.getX(), bounds.getRight());
    g.drawVerticalLine(bounds.getCentreX(), bounds.getY(), bounds.getBottom());
    
    // Draw saturation curve
    drawSaturationCurve(g, bounds);
    
    // Draw labels
    g.setColour(CustomLookAndFeel::textColor);
    g.setFont(12.0f);
    g.drawText("Input", bounds.getX(), bounds.getBottom() + 5, bounds.getWidth(), 15, juce::Justification::centred);
    
    juce::Graphics::ScopedSaveState saveState(g);
    g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, bounds.getX() - 15, bounds.getCentreY()));
    g.drawText("Output", bounds.getX() - 15, bounds.getCentreY() - 30, 60, 15, juce::Justification::centred);
}

void SaturationVisualization::resized()
{
    // Nothing specific to do here
}

void SaturationVisualization::timerCallback()
{
    repaint();
}

void SaturationVisualization::drawSaturationCurve(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (!driveParameter || !satTypeParameter)
        return;
    
    float drive = driveParameter->load();
    int satType = static_cast<int>(satTypeParameter->load());
    
    juce::Path curve;
    bool first = true;
    
    for (int i = 0; i <= curveResolution; ++i)
    {
        float input = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(curveResolution), -1.0f, 1.0f);
        float output = applySaturationForVisualization(input, satType, drive);
        
        float x = juce::jmap(input, -1.0f, 1.0f, static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        float y = juce::jmap(output, 1.0f, -1.0f, static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
        
        if (first)
        {
            curve.startNewSubPath(x, y);
            first = false;
        }
        else
        {
            curve.lineTo(x, y);
        }
    }
    
    // Draw the curve
    g.setColour(CustomLookAndFeel::accentColor);
    g.strokePath(curve, juce::PathStrokeType(2.0f));
    
    // Draw input=output reference line
    g.setColour(CustomLookAndFeel::textColor.withAlpha(0.3f));
    juce::Path referenceLine;
    referenceLine.startNewSubPath(bounds.getX(), bounds.getBottom());
    referenceLine.lineTo(bounds.getRight(), bounds.getY());
    g.strokePath(referenceLine, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
}

float SaturationVisualization::applySaturationForVisualization(float input, int type, float drive) const
{
    // Apply drive
    float driven = input * juce::Decibels::decibelsToGain(drive);
    
    // Apply saturation based on type
    float output;
    switch (type)
    {
        case 0: // Soft clip
        {
            const float threshold = 0.7f;
            const float ratio = 3.0f;
            
            float absInput = std::abs(driven);
            if (absInput <= threshold)
                output = driven;
            else
            {
                float excess = absInput - threshold;
                float compressedExcess = excess / (1.0f + excess * ratio);
                output = threshold + compressedExcess;
                if (driven < 0.0f) output = -output;
            }
            break;
        }
        case 1: // Hard clip
            output = juce::jlimit(-0.95f, 0.95f, driven);
            break;
        case 2: // Tube distortion
        {
            float x = driven * 2.0f;
            if (std::abs(x) < 1.0f)
                output = x - (x * x * x) / 3.0f;
            else
                output = x > 0.0f ? 2.0f / 3.0f : -2.0f / 3.0f;
            break;
        }
        case 3: // Tape distortion
            output = std::tanh(driven * 1.5f) * 0.8f;
            break;
        default:
            output = driven;
            break;
    }
    
    return juce::jlimit(-1.0f, 1.0f, output);
}
