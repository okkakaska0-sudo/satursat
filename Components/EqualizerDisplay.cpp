#include "EqualizerDisplay.h"

EqualizerDisplay::EqualizerDisplay(AdaptiveEqualizer& eq) : equalizer(eq)
{
    startTimer(100); // 10 FPS update rate
}

EqualizerDisplay::~EqualizerDisplay()
{
    stopTimer();
}

void EqualizerDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Background
    g.fillAll(CustomLookAndFeel::backgroundColor.darker(0.2f));
    
    // Border
    g.setColour(CustomLookAndFeel::primaryColor);
    g.drawRect(bounds, 1);
    
    bounds.reduce(5, 5);
    
    // Title
    g.setColour(CustomLookAndFeel::textColor);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    auto titleBounds = bounds.removeFromTop(15);
    g.drawText("ADAPTIVE EQUALIZER", titleBounds, juce::Justification::centred);
    
    bounds.removeFromTop(3);
    
    drawFrequencyGrid(g, bounds);
    drawSpectrum(g, bounds);
    drawTargetCurve(g, bounds);
    drawFrequencyResponse(g, bounds);
}

void EqualizerDisplay::resized()
{
    // Nothing specific needed
}

void EqualizerDisplay::timerCallback()
{
    // Update data from equalizer
    currentResponse = equalizer.getFrequencyResponse();
    currentSpectrum = equalizer.getCurrentSpectrum();
    targetCurve = equalizer.getTargetCurve();
    
    repaint();
}

void EqualizerDisplay::drawFrequencyGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.3f));
    
    // Frequency grid lines (logarithmic)
    std::vector<float> frequencies = { 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f };
    
    for (float freq : frequencies)
    {
        float x = juce::jmap(std::log10(freq), std::log10(minFreq), std::log10(maxFreq), 
                           static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }
    
    // Gain grid lines
    for (float gain = -12.0f; gain <= 12.0f; gain += 6.0f)
    {
        float y = juce::jmap(gain, minGain, maxGain, static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()));
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
    }
    
    // 0dB line
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.6f));
    float zeroY = juce::jmap(0.0f, minGain, maxGain, static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()));
    g.drawHorizontalLine(static_cast<int>(zeroY), bounds.getX(), bounds.getRight());
    
    // Frequency labels
    g.setColour(CustomLookAndFeel::textColor.withAlpha(0.7f));
    g.setFont(juce::Font(7.0f));
    
    std::vector<juce::String> freqLabels = { "100", "1k", "10k" };
    std::vector<float> labelFreqs = { 100.0f, 1000.0f, 10000.0f };
    
    for (size_t i = 0; i < freqLabels.size(); ++i)
    {
        float x = juce::jmap(std::log10(labelFreqs[i]), std::log10(minFreq), std::log10(maxFreq), 
                           static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        g.drawText(freqLabels[i], static_cast<int>(x) - 10, bounds.getBottom() + 2, 20, 10, juce::Justification::centred);
    }
}

void EqualizerDisplay::drawSpectrum(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (currentSpectrum.empty())
        return;
    
    // Draw current spectrum as a filled area
    juce::Path spectrumPath;
    bool first = true;
    
    // Band frequencies for the 8-band EQ
    std::array<float, 8> bandFreqs = { 80.0f, 200.0f, 500.0f, 1200.0f, 3000.0f, 6000.0f, 12000.0f, 16000.0f };
    
    for (size_t i = 0; i < currentSpectrum.size() && i < bandFreqs.size(); ++i)
    {
        float freq = bandFreqs[i];
        float magnitude = currentSpectrum[i];
        
        // Map magnitude to display range (assuming magnitude is in dB)
        float normalizedMag = juce::jlimit(minGain, maxGain, magnitude * 0.1f - 30.0f); // Scale and offset
        
        float x = juce::jmap(std::log10(freq), std::log10(minFreq), std::log10(maxFreq), 
                           static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        float y = juce::jmap(normalizedMag, minGain, maxGain, static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()));
        
        if (first)
        {
            spectrumPath.startNewSubPath(x, bounds.getBottom());
            spectrumPath.lineTo(x, y);
            first = false;
        }
        else
        {
            spectrumPath.lineTo(x, y);
        }
    }
    
    if (!first)
    {
        spectrumPath.lineTo(bounds.getRight(), bounds.getBottom());
        spectrumPath.closeSubPath();
        
        g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.3f));
        g.fillPath(spectrumPath);
        
        g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.6f));
        g.strokePath(spectrumPath, juce::PathStrokeType(1.0f));
    }
}

void EqualizerDisplay::drawTargetCurve(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (targetCurve.empty())
        return;
    
    // Draw target curve
    juce::Path targetPath;
    bool first = true;
    
    std::array<float, 8> bandFreqs = { 80.0f, 200.0f, 500.0f, 1200.0f, 3000.0f, 6000.0f, 12000.0f, 16000.0f };
    
    for (size_t i = 0; i < targetCurve.size() && i < bandFreqs.size(); ++i)
    {
        float freq = bandFreqs[i];
        float gain = targetCurve[i];
        
        float x = juce::jmap(std::log10(freq), std::log10(minFreq), std::log10(maxFreq), 
                           static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        float y = juce::jmap(gain, minGain, maxGain, static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()));
        
        if (first)
        {
            targetPath.startNewSubPath(x, y);
            first = false;
        }
        else
        {
            targetPath.lineTo(x, y);
        }
    }
    
    if (!first)
    {
        g.setColour(CustomLookAndFeel::textColor.withAlpha(0.5f));
        
        float dashLength[] = { 3.0f, 3.0f };
        g.strokePath(targetPath, juce::PathStrokeType(1.5f), {}, dashLength, 2);
    }
}

void EqualizerDisplay::drawFrequencyResponse(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (currentResponse.empty())
        return;
    
    // Draw current EQ response
    juce::Path responsePath;
    bool first = true;
    
    std::array<float, 8> bandFreqs = { 80.0f, 200.0f, 500.0f, 1200.0f, 3000.0f, 6000.0f, 12000.0f, 16000.0f };
    
    for (size_t i = 0; i < currentResponse.size() && i < bandFreqs.size(); ++i)
    {
        float freq = bandFreqs[i];
        float gain = currentResponse[i];
        
        float x = juce::jmap(std::log10(freq), std::log10(minFreq), std::log10(maxFreq), 
                           static_cast<float>(bounds.getX()), static_cast<float>(bounds.getRight()));
        float y = juce::jmap(gain, minGain, maxGain, static_cast<float>(bounds.getBottom()), static_cast<float>(bounds.getY()));
        
        if (first)
        {
            responsePath.startNewSubPath(x, y);
            first = false;
        }
        else
        {
            responsePath.lineTo(x, y);
        }
        
        // Draw band indicators
        g.setColour(CustomLookAndFeel::accentColor.withAlpha(0.7f));
        g.fillEllipse(x - 2, y - 2, 4, 4);
    }
    
    if (!first)
    {
        g.setColour(CustomLookAndFeel::accentColor);
        g.strokePath(responsePath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved));
    }
}
