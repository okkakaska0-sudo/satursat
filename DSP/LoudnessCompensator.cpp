#include "LoudnessCompensator.h"

LoudnessCompensator::LoudnessCompensator()
{
    gainSmoother.setAttackTime(50.0f);  // 50ms attack
    gainSmoother.setReleaseTime(200.0f); // 200ms release
}

void LoudnessCompensator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    
    // Initialize K-weighting filters for each channel
    kWeightingFilters.clear();
    kWeightingFilters.resize(spec.numChannels);
    
    for (auto& filter : kWeightingFilters)
    {
        filter.prepare(spec);
    }
    
    gainSmoother.prepare(spec);
    
    // Initialize loudness buffers
    size_t bufferSize = static_cast<size_t>(measurementTime * sampleRate / 1024); // Assuming 1024 sample blocks
    inputLoudnessBuffer.resize(bufferSize, 0.0f);
    outputLoudnessBuffer.resize(bufferSize, 0.0f);
    
    reset();
}

void LoudnessCompensator::reset()
{
    for (auto& filter : kWeightingFilters)
    {
        filter.reset();
    }
    
    gainSmoother.reset();
    
    std::fill(inputLoudnessBuffer.begin(), inputLoudnessBuffer.end(), 0.0f);
    std::fill(outputLoudnessBuffer.begin(), outputLoudnessBuffer.end(), 0.0f);
    
    bufferIndex = 0;
    buffersInitialized = false;
    
    inputLoudness = 0.0f;
    outputLoudness = 0.0f;
    compensationGain = 0.0f;
}

void LoudnessCompensator::analyzeInput(const juce::dsp::AudioBlock<const float>& inputBlock)
{
    calculateLoudness(inputBlock, inputLoudness);
    
    // Store reference loudness when first analyzing input
    if (!buffersInitialized)
    {
        targetLoudness = inputLoudness;
    }
}

void LoudnessCompensator::analyzeOutput(const juce::dsp::AudioBlock<const float>& outputBlock)
{
    calculateLoudness(outputBlock, outputLoudness);
    
    // Calculate compensation gain
    if (outputLoudness > 0.001f && targetLoudness > 0.001f)
    {
        float loudnessDifference = juce::Decibels::gainToDecibels(targetLoudness / outputLoudness);
        
        // Limit compensation to reasonable range
        loudnessDifference = juce::jlimit(-12.0f, 12.0f, loudnessDifference);
        
        // Smooth the gain changes
        compensationGain = compensationGain * 0.95f + loudnessDifference * 0.05f;
    }
}

float LoudnessCompensator::getCompensationGain() const
{
    return compensationGain;
}

void LoudnessCompensator::applyCompensation(juce::dsp::AudioBlock<float>& block)
{
    if (std::abs(compensationGain) > 0.1f)
    {
        float linearGain = juce::Decibels::decibelsToGain(compensationGain);
        
        // Apply smoothed gain
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            auto* channelData = block.getChannelPointer(channel);
            for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
            {
                float smoothedGain = gainSmoother.processSample(static_cast<int>(channel), linearGain);
                channelData[sample] *= smoothedGain;
            }
        }
    }
}

void LoudnessCompensator::calculateLoudness(const juce::dsp::AudioBlock<const float>& block, float& loudnessTarget)
{
    float rms = calculateRMS(block);
    float peak = calculatePeak(block);
    float crestFactor = calculateCrestFactor(rms, peak);
    
    // Perceptual loudness weighting (simplified)
    // Accounts for frequency weighting and dynamic range
    float perceptualLoudness = rms * (1.0f + crestFactor * 0.3f);
    
    // Apply K-weighting approximation
    if (!kWeightingFilters.empty())
    {
        float weightedSum = 0.0f;
        size_t sampleCount = 0;
        
        for (size_t channel = 0; channel < block.getNumChannels() && channel < kWeightingFilters.size(); ++channel)
        {
            for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
            {
                float inputSample = block.getSample(static_cast<int>(channel), static_cast<int>(sample));
                float weighted = kWeightingFilters[channel].process(inputSample);
                weightedSum += weighted * weighted;
                sampleCount++;
            }
        }
        
        if (sampleCount > 0)
        {
            perceptualLoudness = std::sqrt(weightedSum / sampleCount);
        }
    }
    
    // Smooth the loudness measurement
    loudnessTarget = loudnessTarget * smoothingCoeff + perceptualLoudness * (1.0f - smoothingCoeff);
    
    // Store in circular buffer for gating
    if (buffersInitialized)
    {
        if (loudnessTarget == inputLoudness)
        {
            inputLoudnessBuffer[bufferIndex] = perceptualLoudness;
        }
        else
        {
            outputLoudnessBuffer[bufferIndex] = perceptualLoudness;
        }
        
        bufferIndex = (bufferIndex + 1) % inputLoudnessBuffer.size();
        if (bufferIndex == 0)
            buffersInitialized = true;
    }
}

float LoudnessCompensator::calculateRMS(const juce::dsp::AudioBlock<const float>& block)
{
    float sum = 0.0f;
    size_t sampleCount = 0;
    
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float sampleValue = block.getSample(static_cast<int>(channel), static_cast<int>(sample));
            sum += sampleValue * sampleValue;
            sampleCount++;
        }
    }
    
    return sampleCount > 0 ? std::sqrt(sum / sampleCount) : 0.0f;
}

float LoudnessCompensator::calculatePeak(const juce::dsp::AudioBlock<const float>& block)
{
    float peak = 0.0f;
    
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float sampleValue = std::abs(block.getSample(static_cast<int>(channel), static_cast<int>(sample)));
            peak = juce::jmax(peak, sampleValue);
        }
    }
    
    return peak;
}

float LoudnessCompensator::calculateCrestFactor(float rms, float peak)
{
    if (rms > 0.0001f)
        return peak / rms;
    return 1.0f;
}

// K-weighting filter implementation
void LoudnessCompensator::KWeightingFilter::prepare(const juce::dsp::ProcessSpec& spec)
{
    // Approximate K-weighting with high shelf + high pass
    // High shelf at ~4kHz (+4dB)
    auto highShelfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        spec.sampleRate, 4000.0f, 0.7f, juce::Decibels::decibelsToGain(4.0f));
    highShelf.coefficients = highShelfCoeffs;
    
    // High pass at ~38Hz
    auto highPassCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        spec.sampleRate, 38.0f, 0.5f);
    highPass.coefficients = highPassCoeffs;
    
    highShelf.prepare(spec);
    highPass.prepare(spec);
}

void LoudnessCompensator::KWeightingFilter::reset()
{
    highShelf.reset();
    highPass.reset();
}

float LoudnessCompensator::KWeightingFilter::process(float sample)
{
    float filtered = highPass.processSample(sample);
    filtered = highShelf.processSample(filtered);
    return filtered;
}
