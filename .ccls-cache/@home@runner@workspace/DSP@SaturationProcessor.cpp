#include "SaturationProcessor.h"

SaturationProcessor::SaturationProcessor() 
    : oversampler(2, oversamplingFactor, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false)
{
    rmsLevels.resize(2, 0.0f);
    peakLevels.resize(2, 0.0f);
    hysteresisState.resize(2, 0.0f);
    temperatureDrift.resize(2, 0.0f);
    biasState.resize(2, 0.0f);
}

void SaturationProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    dryWetMixer.prepare(spec);
    oversampler.initProcessing(spec.maximumBlockSize);
    
    rmsLevels.resize(spec.numChannels, 0.0f);
    peakLevels.resize(spec.numChannels, 0.0f);
    hysteresisState.resize(spec.numChannels, 0.0f);
    temperatureDrift.resize(spec.numChannels, 0.0f);
    biasState.resize(spec.numChannels, 0.0f);
    
    reset();
}

void SaturationProcessor::reset()
{
    dryWetMixer.reset();
    oversampler.reset();
    
    std::fill(rmsLevels.begin(), rmsLevels.end(), 0.0f);
    std::fill(peakLevels.begin(), peakLevels.end(), 0.0f);
    std::fill(hysteresisState.begin(), hysteresisState.end(), 0.0f);
    std::fill(temperatureDrift.begin(), temperatureDrift.end(), 0.0f);
    std::fill(biasState.begin(), biasState.end(), 0.0f);
}

void SaturationProcessor::setDrive(float driveDb)
{
    drive = driveDb;
}

void SaturationProcessor::setMix(float mixPercent)
{
    mix = mixPercent / 100.0f;
    dryWetMixer.setWetMixProportion(mix);
}

void SaturationProcessor::setSaturationType(int type)
{
    saturationType = juce::jlimit(0, 4, type);
}

void SaturationProcessor::setSoloMode(bool solo)
{
    soloMode = solo;
}

float SaturationProcessor::getRMSLevel(int channel) const
{
    if (channel < rmsLevels.size())
        return rmsLevels[static_cast<size_t>(channel)];
    return 0.0f;
}

float SaturationProcessor::getPeakLevel(int channel) const
{
    if (channel < peakLevels.size())
        return peakLevels[static_cast<size_t>(channel)];
    return 0.0f;
}

float SaturationProcessor::getSaturationCurveValue(float input) const
{
    float driven = input * juce::Decibels::decibelsToGain(drive);
    
    switch (saturationType)
    {
        case 0: return tubeWarmSaturation(driven);
        case 1: return tapeClassicSaturation(driven);
        case 2: return transistorModernSaturation(driven);
        case 3: return diodeHarshSaturation(driven);
        case 4: return vintageFuzzSaturation(driven);
        default: return driven;
    }
}

// Tube Warm - Multi-stage triode modeling
float SaturationProcessor::tubeWarmSaturation(float input) const
{
    // Three-stage triode cascade
    float stage1 = triodeStage(input, -0.7f, 20.0f);
    float stage2 = triodeStage(stage1, -1.2f, 15.0f);
    float stage3 = triodeStage(stage2, -0.9f, 10.0f);
    
    // Output transformer saturation
    float transformed = outputTransformer(stage3);
    
    return juce::jlimit(-0.95f, 0.95f, transformed);
}

float SaturationProcessor::triodeStage(float input, float bias, float gain) const
{
    // Asymmetric transfer function characteristic of triodes
    float biased = input + bias * 0.1f;
    float amplified = biased * gain;
    
    // Triode plate current equation approximation
    if (amplified < -2.0f)
        return 0.0f; // Cutoff region
    
    float exponential = std::exp(amplified * 0.5f);
    float output = (exponential - 1.0f) / (exponential + 1.0f);
    
    // Add even harmonics characteristic
    output += 0.05f * output * output;
    
    return output * 0.7f;
}

float SaturationProcessor::outputTransformer(float input) const
{
    // Transformer core saturation
    float normalized = input * 2.0f;
    float saturated = normalized / (1.0f + std::abs(normalized) * 0.3f);
    
    // Hysteresis effect
    static float lastOutput = 0.0f;
    float hysteresis = 0.05f * (saturated - lastOutput);
    lastOutput = saturated;
    
    return (saturated + hysteresis) * 0.8f;
}

// Tape Classic - Advanced magnetic tape modeling
float SaturationProcessor::tapeClassicSaturation(float input) const
{
    // Magnetic hysteresis with bias
    float biased = biasSimulation(input);
    float hysteretic = magneticHysteresis(biased, hysteresisState[0]);
    float processed = headGapModeling(hysteretic);
    
    return juce::jlimit(-0.9f, 0.9f, processed);
}

float SaturationProcessor::magneticHysteresis(float input, float& state) const
{
    // Simplified magnetic hysteresis model
    float coercivity = 0.3f;
    float saturation = 0.8f;
    
    if (std::abs(input) > coercivity)
    {
        float direction = input > 0.0f ? 1.0f : -1.0f;
        state = direction * saturation * std::tanh(std::abs(input) / coercivity);
    }
    
    // Magnetic lag
    float output = 0.7f * input + 0.3f * state;
    return output;
}

