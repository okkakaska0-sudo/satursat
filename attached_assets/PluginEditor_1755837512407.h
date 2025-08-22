#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/KnobComponent.h"
#include "Components/VUMeter.h"
#include "Components/SaturationVisualization.h"
#include "LookAndFeel/CustomLookAndFeel.h"

class SaturationPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SaturationPluginAudioProcessorEditor(SaturationPluginAudioProcessor&);
    ~SaturationPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SaturationPluginAudioProcessor& audioProcessor;
    
    CustomLookAndFeel customLookAndFeel;
    
    KnobComponent driveKnob;
    KnobComponent mixKnob;
    KnobComponent outputKnob;
    
    juce::ComboBox saturationTypeCombo;
    juce::Label saturationTypeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> saturationTypeAttachment;
    
    VUMeter vuMeter;
    SaturationVisualization saturationViz;
    
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaturationPluginAudioProcessorEditor)
};
