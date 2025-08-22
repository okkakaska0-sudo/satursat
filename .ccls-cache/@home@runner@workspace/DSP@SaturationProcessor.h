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
    void setSaturationType(int type);
    void setSoloMode(bool solo);
    
    template<typename ProcessContext>
    void process(const ProcessContext& context);
    
    float getRMSLevel(int channel) const;
    float getPeakLevel(int channel) const;
    float getSaturationCurveValue(float input) const;

private:
    // Tube Warm - Multi-stage triode modeling
    float tubeWarmSaturation(float input) const;
    float triodeStage(float input, float bias, float gain) const;
    float outputTransformer(float input) const;
    
    // Tape Classic - Advanced magnetic tape modeling
    float tapeClassicSaturation(float input) const;
    float magneticHysteresis(float input, float& state) const;
    float biasSimulation(float input) const;
    float headGapModeling(float input) const;
    
    // Transistor Modern - Class-AB modeling
    float transistorModernSaturation(float input) const;
    float classABCrossover(float input) const;
    float negativeFeedback(float input, float feedback) const;
    
    // Diode Harsh - Shockley equation modeling
    float diodeHarshSaturation(float input) const;
    float shockleyDiode(float input, bool silicon = true) const;
    float opAmpSaturation(float input) const;
    
    // Vintage Fuzz - Germanium transistor modeling
    float vintageFuzzSaturation(float input) const;
    float germaniumTransistor(float input, float temperature) const;
    float intermodulationDistortion(float input) const;
    
    juce::dsp::DryWetMixer<float> dryWetMixer;
    
    float drive = 0.0f;
    float mix = 1.0f;
    int saturationType = 0;
    bool soloMode = false;
    
    std::vector<float> rmsLevels;
    std::vector<float> peakLevels;
    
    // Saturation algorithm states
    mutable std::vector<float> hysteresisState;
    mutable std::vector<float> temperatureDrift;
    mutable std::vector<float> biasState;
    
    static constexpr float smoothingTime = 0.02f;
    static constexpr int oversamplingFactor = 4;
    
    juce::dsp::Oversampling<float> oversampler;
};

template<typename ProcessContext>
void SaturationProcessor::process(const ProcessContext& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    
    jassert(inputBlock.getNumChannels() == outputBlock.getNumChannels());
    jassert(inputBlock.getNumSamples() == outputBlock.getNumSamples());
    
    juce::ignoreUnused(inputBlock.getNumChannels(), inputBlock.getNumSamples());
    
    // Store dry signal for mix (unless in solo mode)
    if (!soloMode)
        dryWetMixer.pushDrySamples(inputBlock);
    
    // Oversample for high-quality saturation
    auto oversampledBlock = oversampler.processSamplesUp(inputBlock);
    
    // Apply saturation with oversampling
    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);
        const auto oversampledSamples = oversampledBlock.getNumSamples();
        
        float rms = 0.0f;
        float peak = 0.0f;
        
        for (size_t sample = 0; sample < oversampledSamples; ++sample)
        {
            float input = channelData[sample];
            float driven = input * juce::Decibels::decibelsToGain(drive);
            float saturated = 0.0f;
            
            switch (saturationType)
            {
                case 0: saturated = tubeWarmSaturation(driven); break;
                case 1: saturated = tapeClassicSaturation(driven); break;
                case 2: saturated = transistorModernSaturation(driven); break;
                case 3: saturated = diodeHarshSaturation(driven); break;
                case 4: saturated = vintageFuzzSaturation(driven); break;
                default: saturated = driven; break;
            }
            
            channelData[sample] = saturated;
            
            // Calculate levels at original sample rate
            if (sample % oversamplingFactor == 0)
            {
                rms += input * input;
                peak = juce::jmax(peak, std::abs(input));
            }
        }
        
        // Update level meters
        rms = std::sqrt(rms / (oversampledSamples / oversamplingFactor));
        if (channel < rmsLevels.size())
        {
            rmsLevels[channel] = rmsLevels[channel] * (1.0f - smoothingTime) + rms * smoothingTime;
            peakLevels[channel] = peakLevels[channel] * (1.0f - smoothingTime) + peak * smoothingTime;
        }
    }
    
    // Downsample back to original rate
    oversampler.processSamplesDown(outputBlock);
    
    // Apply dry/wet mix (unless in solo mode)
    if (!soloMode)
        dryWetMixer.mixWetSamples(outputBlock);
}
