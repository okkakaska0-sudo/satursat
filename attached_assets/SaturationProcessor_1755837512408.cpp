#include "SaturationProcessor.h"

SaturationProcessor::SaturationProcessor()
{
    rmsLevels.resize(2, 0.0f);
    peakLevels.resize(2, 0.0f);
}

void SaturationProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    dryWetMixer.prepare(spec);
    
    rmsLevels.resize(spec.numChannels, 0.0f);
    peakLevels.resize(spec.numChannels, 0.0f);
    
    reset();
}

void SaturationProcessor::reset()
{
    inputGain.reset();
    outputGain.reset();
    dryWetMixer.reset();
    
    std::fill(rmsLevels.begin(), rmsLevels.end(), 0.0f);
    std::fill(peakLevels.begin(), peakLevels.end(), 0.0f);
}

void SaturationProcessor::setDrive(float driveDb)
{
    drive = driveDb;
    inputGain.setGainDecibels(driveDb);
}

void SaturationProcessor::setMix(float mixPercent)
{
    mix = mixPercent / 100.0f;
    dryWetMixer.setWetMixProportion(mix);
}

void SaturationProcessor::setOutputGain(float gainDb)
{
    outputGain.setGainDecibels(gainDb);
}

void SaturationProcessor::setSaturationType(int type)
{
    saturationType = juce::jlimit(0, 3, type);
}

float SaturationProcessor::applySaturation(float input, int type) const
{
    switch (type)
    {
        case 0: return softClip(input);
        case 1: return hardClip(input);
        case 2: return tubeDistortion(input);
        case 3: return tapeDistortion(input);
        default: return input;
    }
}

float SaturationProcessor::softClip(float input) const
{
    const float threshold = 0.7f;
    const float ratio = 3.0f;
    
    float absInput = std::abs(input);
    if (absInput <= threshold)
        return input;
    
    float excess = absInput - threshold;
    float compressedExcess = excess / (1.0f + excess * ratio);
    float output = threshold + compressedExcess;
    
    return input >= 0.0f ? output : -output;
}

float SaturationProcessor::hardClip(float input) const
{
    return juce::jlimit(-0.95f, 0.95f, input);
}

float SaturationProcessor::tubeDistortion(float input) const
{
    float x = input * 2.0f;
    if (std::abs(x) < 1.0f)
        return x - (x * x * x) / 3.0f;
    else
        return x > 0.0f ? 2.0f / 3.0f : -2.0f / 3.0f;
}

float SaturationProcessor::tapeDistortion(float input) const
{
    float x = input * 1.5f;
    return std::tanh(x) * 0.8f;
}
