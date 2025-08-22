#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "DSP/SaturationProcessor.h"
#include "DSP/AdaptiveEqualizer.h"
#include "DSP/LinearPhaseFilters.h"
#include "DSP/LoudnessCompensator.h"

class ProfessionalSaturationAudioProcessor : public juce::AudioProcessor
{
public:
    ProfessionalSaturationAudioProcessor();
    ~ProfessionalSaturationAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Accessors for UI components
    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    SaturationProcessor& getSaturationProcessor() { return saturationProcessor; }
    AdaptiveEqualizer& getAdaptiveEqualizer() { return adaptiveEqualizer; }
    LoudnessCompensator& getLoudnessCompensator() { return loudnessCompensator; }
    
    // Level monitoring
    float getInputRMS(int channel) const;
    float getInputPeak(int channel) const;
    float getOutputRMS(int channel) const;
    float getOutputPeak(int channel) const;

private:
    void updateParameters();
    void updateProcessingChain();
    
    juce::AudioProcessorValueTreeState valueTreeState;
    
    // DSP Chain
    juce::dsp::Gain<float> inputGain;
    LinearPhaseFilters preFilters;
    SaturationProcessor saturationProcessor;
    AdaptiveEqualizer adaptiveEqualizer;
    LinearPhaseFilters postFilters;
    juce::dsp::Gain<float> outputGain;
    LoudnessCompensator loudnessCompensator;
    
    // Parameter pointers for efficient access
    std::atomic<float>* inputGainParameter = nullptr;
    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* outputGainParameter = nullptr;
    std::atomic<float>* satTypeParameter = nullptr;
    std::atomic<float>* soloSaturationParameter = nullptr;
    
    // Filter parameters
    std::atomic<float>* lowCutFreqParameter = nullptr;
    std::atomic<float>* highCutFreqParameter = nullptr;
    std::atomic<float>* filterEnabledParameter = nullptr;
    
    // EQ parameters
    std::atomic<float>* eqEnabledParameter = nullptr;
    std::atomic<float>* eqTargetCurveParameter = nullptr;
    std::atomic<float>* eqAdaptionStrengthParameter = nullptr;
    std::atomic<float>* eqReactionSpeedParameter = nullptr;
    
    // Level monitoring
    std::array<float, 2> inputRMSLevels = { 0.0f, 0.0f };
    std::array<float, 2> inputPeakLevels = { 0.0f, 0.0f };
    std::array<float, 2> outputRMSLevels = { 0.0f, 0.0f };
    std::array<float, 2> outputPeakLevels = { 0.0f, 0.0f };
    
    // Processing buffers
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> wetBuffer;
    
    // Smoothing for parameter changes
    static constexpr float smoothingTimeSeconds = 0.05f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProfessionalSaturationAudioProcessor)
};
