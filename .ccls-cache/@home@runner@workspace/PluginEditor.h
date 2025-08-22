#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/KnobComponent.h"
#include "Components/VUMeter.h"
#include "Components/SaturationVisualization.h"
#include "Components/EqualizerDisplay.h"
#include "LookAndFeel/CustomLookAndFeel.h"

class ProfessionalSaturationAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    ProfessionalSaturationAudioProcessorEditor(ProfessionalSaturationAudioProcessor&);
    ~ProfessionalSaturationAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    
    void setupComponents();
    void setupLayout();
    
    struct ComponentBounds
    {
        juce::Rectangle<int> titleArea;
        juce::Rectangle<int> inputVUArea;
        juce::Rectangle<int> outputVUArea;
        juce::Rectangle<int> saturationVizArea;
        juce::Rectangle<int> eqDisplayArea;
        juce::Rectangle<int> controlsArea;
        juce::Rectangle<int> filtersArea;
        juce::Rectangle<int> eqControlsArea;
        juce::Rectangle<int> saturationControlsArea;
    };
    
    ComponentBounds calculateLayout(juce::Rectangle<int> bounds);
    juce::Rectangle<int> getScaledBounds(int baseWidthParam, int baseHeightParam, float scaleFactor = 1.0f);
    
    ProfessionalSaturationAudioProcessor& audioProcessor;
    
    CustomLookAndFeel customLookAndFeel;
    
    // Control components
    std::unique_ptr<KnobComponent> inputGainKnob;
    std::unique_ptr<KnobComponent> driveKnob;
    std::unique_ptr<KnobComponent> mixKnob;
    std::unique_ptr<KnobComponent> outputGainKnob;
    
    // Filter controls
    std::unique_ptr<KnobComponent> lowCutKnob;
    std::unique_ptr<KnobComponent> highCutKnob;
    juce::ToggleButton filterEnableButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> filterEnableAttachment;
    
    // EQ controls
    std::unique_ptr<KnobComponent> eqStrengthKnob;
    std::unique_ptr<KnobComponent> eqSpeedKnob;
    juce::ComboBox eqTargetCombo;
    juce::ToggleButton eqEnableButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> eqTargetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqEnableAttachment;
    
    // Saturation controls
    juce::ComboBox saturationTypeCombo;
    juce::ToggleButton soloButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> saturationTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;
    
    // Visualization components
    std::unique_ptr<VUMeter> inputVUMeter;
    std::unique_ptr<VUMeter> outputVUMeter;
    std::unique_ptr<SaturationVisualization> saturationViz;
    std::unique_ptr<EqualizerDisplay> eqDisplay;
    
    // Labels and groupings
    juce::Label titleLabel;
    juce::Label saturationSectionLabel;
    juce::Label filterSectionLabel;
    juce::Label eqSectionLabel;
    juce::Label inputOutputSectionLabel;
    
    // Responsive design parameters
    float currentScaleFactor = 1.0f;
    static constexpr int baseWidth = 800;
    static constexpr int baseHeight = 600;
    static constexpr float minScaleFactor = 0.7f;
    static constexpr float maxScaleFactor = 2.0f;
    
    // Component bounds for responsive layout

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProfessionalSaturationAudioProcessorEditor)
};
