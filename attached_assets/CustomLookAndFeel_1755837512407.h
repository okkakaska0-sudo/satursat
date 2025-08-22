#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         juce::Slider::SliderStyle style, juce::Slider& slider) override;

    juce::Font getLabelFont(juce::Label& label) override;

    static const juce::Colour primaryColor;
    static const juce::Colour secondaryColor;
    static const juce::Colour accentColor;
    static const juce::Colour backgroundColor;
    static const juce::Colour textColor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
