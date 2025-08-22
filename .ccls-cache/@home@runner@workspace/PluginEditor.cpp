#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "Parameters.h"

ProfessionalSaturationAudioProcessorEditor::ProfessionalSaturationAudioProcessorEditor(ProfessionalSaturationAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
    
    setupComponents();
    setSize(baseWidth, baseHeight);
    setResizable(true, true);
    setResizeLimits(static_cast<int>(baseWidth * minScaleFactor), 
                   static_cast<int>(baseHeight * minScaleFactor),
                   static_cast<int>(baseWidth * maxScaleFactor), 
                   static_cast<int>(baseHeight * maxScaleFactor));
    
    startTimer(50); // 20 FPS for UI updates
}

ProfessionalSaturationAudioProcessorEditor::~ProfessionalSaturationAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
    
    saturationTypeCombo.setLookAndFeel(nullptr);
    eqTargetCombo.setLookAndFeel(nullptr);
    filterEnableButton.setLookAndFeel(nullptr);
    eqEnableButton.setLookAndFeel(nullptr);
    soloButton.setLookAndFeel(nullptr);
}

void ProfessionalSaturationAudioProcessorEditor::setupComponents()
{
    // Title
    titleLabel.setText("PROFESSIONAL SATURATION", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f).boldened());
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColor);
    addAndMakeVisible(titleLabel);
    
    // Section labels
    saturationSectionLabel.setText("SATURATION", juce::dontSendNotification);
    saturationSectionLabel.setFont(juce::Font(12.0f).boldened());
    saturationSectionLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::accentColor);
    addAndMakeVisible(saturationSectionLabel);
    
    filterSectionLabel.setText("LINEAR PHASE FILTERS", juce::dontSendNotification);
    filterSectionLabel.setFont(juce::Font(12.0f).boldened());
    filterSectionLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::accentColor);
    addAndMakeVisible(filterSectionLabel);
    
    eqSectionLabel.setText("ADAPTIVE EQUALIZER", juce::dontSendNotification);
    eqSectionLabel.setFont(juce::Font(12.0f).boldened());
    eqSectionLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::accentColor);
    addAndMakeVisible(eqSectionLabel);
    
    inputOutputSectionLabel.setText("INPUT / OUTPUT", juce::dontSendNotification);
    inputOutputSectionLabel.setFont(juce::Font(12.0f).boldened());
    inputOutputSectionLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::accentColor);
    addAndMakeVisible(inputOutputSectionLabel);
    
    // Control knobs
    inputGainKnob = std::make_unique<KnobComponent>("INPUT", audioProcessor.getValueTreeState(), ParameterIDs::inputGain);
    addAndMakeVisible(*inputGainKnob);
    
    driveKnob = std::make_unique<KnobComponent>("DRIVE", audioProcessor.getValueTreeState(), ParameterIDs::drive);
    addAndMakeVisible(*driveKnob);
    
    mixKnob = std::make_unique<KnobComponent>("MIX", audioProcessor.getValueTreeState(), ParameterIDs::mix);
    addAndMakeVisible(*mixKnob);
    
    outputGainKnob = std::make_unique<KnobComponent>("OUTPUT", audioProcessor.getValueTreeState(), ParameterIDs::outputGain);
    addAndMakeVisible(*outputGainKnob);
    
    // Filter controls
    lowCutKnob = std::make_unique<KnobComponent>("LOW CUT", audioProcessor.getValueTreeState(), ParameterIDs::lowCutFreq);
    addAndMakeVisible(*lowCutKnob);
    
    highCutKnob = std::make_unique<KnobComponent>("HIGH CUT", audioProcessor.getValueTreeState(), ParameterIDs::highCutFreq);
    addAndMakeVisible(*highCutKnob);
    
    filterEnableButton.setButtonText("ENABLE");
    filterEnableButton.setToggleable(true);
    filterEnableButton.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(filterEnableButton);
    filterEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), ParameterIDs::filterEnabled, filterEnableButton);
    
    // EQ controls
    eqStrengthKnob = std::make_unique<KnobComponent>("STRENGTH", audioProcessor.getValueTreeState(), ParameterIDs::eqAdaptionStrength);
    addAndMakeVisible(*eqStrengthKnob);
    
    eqSpeedKnob = std::make_unique<KnobComponent>("SPEED", audioProcessor.getValueTreeState(), ParameterIDs::eqReactionSpeed);
    addAndMakeVisible(*eqSpeedKnob);
    
    eqTargetCombo.addItem("Flat", 1);
    eqTargetCombo.addItem("Musical", 2);
    eqTargetCombo.addItem("Presence", 3);
    eqTargetCombo.addItem("Warm", 4);
    eqTargetCombo.addItem("Bright", 5);
    eqTargetCombo.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(eqTargetCombo);
    eqTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), ParameterIDs::eqTargetCurve, eqTargetCombo);
    
    eqEnableButton.setButtonText("ENABLE");
    eqEnableButton.setToggleable(true);
    eqEnableButton.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(eqEnableButton);
    eqEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), ParameterIDs::eqEnabled, eqEnableButton);
    
    // Saturation controls
    saturationTypeCombo.addItem("Tube Warm", 1);
    saturationTypeCombo.addItem("Tape Classic", 2);
    saturationTypeCombo.addItem("Transistor Modern", 3);
    saturationTypeCombo.addItem("Diode Harsh", 4);
    saturationTypeCombo.addItem("Vintage Fuzz", 5);
    saturationTypeCombo.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(saturationTypeCombo);
    saturationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), ParameterIDs::satType, saturationTypeCombo);
    
    soloButton.setButtonText("SOLO SAT");
    soloButton.setToggleable(true);
    soloButton.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(soloButton);
    soloAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), ParameterIDs::soloSaturation, soloButton);
    
    // Visualization components
    inputVUMeter = std::make_unique<VUMeter>(VUMeter::Input, &audioProcessor.getSaturationProcessor());
    addAndMakeVisible(*inputVUMeter);
    
    outputVUMeter = std::make_unique<VUMeter>(VUMeter::Output, nullptr, &audioProcessor.getLoudnessCompensator());
    addAndMakeVisible(*outputVUMeter);
    
    saturationViz = std::make_unique<SaturationVisualization>(audioProcessor.getSaturationProcessor(), audioProcessor.getValueTreeState());
    addAndMakeVisible(*saturationViz);
    
    eqDisplay = std::make_unique<EqualizerDisplay>(audioProcessor.getAdaptiveEqualizer());
    addAndMakeVisible(*eqDisplay);
}

void ProfessionalSaturationAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Gradient background
    juce::ColourGradient background(CustomLookAndFeel::backgroundColor.darker(0.2f), 0, 0,
                                   CustomLookAndFeel::backgroundColor, getWidth(), getHeight(), false);
    g.setGradientFill(background);
    g.fillAll();
    
    // Subtle pattern overlay
    g.setColour(CustomLookAndFeel::primaryColor.withAlpha(0.05f));
    for (int i = 0; i < getWidth(); i += 30)
    {
        g.drawVerticalLine(i, 0, getHeight());
    }
    for (int i = 0; i < getHeight(); i += 30)
    {
        g.drawHorizontalLine(i, 0, getWidth());
    }
    
    // Main border with glow effect
    auto bounds = getLocalBounds().toFloat();
    g.setColour(CustomLookAndFeel::accentColor.withAlpha(0.3f));
    g.drawRect(bounds, 3.0f);
    g.setColour(CustomLookAndFeel::accentColor.withAlpha(0.1f));
    g.drawRect(bounds.expanded(1.0f), 1.0f);
    g.drawRect(bounds.expanded(2.0f), 1.0f);
    
    // Section dividers
    g.setColour(CustomLookAndFeel::secondaryColor.withAlpha(0.3f));
    
    // Vertical dividers
    auto layout = calculateLayout(getLocalBounds());
    int dividerX1 = layout.inputVUArea.getRight() + 5;
    int dividerX2 = layout.saturationVizArea.getRight() + 5;
    
    g.drawVerticalLine(dividerX1, layout.titleArea.getBottom() + 10, getHeight() - 10);
    g.drawVerticalLine(dividerX2, layout.titleArea.getBottom() + 10, getHeight() - 10);
    
    // Horizontal dividers
    int dividerY = layout.saturationVizArea.getBottom() + 10;
    g.drawHorizontalLine(dividerY, 10, getWidth() - 10);
}

