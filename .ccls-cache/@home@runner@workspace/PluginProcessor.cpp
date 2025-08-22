#include "PluginProcessor.h"
#include "PluginEditor.h"

ProfessionalSaturationAudioProcessor::ProfessionalSaturationAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
      valueTreeState(*this, nullptr, "Parameters", ParameterLayout::createParameterLayout())
{
    // Get parameter pointers for efficient access
    inputGainParameter = valueTreeState.getRawParameterValue(ParameterIDs::inputGain);
    driveParameter = valueTreeState.getRawParameterValue(ParameterIDs::drive);
    mixParameter = valueTreeState.getRawParameterValue(ParameterIDs::mix);
    outputGainParameter = valueTreeState.getRawParameterValue(ParameterIDs::outputGain);
    satTypeParameter = valueTreeState.getRawParameterValue(ParameterIDs::satType);
    soloSaturationParameter = valueTreeState.getRawParameterValue(ParameterIDs::soloSaturation);
    
    lowCutFreqParameter = valueTreeState.getRawParameterValue(ParameterIDs::lowCutFreq);
    highCutFreqParameter = valueTreeState.getRawParameterValue(ParameterIDs::highCutFreq);
    filterEnabledParameter = valueTreeState.getRawParameterValue(ParameterIDs::filterEnabled);
    
    eqEnabledParameter = valueTreeState.getRawParameterValue(ParameterIDs::eqEnabled);
    eqTargetCurveParameter = valueTreeState.getRawParameterValue(ParameterIDs::eqTargetCurve);
    eqAdaptionStrengthParameter = valueTreeState.getRawParameterValue(ParameterIDs::eqAdaptionStrength);
    eqReactionSpeedParameter = valueTreeState.getRawParameterValue(ParameterIDs::eqReactionSpeed);
}

ProfessionalSaturationAudioProcessor::~ProfessionalSaturationAudioProcessor()
{
}

const juce::String ProfessionalSaturationAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ProfessionalSaturationAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ProfessionalSaturationAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ProfessionalSaturationAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ProfessionalSaturationAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ProfessionalSaturationAudioProcessor::getNumPrograms()
{
    return 1;
}

int ProfessionalSaturationAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ProfessionalSaturationAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String ProfessionalSaturationAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void ProfessionalSaturationAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void ProfessionalSaturationAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = static_cast<uint32>(samplesPerBlock);
    spec.numChannels = static_cast<uint32>(getTotalNumOutputChannels());
    spec.sampleRate = sampleRate;
    
    // Prepare all DSP components
    inputGain.prepare(spec);
    preFilters.prepare(spec);
    saturationProcessor.prepare(spec);
    adaptiveEqualizer.prepare(spec);
    postFilters.prepare(spec);
    outputGain.prepare(spec);
    loudnessCompensator.prepare(spec);
    
    // Initialize processing buffers
    dryBuffer.setSize(static_cast<int>(spec.numChannels), samplesPerBlock);
    wetBuffer.setSize(static_cast<int>(spec.numChannels), samplesPerBlock);
    
    updateParameters();
}

