#pragma once

#include <JuceHeader.h>

class LoudnessCompensator
{
public:
    LoudnessCompensator();
    ~LoudnessCompensator() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void analyzeInput(const juce::dsp::AudioBlock<const float>& inputBlock);
    void analyzeOutput(const juce::dsp::AudioBlock<const float>& outputBlock);
    
    float getCompensationGain() const;
    void applyCompensation(juce::dsp::AudioBlock<float>& block);
    
    // Get current loudness measurements
    float getInputLoudness() const { return inputLoudness; }
    float getOutputLoudness() const { return outputLoudness; }

private:
    void calculateLoudness(const juce::dsp::AudioBlock<const float>& block, float& loudnessTarget);
    float calculateRMS(const juce::dsp::AudioBlock<const float>& block);
    float calculatePeak(const juce::dsp::AudioBlock<const float>& block);
    float calculateCrestFactor(float rms, float peak);
    
    // K-weighted filters for loudness measurement (approximation)
    struct KWeightingFilter
    {
        juce::dsp::IIR::Filter<float> highShelf;
        juce::dsp::IIR::Filter<float> highPass;
        void prepare(const juce::dsp::ProcessSpec& spec);
        void reset();
        float process(float sample);
    };
    
    std::vector<KWeightingFilter> kWeightingFilters;
    
    float sampleRate = 44100.0f;
    
    // Loudness measurements
    float inputLoudness = 0.0f;
    float outputLoudness = 0.0f;
    float targetLoudness = 0.0f;
    float compensationGain = 0.0f;
    
    // Smoothing and timing
    float smoothingCoeff = 0.999f;
    static constexpr float measurementTime = 0.4f; // 400ms gating
    
    // Circular buffers for loudness measurement
    std::vector<float> inputLoudnessBuffer;
    std::vector<float> outputLoudnessBuffer;
    size_t bufferIndex = 0;
    bool buffersInitialized = false;
    
    // Gain smoothing
    juce::dsp::BallisticsFilter<float> gainSmoother;
};
