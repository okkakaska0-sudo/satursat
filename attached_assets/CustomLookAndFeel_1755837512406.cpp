#include "CustomLookAndFeel.h"

const juce::Colour CustomLookAndFeel::primaryColor = juce::Colour(0xff2d3748);
const juce::Colour CustomLookAndFeel::secondaryColor = juce::Colour(0xff4a5568);
const juce::Colour CustomLookAndFeel::accentColor = juce::Colour(0xff63b3ed);
const juce::Colour CustomLookAndFeel::backgroundColor = juce::Colour(0xff1a202c);
const juce::Colour CustomLookAndFeel::textColor = juce::Colour(0xfff7fafc);

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, backgroundColor);
    setColour(juce::Label::textColourId, textColor);
    setColour(juce::Slider::thumbColourId, accentColor);
    setColour(juce::Slider::trackColourId, secondaryColor);
    setColour(juce::Slider::rotarySliderFillColourId, accentColor);
    setColour(juce::Slider::rotarySliderOutlineColourId, primaryColor);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider&)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    // Draw background arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                               0.0f, rotaryStartAngle, rotaryEndAngle, true);

    g.setColour(primaryColor);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw value arc
    if (sliderPos > 0.0f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, toAngle, true);

        g.setColour(accentColor);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Draw thumb
    juce::Path thumb;
    auto thumbWidth = lineW * 2.0f;
    thumb.addRectangle(-thumbWidth / 2, -thumbWidth / 2, thumbWidth, thumbWidth);

    g.setColour(textColor);
    g.fillPath(thumb, juce::AffineTransform::rotation(toAngle + juce::MathConstants<float>::halfPi)
               .translated(bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                          bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi)));
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.isBar())
    {
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
                                         : juce::Rectangle<float>(x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
    }
    else
    {
        auto isTwoVal = (style == juce::Slider::SliderStyle::TwoValueVertical || style == juce::Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == juce::Slider::SliderStyle::ThreeValueVertical || style == juce::Slider::SliderStyle::ThreeValueHorizontal);

        auto trackWidth = juce::jmin(6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);

        juce::Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f,
                                     slider.isHorizontal() ? y + height * 0.5f : height + y);

        juce::Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x,
                                   slider.isHorizontal() ? startPoint.y : y);

        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        g.setColour(secondaryColor);
        g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        juce::Path valueTrack;
        juce::Point<float> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
                        slider.isHorizontal() ? height * 0.5f : minSliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
                        slider.isHorizontal() ? height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
            auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

        auto thumbWidth = getSliderThumbRadius(slider);

        valueTrack.startNewSubPath(minPoint);
        valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
        g.setColour(accentColor);
        g.strokePath(valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

        if (!isTwoVal)
        {
            g.setColour(textColor);
            g.fillEllipse(juce::Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = juce::jmin(trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
            auto pointerColour = slider.findColour(juce::Slider::thumbColourId);

            if (slider.isHorizontal())
            {
                drawPointer(g, minSliderPos - sr,
                           juce::jmax(0.0f, y + height * 0.5f - trackWidth * 2.0f),
                           trackWidth * 2.0f, pointerColour, 2);

                drawPointer(g, maxSliderPos - trackWidth,
                           juce::jmin(y + height - trackWidth * 2.0f, y + height * 0.5f),
                           trackWidth * 2.0f, pointerColour, 4);
            }
            else
            {
                drawPointer(g, juce::jmax(0.0f, x + width * 0.5f - trackWidth * 2.0f),
                           minSliderPos - trackWidth,
                           trackWidth * 2.0f, pointerColour, 1);

                drawPointer(g, juce::jmin(x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                           trackWidth * 2.0f, pointerColour, 3);
            }
        }
    }
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(14.0f, juce::Font::bold);
}
