#pragma once

#include <JuceHeader.h>

class SaturationProcessor
{
public:
    SaturationProcessor();
    ~SaturationProcessor() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setDrive(float driveDb);
    void setMix(float mixPercent);
    void setOutputGain(float gainDb);
    void setSaturationType(int type);
    
    template<typename ProcessContext>
    void process(const ProcessContext& context);
    
    float getRMSLevel(int channel) const { return rmsLevels[static_cast<size_t>(channel)]; }
    float getPeakLevel(int channel) const { return peakLevels[static_cast<size_t>(channel)]; }

private:
    float applySaturation(float input, int type) const;
    float softClip(float input) const;
    float hardClip(float input) const;
    float tubeDistortion(float input) const;
    float tapeDistortion(float input) const;
    
    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    
    float drive = 0.0f;
    float mix = 1.0f;
    int saturationType = 0;
    
    std::vector<float> rmsLevels;
    std::vector<float> peakLevels;
    
    static constexpr float smoothingTime = 0.05f;
};

template<typename ProcessContext>
void SaturationProcessor::process(const ProcessContext& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    
    jassert(inputBlock.getNumChannels() == outputBlock.getNumChannels());
    jassert(inputBlock.getNumSamples() == outputBlock.getNumSamples());
    
    const auto numChannels = inputBlock.getNumChannels();
    const auto numSamples = inputBlock.getNumSamples();
    
    // Store dry signal for mix
    dryWetMixer.pushDrySamples(inputBlock);
    
    // Apply input gain
    inputGain.process(context);
    
    // Apply saturation
    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = outputBlock.getChannelPointer(channel);
        float rms = 0.0f;
        float peak = 0.0f;
        
        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            float output = applySaturation(input, saturationType);
            channelData[sample] = output;
            
            // Calculate RMS and peak levels
            rms += input * input;
            peak = juce::jmax(peak, std::abs(input));
        }
        
        // Update level meters
        rms = std::sqrt(rms / numSamples);
        if (channel < rmsLevels.size())
        {
            rmsLevels[channel] = rmsLevels[channel] * (1.0f - smoothingTime) + rms * smoothingTime;
            peakLevels[channel] = peakLevels[channel] * (1.0f - smoothingTime) + peak * smoothingTime;
        }
    }
    
    // Apply output gain
    outputGain.process(context);
    
    // Apply dry/wet mix
    dryWetMixer.mixWetSamples(outputBlock);
}
