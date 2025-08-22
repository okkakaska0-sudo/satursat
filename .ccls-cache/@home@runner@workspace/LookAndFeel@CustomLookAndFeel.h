#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Slider drawing
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         juce::Slider::SliderStyle style, juce::Slider& slider) override;

    // Button drawing
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    // ComboBox drawing
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH,
                     juce::ComboBox& box) override;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

    // Font handling
    juce::Font getLabelFont(juce::Label& label) override;
    juce::Font getComboBoxFont(juce::ComboBox& box) override;

    // Color scheme
    static const juce::Colour primaryColor;
    static const juce::Colour secondaryColor;
    static const juce::Colour accentColor;
    static const juce::Colour backgroundColor;
    static const juce::Colour textColor;
    static const juce::Colour warningColor;
    static const juce::Colour successColor;

private:
    void drawGlowEffect(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color, float intensity);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