float SaturationProcessor::biasSimulation(float input) const
{
    // AC bias adds high-frequency content for linearization
    static float biasPhase = 0.0f;
    biasPhase += 0.1f; // High frequency bias
    
    float bias = 0.05f * std::sin(biasPhase);
    return input + bias;
}

float SaturationProcessor::headGapModeling(float input) const
{
    // Gap loss affects high frequencies
    static float lastInput = 0.0f;
    float derivative = input - lastInput;
    lastInput = input;
    
    // Frequency-dependent loss
    float loss = 0.1f * derivative;
    return input - loss;
}

// Transistor Modern - Class-AB modeling
float SaturationProcessor::transistorModernSaturation(float input) const
{
    float crossover = classABCrossover(input);
    float feedback = negativeFeedback(crossover, 0.05f);
    
    return juce::jlimit(-0.95f, 0.95f, feedback);
}

float SaturationProcessor::classABCrossover(float input) const
{
    // Class-AB crossover distortion
    float threshold = 0.02f;
    
    if (std::abs(input) < threshold)
    {
        // Crossover region - both transistors partially off
        float distortion = 0.3f * input * input * input;
        return input + distortion;
    }
    
    // Linear region
    return input * 0.98f;
}

float SaturationProcessor::negativeFeedback(float input, float feedback) const
{
    // Negative feedback reduces distortion and extends bandwidth
    static float delayedOutput = 0.0f;
    
    float corrected = input - feedback * delayedOutput;
    delayedOutput = corrected;
    
    return corrected;
}

// Diode Harsh - Shockley equation modeling
float SaturationProcessor::diodeHarshSaturation(float input) const
{
    float clipped = shockleyDiode(input, true); // Silicon
    float opamp = opAmpSaturation(clipped);
    
    return juce::jlimit(-0.98f, 0.98f, opamp);
}

float SaturationProcessor::shockleyDiode(float input, bool silicon) const
{
    // Shockley diode equation: I = Is * (exp(V/nVt) - 1)
    float thermalVoltage = silicon ? 0.026f : 0.033f; // Vt at room temperature
    float ideality = silicon ? 1.0f : 1.3f; // n factor
    
    float normalized = input / thermalVoltage / ideality;
    
    if (normalized > 10.0f) // Prevent numerical overflow
        normalized = 10.0f;
    
    float exponential = std::exp(normalized);
    float current = (exponential - 1.0f) / exponential;
    
    // Asymmetric clipping for silicon vs germanium
    if (input > 0.0f)
        return current * (silicon ? 0.7f : 0.3f);
    else
        return -current * (silicon ? 0.7f : 0.2f);
}

float SaturationProcessor::opAmpSaturation(float input) const
{
    // Op-amp rail saturation
    float supply = 12.0f; // ±12V supply
    float normalizedInput = input * supply;
    
    // Smooth saturation near rails
    float saturated = std::tanh(normalizedInput / supply) * 0.9f;
    
    return saturated;
}

// Vintage Fuzz - Germanium transistor modeling
float SaturationProcessor::vintageFuzzSaturation(float input) const
{
    // Temperature-dependent germanium behavior
    float temperature = 25.0f + temperatureDrift[0] * 10.0f; // Room temp + drift
    float processed = germaniumTransistor(input, temperature);
    float fuzzed = intermodulationDistortion(processed);
    
    return juce::jlimit(-0.85f, 0.85f, fuzzed);
}

float SaturationProcessor::germaniumTransistor(float input, float temperature) const
{
    // Germanium transistor characteristics with temperature dependency
    float thermalVoltage = 0.026f * (temperature + 273.15f) / 298.15f;
    float leakageCurrent = 0.01f * std::exp((temperature - 25.0f) / 10.0f);
    
    // Base-collector leakage affects biasing
    float biasShift = leakageCurrent * 0.1f;
    float biased = input + biasShift;
    
    // Exponential collector current
    float normalized = biased / thermalVoltage;
    if (normalized > 10.0f) normalized = 10.0f;
    
    float current = std::tanh(normalized);
    
    // Temperature instability
    temperatureDrift[0] += (input * input - temperatureDrift[0]) * 0.001f;
    
    return current * 0.8f;
}

float SaturationProcessor::intermodulationDistortion(float input) const
{
    // Intermodulation between cascaded stages
    static float stage1Memory = 0.0f;
    static float stage2Memory = 0.0f;
    
    // First stage
    float stage1 = std::tanh(input * 3.0f) * 0.7f;
    
    // Intermodulation with previous sample
    float intermod = 0.05f * stage1 * stage1Memory;
    stage1Memory = stage1;
    
    // Second stage with memory
    float stage2 = std::tanh((stage1 + intermod) * 2.0f) * 0.8f;
    float finalIntermod = 0.03f * stage2 * stage2Memory;
    stage2Memory = stage2;
    
    return stage2 + finalIntermod;
}
