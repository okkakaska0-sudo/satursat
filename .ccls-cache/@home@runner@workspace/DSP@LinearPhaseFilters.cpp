#include "LinearPhaseFilters.h"

LinearPhaseFilters::LinearPhaseFilters()
{
}

void LinearPhaseFilters::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    
    for (auto& filter : lowCutFilters)
        filter.prepare(spec);
    
    for (auto& filter : highCutFilters)
        filter.prepare(spec);
    
    updateLowCutFilter();
    updateHighCutFilter();
    
    reset();
}

void LinearPhaseFilters::reset()
{
    for (auto& filter : lowCutFilters)
        filter.reset();
    
    for (auto& filter : highCutFilters)
        filter.reset();
}

void LinearPhaseFilters::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
}

void LinearPhaseFilters::setLowCutFrequency(float frequency)
{
    if (std::abs(lowCutFreq - frequency) > 0.1f)
    {
        lowCutFreq = juce::jlimit(20.0f, sampleRate * 0.45f, frequency);
        updateLowCutFilter();
    }
}

void LinearPhaseFilters::setHighCutFrequency(float frequency)
{
    if (std::abs(highCutFreq - frequency) > 0.1f)
    {
        highCutFreq = juce::jlimit(1000.0f, sampleRate * 0.45f, frequency);
        updateHighCutFilter();
    }
}

void LinearPhaseFilters::updateLowCutFilter()
{
    // Create linear phase high-pass FIR filter
    std::vector<float> coefficients(filterOrder + 1);
    
    // Calculate normalized cutoff frequency
    float normalizedFreq = lowCutFreq / sampleRate;
    
    // Generate windowed sinc high-pass filter
    int center = static_cast<int>(filterOrder / 2);
    
    for (int i = 0; i <= static_cast<int>(filterOrder); ++i)
    {
        int n = i - center;
        
        if (n == 0)
        {
            coefficients[i] = 1.0f - 2.0f * normalizedFreq;
        }
        else
        {
            // Sinc function for high-pass (1 - low-pass)
            float sinc = std::sin(juce::MathConstants<float>::twoPi * normalizedFreq * n) / (juce::MathConstants<float>::pi * n);
            coefficients[i] = -sinc;
            
            // Add impulse at center for high-pass
            if (i == center)
                coefficients[i] += 1.0f;
        }
        
        // Apply Blackman window for better frequency response
        float window = 0.42f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * i / filterOrder) + 
                      0.08f * std::cos(4.0f * juce::MathConstants<float>::pi * i / filterOrder);
        coefficients[i] *= window;
    }
    
    // Apply coefficients to all filters
    for (auto& filter : lowCutFilters)
    {
        *filter.coefficients = juce::dsp::FIR::Coefficients<float>::makeHighPass(sampleRate, lowCutFreq);
    }
}

void LinearPhaseFilters::updateHighCutFilter()
{
    // Create linear phase low-pass FIR filter
    std::vector<float> coefficients(filterOrder + 1);
    
    // Calculate normalized cutoff frequency
    float normalizedFreq = highCutFreq / sampleRate;
    
    // Generate windowed sinc low-pass filter
    int center = static_cast<int>(filterOrder / 2);
    
    for (int i = 0; i <= static_cast<int>(filterOrder); ++i)
    {
        int n = i - center;
        
        if (n == 0)
        {
            coefficients[i] = 2.0f * normalizedFreq;
        }
        else
        {
            // Sinc function for low-pass
            float sinc = std::sin(juce::MathConstants<float>::twoPi * normalizedFreq * n) / (juce::MathConstants<float>::pi * n);
            coefficients[i] = sinc;
        }
        
        // Apply Blackman window (ModifiedBesselI0 removed in JUCE 8.0.8)
        float window = 0.42f - 0.5f * std::cos(2.0f * juce::MathConstants<float>::pi * i / filterOrder) + 
                      0.08f * std::cos(4.0f * juce::MathConstants<float>::pi * i / filterOrder);
        coefficients[i] *= window;
    }
    
    // Apply coefficients to all filters
    for (auto& filter : highCutFilters)
    {
        *filter.coefficients = juce::dsp::FIR::Coefficients<float>::makeLowPass(sampleRate, highCutFreq);
    }
}