void ProfessionalSaturationAudioProcessorEditor::resized()
{
    // Calculate scale factor based on current size
    float scaleX = static_cast<float>(getWidth()) / static_cast<float>(baseWidth);
    float scaleY = static_cast<float>(getHeight()) / static_cast<float>(baseHeight);
    currentScaleFactor = juce::jmin(scaleX, scaleY);
    currentScaleFactor = juce::jlimit(minScaleFactor, maxScaleFactor, currentScaleFactor);
    
    auto layout = calculateLayout(getLocalBounds());
    
    // Position components
    titleLabel.setBounds(layout.titleArea);
    
    // VU Meters
    inputVUMeter->setBounds(layout.inputVUArea);
    outputVUMeter->setBounds(layout.outputVUArea);
    
    // Visualizations
    saturationViz->setBounds(layout.saturationVizArea);
    eqDisplay->setBounds(layout.eqDisplayArea);
    
    // Controls layout
    setupLayout();
}

void ProfessionalSaturationAudioProcessorEditor::timerCallback()
{
    // Update VU meters with current levels
    outputVUMeter->setLevels(audioProcessor.getOutputRMS(0), audioProcessor.getOutputRMS(1),
                           audioProcessor.getOutputPeak(0), audioProcessor.getOutputPeak(1));
}

ProfessionalSaturationAudioProcessorEditor::ComponentBounds ProfessionalSaturationAudioProcessorEditor::calculateLayout(juce::Rectangle<int> bounds)
{
    ComponentBounds layout;
    
    bounds.reduce(static_cast<int>(10 * currentScaleFactor), static_cast<int>(10 * currentScaleFactor));
    
    // Title area
    layout.titleArea = bounds.removeFromTop(static_cast<int>(40 * currentScaleFactor));
    bounds.removeFromTop(static_cast<int>(10 * currentScaleFactor));
    
    // Top section: VU meters and visualizations
    auto topSection = bounds.removeFromTop(static_cast<int>(200 * currentScaleFactor));
    
    // VU meters (left side)
    auto vuSection = topSection.removeFromLeft(static_cast<int>(120 * currentScaleFactor));
    layout.inputVUArea = vuSection.removeFromTop(vuSection.getHeight() / 2 - 5);
    vuSection.removeFromTop(10);
    layout.outputVUArea = vuSection;
    
    topSection.removeFromLeft(static_cast<int>(10 * currentScaleFactor));
    
    // Saturation visualization (center)
    layout.saturationVizArea = topSection.removeFromLeft(static_cast<int>(300 * currentScaleFactor));
    topSection.removeFromLeft(static_cast<int>(10 * currentScaleFactor));
    
    // EQ display (right)
    layout.eqDisplayArea = topSection;
    
    bounds.removeFromTop(static_cast<int>(20 * currentScaleFactor));
    
    // Bottom section: Controls
    layout.controlsArea = bounds.removeFromTop(static_cast<int>(120 * currentScaleFactor));
    bounds.removeFromTop(static_cast<int>(10 * currentScaleFactor));
    
    // Filter controls
    layout.filtersArea = bounds.removeFromTop(static_cast<int>(80 * currentScaleFactor));
    bounds.removeFromTop(static_cast<int>(10 * currentScaleFactor));
    
    // EQ controls
    layout.eqControlsArea = bounds.removeFromTop(static_cast<int>(80 * currentScaleFactor));
    bounds.removeFromTop(static_cast<int>(10 * currentScaleFactor));
    
    // Saturation controls
    layout.saturationControlsArea = bounds;
    
    return layout;
}

