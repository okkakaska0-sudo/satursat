#pragma once

#include <JuceHeader.h>

class LinearPhaseFilters
{
public:
    LinearPhaseFilters();
    ~LinearPhaseFilters() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setEnabled(bool enabled);
    void setLowCutFrequency(float frequency);
    void setHighCutFrequency(float frequency);
    
    template<typename ProcessContext>
    void process(const ProcessContext& context);

private:
    void updateLowCutFilter();
    void updateHighCutFilter();
    
    // Linear phase FIR filters
    juce::dsp::FIR::Filter<float> lowCutFilter;
    juce::dsp::FIR::Filter<float> highCutFilter;
    
    bool enabled = true;
    float lowCutFreq = 20.0f;
    float highCutFreq = 20000.0f;
    float sampleRate = 44100.0f;
    
    static constexpr size_t filterOrder = 511; // High order for linear phase
    static constexpr size_t numChannels = 2;
    
    std::array<juce::dsp::FIR::Filter<float>, numChannels> lowCutFilters;
    std::array<juce::dsp::FIR::Filter<float>, numChannels> highCutFilters;
};

template<typename ProcessContext>
void LinearPhaseFilters::process(const ProcessContext& context)
{
    if (!enabled)
        return;
        
    auto& outputBlock = context.getOutputBlock();
    const auto numChannelsToProcess = outputBlock.getNumChannels();
    
    // Process each channel
    for (size_t channel = 0; channel < numChannelsToProcess && channel < numChannels; ++channel)
    {
        auto channelBlock = outputBlock.getSingleChannelBlock(channel);
        juce::dsp::ProcessContextReplacing<float> channelContext(channelBlock);
        
        // Apply low cut filter
        if (lowCutFreq > 20.0f)
            lowCutFilters[channel].process(channelContext);
        
        // Apply high cut filter  
        if (highCutFreq < 20000.0f)
            highCutFilters[channel].process(channelContext);
    }
}
