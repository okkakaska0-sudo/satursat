#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"

SaturationPluginAudioProcessorEditor::SaturationPluginAudioProcessorEditor(SaturationPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      driveKnob("DRIVE", audioProcessor.getValueTreeState(), ParameterIDs::drive),
      mixKnob("MIX", audioProcessor.getValueTreeState(), ParameterIDs::mix),
      outputKnob("OUTPUT", audioProcessor.getValueTreeState(), ParameterIDs::outputGain),
      vuMeter(audioProcessor.getSaturationProcessor()),
      saturationViz(audioProcessor.getSaturationProcessor(), audioProcessor.getValueTreeState())
{
    setLookAndFeel(&customLookAndFeel);
    
    // Setup title
    titleLabel.setText("SATURATION", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColor);
    addAndMakeVisible(titleLabel);
    
    // Setup knobs
    addAndMakeVisible(driveKnob);
    addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputKnob);
    
    // Setup saturation type combo box
    saturationTypeCombo.addItem("Soft Clip", 1);
    saturationTypeCombo.addItem("Hard Clip", 2);
    saturationTypeCombo.addItem("Tube", 3);
    saturationTypeCombo.addItem("Tape", 4);
    saturationTypeCombo.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(saturationTypeCombo);
    
    saturationTypeLabel.setText("TYPE", juce::dontSendNotification);
    saturationTypeLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    saturationTypeLabel.setJustificationType(juce::Justification::centred);
    saturationTypeLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColor);
    addAndMakeVisible(saturationTypeLabel);
    
    saturationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), ParameterIDs::satType, saturationTypeCombo);
    
    // Setup VU meter and visualization
    addAndMakeVisible(vuMeter);
    addAndMakeVisible(saturationViz);
    
    setSize(600, 400);
}

SaturationPluginAudioProcessorEditor::~SaturationPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    saturationTypeCombo.setLookAndFeel(nullptr);
}

void SaturationPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(CustomLookAndFeel::backgroundColor);
    
    // Draw subtle background pattern
    g.setColour(CustomLookAndFeel::primaryColor.withAlpha(0.1f));
    for (int i = 0; i < getWidth(); i += 20)
    {
        g.drawVerticalLine(i, 0, getHeight());
    }
    
    // Draw main border
    g.setColour(CustomLookAndFeel::secondaryColor);
    g.drawRect(getLocalBounds(), 2);
}

void SaturationPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Title
    titleLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(10);
    
    // Top row: Saturation visualization
    auto topSection = bounds.removeFromTop(150);
    saturationViz.setBounds(topSection.removeFromLeft(400));
    topSection.removeFromLeft(10);
    vuMeter.setBounds(topSection);
    
    bounds.removeFromTop(20);
    
    // Bottom row: Controls
    auto controlsSection = bounds.removeFromTop(120);
    
    // Saturation type
    auto typeSection = controlsSection.removeFromLeft(120);
    saturationTypeLabel.setBounds(typeSection.removeFromTop(20));
    typeSection.removeFromTop(5);
    saturationTypeCombo.setBounds(typeSection.removeFromTop(25));
    
    controlsSection.removeFromLeft(20);
    
    // Knobs
    auto knobWidth = (controlsSection.getWidth() - 40) / 3;
    driveKnob.setBounds(controlsSection.removeFromLeft(knobWidth));
    controlsSection.removeFromLeft(20);
    mixKnob.setBounds(controlsSection.removeFromLeft(knobWidth));
    controlsSection.removeFromLeft(20);
    outputKnob.setBounds(controlsSection);
}