void ProfessionalSaturationAudioProcessor::releaseResources()
{
    inputGain.reset();
    preFilters.reset();
    saturationProcessor.reset();
    adaptiveEqualizer.reset();
    postFilters.reset();
    outputGain.reset();
    loudnessCompensator.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProfessionalSaturationAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ProfessionalSaturationAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Update parameters
    updateParameters();
    
    // Create audio block for processing
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Store dry signal for loudness compensation analysis
    dryBuffer.makeCopyOf(buffer);
    juce::dsp::AudioBlock<const float> dryBlock(dryBuffer);
    loudnessCompensator.analyzeInput(dryBlock);
    
    // Measure input levels
    for (int channel = 0; channel < totalNumInputChannels && channel < 2; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        float rms = 0.0f;
        float peak = 0.0f;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float sampleValue = std::abs(channelData[sample]);
            rms += channelData[sample] * channelData[sample];
            peak = juce::jmax(peak, sampleValue);
        }
        
        rms = std::sqrt(rms / buffer.getNumSamples());
        inputRMSLevels[channel] = inputRMSLevels[channel] * 0.9f + rms * 0.1f;
        inputPeakLevels[channel] = inputPeakLevels[channel] * 0.9f + peak * 0.1f;
    }
    
    // Processing chain
    // 1. Input gain
    inputGain.process(context);
    
    // 2. Pre-filtering (anti-aliasing)
    preFilters.process(context);
    
    // 3. Saturation processing
    saturationProcessor.process(context);
    
    // 4. Adaptive EQ (post-saturation)
    adaptiveEqualizer.process(context);
    
    // 5. Post-filtering
    postFilters.process(context);
    
    // 6. Output gain
    outputGain.process(context);
    
    // Analyze output for loudness compensation
    juce::dsp::AudioBlock<const float> outputBlock(buffer);
    loudnessCompensator.analyzeOutput(outputBlock);
    
    // Apply loudness compensation
    loudnessCompensator.applyCompensation(block);
    
    // Measure output levels
    for (int channel = 0; channel < totalNumInputChannels && channel < 2; ++channel)
    {
        auto* channelData = buffer.getReadPointer(channel);
        float rms = 0.0f;
        float peak = 0.0f;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float sampleValue = std::abs(channelData[sample]);
            rms += channelData[sample] * channelData[sample];
            peak = juce::jmax(peak, sampleValue);
        }
        
        rms = std::sqrt(rms / buffer.getNumSamples());
        outputRMSLevels[channel] = outputRMSLevels[channel] * 0.9f + rms * 0.1f;
        outputPeakLevels[channel] = outputPeakLevels[channel] * 0.9f + peak * 0.1f;
    }
}

bool ProfessionalSaturationAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ProfessionalSaturationAudioProcessor::createEditor()
{
    return new ProfessionalSaturationAudioProcessorEditor(*this);
}

void ProfessionalSaturationAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ProfessionalSaturationAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(valueTreeState.state.getType()))
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void ProfessionalSaturationAudioProcessor::updateParameters()
{
    // Update input/output gains
    if (inputGainParameter)
        inputGain.setGainDecibels(inputGainParameter->load());
    
    if (outputGainParameter)
        outputGain.setGainDecibels(outputGainParameter->load());
    
    // Update saturation processor
    if (driveParameter)
        saturationProcessor.setDrive(driveParameter->load());
    
    if (mixParameter)
        saturationProcessor.setMix(mixParameter->load());
    
    if (satTypeParameter)
        saturationProcessor.setSaturationType(static_cast<int>(satTypeParameter->load()));
    
    if (soloSaturationParameter)
        saturationProcessor.setSoloMode(soloSaturationParameter->load() > 0.5f);
    
    // Update linear phase filters
    if (filterEnabledParameter)
        preFilters.setEnabled(filterEnabledParameter->load() > 0.5f);
    
    if (lowCutFreqParameter)
        preFilters.setLowCutFrequency(lowCutFreqParameter->load());
    
    if (highCutFreqParameter)
        postFilters.setHighCutFrequency(highCutFreqParameter->load());
    
    // Update adaptive equalizer
    if (eqEnabledParameter)
        adaptiveEqualizer.setEnabled(eqEnabledParameter->load() > 0.5f);
    
    if (eqTargetCurveParameter)
        adaptiveEqualizer.setTargetCurve(static_cast<int>(eqTargetCurveParameter->load()));
    
    if (eqAdaptionStrengthParameter)
        adaptiveEqualizer.setAdaptionStrength(eqAdaptionStrengthParameter->load());
    
    if (eqReactionSpeedParameter)
        adaptiveEqualizer.setReactionSpeed(eqReactionSpeedParameter->load());
}

float ProfessionalSaturationAudioProcessor::getInputRMS(int channel) const
{
    if (channel >= 0 && channel < 2)
        return inputRMSLevels[channel];
    return 0.0f;
}

float ProfessionalSaturationAudioProcessor::getInputPeak(int channel) const
{
    if (channel >= 0 && channel < 2)
        return inputPeakLevels[channel];
    return 0.0f;
}

float ProfessionalSaturationAudioProcessor::getOutputRMS(int channel) const
{
    if (channel >= 0 && channel < 2)
        return outputRMSLevels[channel];
    return 0.0f;
}

float ProfessionalSaturationAudioProcessor::getOutputPeak(int channel) const
{
    if (channel >= 0 && channel < 2)
        return outputPeakLevels[channel];
    return 0.0f;
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProfessionalSaturationAudioProcessor();
}