void ProfessionalSaturationAudioProcessorEditor::setupLayout()
{
    auto layout = calculateLayout(getLocalBounds());
    
    // Main controls section
    auto controlsBounds = layout.controlsArea;
    
    // Section label
    inputOutputSectionLabel.setBounds(controlsBounds.removeFromTop(15));
    controlsBounds.removeFromTop(5);
    
    // Distribute knobs evenly
    int knobWidth = controlsBounds.getWidth() / 4 - 10;
    
    inputGainKnob->setBounds(controlsBounds.removeFromLeft(knobWidth));
    controlsBounds.removeFromLeft(10);
    driveKnob->setBounds(controlsBounds.removeFromLeft(knobWidth));
    controlsBounds.removeFromLeft(10);
    mixKnob->setBounds(controlsBounds.removeFromLeft(knobWidth));
    controlsBounds.removeFromLeft(10);
    outputGainKnob->setBounds(controlsBounds);
    
    // Filter controls
    auto filterBounds = layout.filtersArea;
    filterSectionLabel.setBounds(filterBounds.removeFromTop(15));
    filterBounds.removeFromTop(5);
    
    auto filterControlWidth = filterBounds.getWidth() / 3 - 10;
    lowCutKnob->setBounds(filterBounds.removeFromLeft(filterControlWidth));
    filterBounds.removeFromLeft(10);
    highCutKnob->setBounds(filterBounds.removeFromLeft(filterControlWidth));
    filterBounds.removeFromLeft(10);
    filterEnableButton.setBounds(filterBounds.removeFromTop(30));
    
    // EQ controls
    auto eqBounds = layout.eqControlsArea;
    eqSectionLabel.setBounds(eqBounds.removeFromTop(15));
    eqBounds.removeFromTop(5);
    
    auto eqControlsTop = eqBounds.removeFromTop(eqBounds.getHeight() / 2);
    auto eqControlWidth = eqControlsTop.getWidth() / 2 - 5;
    
    eqStrengthKnob->setBounds(eqControlsTop.removeFromLeft(eqControlWidth));
    eqControlsTop.removeFromLeft(10);
    eqSpeedKnob->setBounds(eqControlsTop);
    
    auto eqComboWidth = eqBounds.getWidth() / 2 - 5;
    eqTargetCombo.setBounds(eqBounds.removeFromLeft(eqComboWidth));
    eqBounds.removeFromLeft(10);
    eqEnableButton.setBounds(eqBounds.removeFromTop(30));
    
    // Saturation controls
    auto satBounds = layout.saturationControlsArea;
    saturationSectionLabel.setBounds(satBounds.removeFromTop(15));
    satBounds.removeFromTop(5);
    
    auto satComboWidth = satBounds.getWidth() / 2 - 5;
    saturationTypeCombo.setBounds(satBounds.removeFromLeft(satComboWidth));
    satBounds.removeFromLeft(10);
    soloButton.setBounds(satBounds.removeFromTop(30));
    
    // Scale knobs based on current scale factor
    for (auto* knob : { inputGainKnob.get(), driveKnob.get(), mixKnob.get(), outputGainKnob.get(),
                       lowCutKnob.get(), highCutKnob.get(), eqStrengthKnob.get(), eqSpeedKnob.get() })
    {
        if (knob)
            knob->setKnobSize(currentScaleFactor);
    }
}

juce::Rectangle<int> ProfessionalSaturationAudioProcessorEditor::getScaledBounds(int baseWidthParam, int baseHeightParam, float scaleFactor)
{
    return juce::Rectangle<int>(0, 0, 
                              static_cast<int>(baseWidthParam * scaleFactor * currentScaleFactor),
                              static_cast<int>(baseHeightParam * scaleFactor * currentScaleFactor));
}
