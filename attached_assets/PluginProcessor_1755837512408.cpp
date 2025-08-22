#include "PluginProcessor.h"
#include "PluginEditor.h"

SaturationPluginAudioProcessor::SaturationPluginAudioProcessor()
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
    driveParameter = valueTreeState.getRawParameterValue(ParameterIDs::drive);
    mixParameter = valueTreeState.getRawParameterValue(ParameterIDs::mix);
    outputGainParameter = valueTreeState.getRawParameterValue(ParameterIDs::outputGain);
    satTypeParameter = valueTreeState.getRawParameterValue(ParameterIDs::satType);
}

SaturationPluginAudioProcessor::~SaturationPluginAudioProcessor()
{
}

const juce::String SaturationPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SaturationPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SaturationPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SaturationPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SaturationPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SaturationPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int SaturationPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SaturationPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String SaturationPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SaturationPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void SaturationPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = static_cast<uint32>(samplesPerBlock);
    spec.numChannels = static_cast<uint32>(getTotalNumOutputChannels());
    spec.sampleRate = sampleRate;
    
    saturationProcessor.prepare(spec);
}

void SaturationPluginAudioProcessor::releaseResources()
{
    saturationProcessor.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SaturationPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
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

void SaturationPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update parameters (with null checks for safety)
    if (driveParameter)
        saturationProcessor.setDrive(driveParameter->load());
    if (mixParameter)
        saturationProcessor.setMix(mixParameter->load());
    if (outputGainParameter)
        saturationProcessor.setOutputGain(outputGainParameter->load());
    if (satTypeParameter)
        saturationProcessor.setSaturationType(static_cast<int>(satTypeParameter->load()));

    // Process audio
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    saturationProcessor.process(context);
}

bool SaturationPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SaturationPluginAudioProcessor::createEditor()
{
    return new SaturationPluginAudioProcessorEditor (*this);
}

void SaturationPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SaturationPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (valueTreeState.state.getType()))
            valueTreeState.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SaturationPluginAudioProcessor();
}
