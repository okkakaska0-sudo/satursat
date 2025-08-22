#pragma once

#include <JuceHeader.h>
#include "FFTProcessor.h"

class AdaptiveEqualizer
{
public:
    enum TargetCurve
    {
        Flat = 0,
        Musical,
        Presence,
        Warm,
        Bright
    };

    AdaptiveEqualizer();
    ~AdaptiveEqualizer() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setEnabled(bool enabled);
    void setTargetCurve(int curve);
    void setAdaptionStrength(float strength);
    void setReactionSpeed(float speedMs);
    
    template<typename ProcessContext>
    void process(const ProcessContext& context);
    
    std::vector<float> getFrequencyResponse() const;
    std::vector<float> getTargetCurve() const;
    std::vector<float> getCurrentSpectrum() const;

private:
    struct Band
    {
        float frequency;
        float gain;
        float targetGain;
        float smoothedGain;
        juce::dsp::IIR::Filter<float> filter;
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
    };
    
    void updateTargetCurve();
    void analyzeSpectrum(const juce::dsp::AudioBlock<const float>& block);
    void updateBandGains();
    void updateFilterCoefficients();
    
    // 8 frequency bands (Hz)
    static constexpr std::array<float, 8> bandFrequencies = {
        80.0f, 200.0f, 500.0f, 1200.0f, 3000.0f, 6000.0f, 12000.0f, 16000.0f
    };
    
    std::array<Band, 8> bands;
    FFTProcessor fftProcessor;
    
    bool enabled = false;
    int targetCurveType = Flat;
    float adaptionStrength = 0.5f;
    float reactionSpeed = 100.0f; // ms
    
    float sampleRate = 44100.0f;
    float smoothingCoeff = 0.95f;
    
    std::vector<float> targetCurveValues;
    std::vector<float> currentSpectrum;
    std::vector<float> smoothedSpectrum;
    
    // Processing chains for each channel
    std::vector<juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>
    >> processingChains;
    
    juce::dsp::ProcessSpec currentSpec;
};

template<typename ProcessContext>
void AdaptiveEqualizer::process(const ProcessContext& context)
{
    if (!enabled)
        return;
        
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    
    // Analyze spectrum for adaptation
    analyzeSpectrum(inputBlock);
    
    // Update band gains based on analysis
    updateBandGains();
    
    // Update filter coefficients
    updateFilterCoefficients();
    
    // Process each channel through the filter chain
    for (size_t channel = 0; channel < inputBlock.getNumChannels(); ++channel)
    {
        if (channel < processingChains.size())
        {
            auto channelBlock = outputBlock.getSingleChannelBlock(channel);
            juce::dsp::ProcessContextReplacing<float> channelContext(channelBlock);
            processingChains[channel].process(channelContext);
        }
    }
}
