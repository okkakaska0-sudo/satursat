#include "AdaptiveEqualizer.h"

AdaptiveEqualizer::AdaptiveEqualizer()
{
    targetCurveValues.resize(8, 0.0f);
    currentSpectrum.resize(8, 0.0f);
    smoothedSpectrum.resize(8, 0.0f);
    
    // Initialize bands
    for (size_t i = 0; i < bands.size(); ++i)
    {
        bands[i].frequency = bandFrequencies[i];
        bands[i].gain = 0.0f;
        bands[i].targetGain = 0.0f;
        bands[i].smoothedGain = 0.0f;
    }
}

void AdaptiveEqualizer::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    sampleRate = static_cast<float>(spec.sampleRate);
    
    fftProcessor.prepare(spec);
    
    // Prepare processing chains for each channel
    processingChains.clear();
    processingChains.resize(spec.numChannels);
    
    for (auto& chain : processingChains)
    {
        chain.prepare(spec);
    }
    
    // Initialize filter coefficients
    for (size_t i = 0; i < bands.size(); ++i)
    {
        auto& band = bands[i];
        
        // Create bell filter coefficients
        band.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, band.frequency, 2.0f, juce::Decibels::decibelsToGain(band.gain));
        
        // Apply coefficients to all processing chains
        for (auto& chain : processingChains)
        {
            switch (i)
            {
                case 0: chain.template get<0>().coefficients = band.coefficients; break;
                case 1: chain.template get<1>().coefficients = band.coefficients; break;
                case 2: chain.template get<2>().coefficients = band.coefficients; break;
                case 3: chain.template get<3>().coefficients = band.coefficients; break;
                case 4: chain.template get<4>().coefficients = band.coefficients; break;
                case 5: chain.template get<5>().coefficients = band.coefficients; break;
                case 6: chain.template get<6>().coefficients = band.coefficients; break;
                case 7: chain.template get<7>().coefficients = band.coefficients; break;
            }
        }
    }
    
    updateTargetCurve();
    reset();
}

void AdaptiveEqualizer::reset()
{
    fftProcessor.reset();
    
    for (auto& chain : processingChains)
    {
        chain.reset();
    }
    
    std::fill(currentSpectrum.begin(), currentSpectrum.end(), 0.0f);
    std::fill(smoothedSpectrum.begin(), smoothedSpectrum.end(), 0.0f);
    
    for (auto& band : bands)
    {
        band.gain = 0.0f;
        band.smoothedGain = 0.0f;
    }
}

void AdaptiveEqualizer::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
}

void AdaptiveEqualizer::setTargetCurve(int curve)
{
    targetCurveType = juce::jlimit(0, 4, curve);
    updateTargetCurve();
}

void AdaptiveEqualizer::setAdaptionStrength(float strength)
{
    adaptionStrength = juce::jlimit(0.0f, 1.0f, strength / 100.0f);
}

void AdaptiveEqualizer::setReactionSpeed(float speedMs)
{
    reactionSpeed = juce::jlimit(10.0f, 1000.0f, speedMs);
    
    // Calculate smoothing coefficient based on reaction speed
    // Faster reaction = lower smoothing coefficient
    float timeConstant = speedMs / 1000.0f; // Convert to seconds
    smoothingCoeff = std::exp(-1.0f / (timeConstant * sampleRate / 1024.0f)); // Assuming 1024 sample analysis blocks
}

void AdaptiveEqualizer::updateTargetCurve()
{
    switch (targetCurveType)
    {
        case Flat:
            std::fill(targetCurveValues.begin(), targetCurveValues.end(), 0.0f);
            break;
            
        case Musical:
            // Slight bass boost, presence dip, air boost
            targetCurveValues = { 2.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f };
            break;
            
        case Presence:
            // Mid boost for vocal clarity
            targetCurveValues = { -1.0f, 0.0f, 1.0f, 3.0f, 2.0f, 0.0f, -1.0f, 0.0f };
            break;
            
        case Warm:
            // Bass boost, high cut
            targetCurveValues = { 3.0f, 2.0f, 1.0f, 0.0f, -1.0f, -2.0f, -2.0f, -1.0f };
            break;
            
        case Bright:
            // High boost, bass cut
            targetCurveValues = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 3.0f };
            break;
    }
}

void AdaptiveEqualizer::analyzeSpectrum(const juce::dsp::AudioBlock<const float>& block)
{
    // Get FFT analysis
    auto spectrum = fftProcessor.getSpectrum(block);
    
    // Map FFT bins to our 8 bands
    for (size_t i = 0; i < bands.size(); ++i)
    {
        float frequency = bands[i].frequency;
        size_t binIndex = static_cast<size_t>((frequency / (sampleRate * 0.5f)) * spectrum.size());
        binIndex = juce::jlimit(size_t(1), spectrum.size() - 1, binIndex);
        
        // Get magnitude in dB
        float magnitude = spectrum[binIndex];
        float magnitudeDb = juce::Decibels::gainToDecibels(magnitude, -60.0f);
        
        // Smooth the spectrum analysis
        smoothedSpectrum[i] = smoothedSpectrum[i] * smoothingCoeff + magnitudeDb * (1.0f - smoothingCoeff);
        currentSpectrum[i] = magnitudeDb;
    }
}

void AdaptiveEqualizer::updateBandGains()
{
    for (size_t i = 0; i < bands.size(); ++i)
    {
        auto& band = bands[i];
        
        // Calculate difference between current and target
        float difference = targetCurveValues[i] - smoothedSpectrum[i];
        
        // Apply adaption strength
        float correction = difference * adaptionStrength;
        
        // Limit correction amount
        correction = juce::jlimit(-12.0f, 12.0f, correction);
        
        // Smooth the gain changes
        band.targetGain = correction;
        band.smoothedGain = band.smoothedGain * 0.95f + band.targetGain * 0.05f;
        band.gain = band.smoothedGain;
    }
}

void AdaptiveEqualizer::updateFilterCoefficients()
{
    for (size_t i = 0; i < bands.size(); ++i)
    {
        auto& band = bands[i];
        
        // Update coefficients only if gain changed significantly
        if (std::abs(band.gain - band.smoothedGain) > 0.1f)
        {
            band.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate, band.frequency, 2.0f, juce::Decibels::decibelsToGain(band.gain));
            
            // Apply to all processing chains
            for (auto& chain : processingChains)
            {
                switch (i)
                {
                    case 0: chain.template get<0>().coefficients = band.coefficients; break;
                    case 1: chain.template get<1>().coefficients = band.coefficients; break;
                    case 2: chain.template get<2>().coefficients = band.coefficients; break;
                    case 3: chain.template get<3>().coefficients = band.coefficients; break;
                    case 4: chain.template get<4>().coefficients = band.coefficients; break;
                    case 5: chain.template get<5>().coefficients = band.coefficients; break;
                    case 6: chain.template get<6>().coefficients = band.coefficients; break;
                    case 7: chain.template get<7>().coefficients = band.coefficients; break;
                }
            }
        }
    }
}

std::vector<float> AdaptiveEqualizer::getFrequencyResponse() const
{
    std::vector<float> response(bands.size());
    for (size_t i = 0; i < bands.size(); ++i)
    {
        response[i] = bands[i].gain;
    }
    return response;
}

std::vector<float> AdaptiveEqualizer::getTargetCurve() const
{
    return targetCurveValues;
}

std::vector<float> AdaptiveEqualizer::getCurrentSpectrum() const
{
    return currentSpectrum;
}
