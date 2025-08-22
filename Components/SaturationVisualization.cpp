#include "SaturationVisualization.h"
#include "../Parameters.h"

SaturationVisualization::SaturationVisualization(SaturationProcessor& processor, juce::AudioProcessorValueTreeState& vts)
    : saturationProcessor(processor), valueTreeState(vts)
{
    driveParameter = valueTreeState.getRawParameterValue(ParameterIDs::drive);
    satTypeParameter = valueTreeState.getRawParameterValue(ParameterIDs::satType);
    
    if (driveParameter && satTypeParameter)
        startTimer(50); // 20 FPS for smooth visualization
}

SaturationVisualization::~SaturationVisualization()
{
    stopTimer();
}

void SaturationVisualization::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(CustomLookAndFeel::backgroundColor);
    
    // Main border
    g.setColour(CustomLookAndFeel::primaryColor);
    g.drawRect(bounds, 2);
    
    bounds.reduce(8, 8);
    
    // Title
    g.setColour(CustomLookAndFeel::textColor);
    g.setFont(juce::FontOptions().withHeight(12.0f).withStyle("bold"));
    auto titleBounds = bounds.removeFromTop(20);
    g.drawText("SATURATION CURVE", titleBounds, juce::Justification::centred);
    
    bounds.removeFromTop(5);
    
    // Draw visualization components
    drawGrid(g, bounds);
    drawInputOutputLine(g, bounds);
    drawSaturationCurve(g, bounds);
    drawLabels(g, bounds);
}

void SaturationVisualization::resized()
{
    // Nothing specific needed
}

void SaturationVisualization::timerCallback()
{
    repaint();
}

void SaturationVisualization::drawGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.3f));
    
    // Vertical grid lines
    for (int i = 1; i < 8; ++i)
    {
        int x = bounds.getX() + (bounds.getWidth() * i) / 8;
        g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
    }
    
    // Horizontal grid lines
    for (int i = 1; i < 8; ++i)
    {
        int y = bounds.getY() + (bounds.getHeight() * i) / 8;
        g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
    }
    
    // Center lines (0dB reference)
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.6f));
    g.drawHorizontalLine(bounds.getCentreY(), bounds.getX(), bounds.getRight());
    g.drawVerticalLine(bounds.getCentreX(), bounds.getY(), bounds.getBottom());
}

void SaturationVisualization::drawInputOutputLine(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Draw unity gain reference line
    g.setColour(CustomLookAndFeel::textColor.withAlpha(0.4f));
    juce::Path referenceLine;
    referenceLine.startNewSubPath(bounds.getX(), bounds.getBottom());
    referenceLine.lineTo(bounds.getRight(), bounds.getY());
    
    g.strokePath(referenceLine, juce::PathStrokeType(1.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
}

void SaturationVisualization::drawSaturationCurve(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (!driveParameter || !satTypeParameter)
        return;
    
    // Parameters are accessed for curve generation in getSaturationCurveValue
    
    juce::Path curve;
    bool first = true;
    
    // Generate curve points
    for (int i = 0; i <= curveResolution; ++i)
    {
        float input = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(curveResolution), -inputRange, inputRange);
        float output = saturationProcessor.getSaturationCurveValue(input);
        
        // Map to screen coordinates
        float x = juce::jmap(input, -inputRange, inputRange, static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        float y = juce::jmap(output, inputRange, -inputRange, static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
        
        // Clamp to bounds
        x = juce::jlimit(static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()), x);
        y = juce::jlimit(static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()), y);
        
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
    
    // Draw the main curve
    g.setColour(CustomLookAndFeel::accentColor);
    g.strokePath(curve, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw curve with glow effect
    g.setColour(CustomLookAndFeel::accentColor.withAlpha(0.3f));
    g.strokePath(curve, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SaturationVisualization::drawLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(CustomLookAndFeel::textColor.withAlpha(0.8f));
    g.setFont(juce::FontOptions().withHeight(10.0f));
    
    // Input axis label
    g.drawText("Input Level", bounds.getX(), bounds.getBottom() + 5, bounds.getWidth(), 12, juce::Justification::centred);
    
    // Output axis label (rotated)
    juce::Graphics::ScopedSaveState saveState(g);
    g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, bounds.getX() - 15, bounds.getCentreY()));
    g.drawText("Output Level", bounds.getX() - 40, bounds.getCentreY() - 25, 50, 12, juce::Justification::centred);
    
    // Scale markings
    g.setFont(juce::FontOptions().withHeight(8.0f));
    
    // Input scale
    std::vector<float> inputMarks = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
    for (float mark : inputMarks)
    {
        float x = juce::jmap(mark, -inputRange, inputRange, static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        g.drawText(juce::String(mark, 1), static_cast<int>(x) - 10, bounds.getBottom() + 15, 20, 10, juce::Justification::centred);
    }
    
    // Output scale
    std::vector<float> outputMarks = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
    for (float mark : outputMarks)
    {
        float y = juce::jmap(mark, inputRange, -inputRange, static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));
        g.drawText(juce::String(mark, 1), bounds.getX() - 25, static_cast<int>(y) - 5, 20, 10, juce::Justification::centred);
    }
    
    // Saturation type label
    if (satTypeParameter)
    {
        int satType = static_cast<int>(satTypeParameter->load());
        std::vector<juce::String> typeNames = { "Tube Warm", "Tape Classic", "Transistor Modern", "Diode Harsh", "Vintage Fuzz" };
        
        if (satType >= 0 && satType < static_cast<int>(typeNames.size()))
        {
            g.setColour(CustomLookAndFeel::accentColor);
            g.setFont(juce::FontOptions().withHeight(11.0f).withStyle("bold"));
            g.drawText(typeNames[static_cast<size_t>(satType)], bounds.getX(), bounds.getY() - 15, bounds.getWidth(), 12, juce::Justification::centred);
        }
    }
}
