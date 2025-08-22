#include "CustomLookAndFeel.h"

// Color definitions
const juce::Colour CustomLookAndFeel::primaryColor = juce::Colour(0xff2a3441);
const juce::Colour CustomLookAndFeel::secondaryColor = juce::Colour(0xff3e4a59);
const juce::Colour CustomLookAndFeel::accentColor = juce::Colour(0xff5fb3d4);
const juce::Colour CustomLookAndFeel::backgroundColor = juce::Colour(0xff1e252e);
const juce::Colour CustomLookAndFeel::textColor = juce::Colour(0xfff5f7fa);
const juce::Colour CustomLookAndFeel::warningColor = juce::Colour(0xffe74c3c);
const juce::Colour CustomLookAndFeel::successColor = juce::Colour(0xff2ecc71);

CustomLookAndFeel::CustomLookAndFeel()
{
    // Set default colors
    setColour(juce::ResizableWindow::backgroundColourId, backgroundColor);
    setColour(juce::Label::textColourId, textColor);
    setColour(juce::Slider::thumbColourId, accentColor);
    setColour(juce::Slider::trackColourId, secondaryColor);
    setColour(juce::Slider::rotarySliderFillColourId, accentColor);
    setColour(juce::Slider::rotarySliderOutlineColourId, primaryColor);
    setColour(juce::Slider::textBoxTextColourId, textColor);
    setColour(juce::Slider::textBoxBackgroundColourId, primaryColor.darker(0.3f));
    setColour(juce::Slider::textBoxOutlineColourId, secondaryColor);
    
    // Button colors
    setColour(juce::TextButton::buttonColourId, primaryColor);
    setColour(juce::TextButton::buttonOnColourId, accentColor);
    setColour(juce::TextButton::textColourOffId, textColor);
    setColour(juce::TextButton::textColourOnId, backgroundColor);
    
    // ComboBox colors
    setColour(juce::ComboBox::backgroundColourId, primaryColor);
    setColour(juce::ComboBox::textColourId, textColor);
    setColour(juce::ComboBox::outlineColourId, secondaryColor);
    setColour(juce::ComboBox::arrowColourId, accentColor);
    
    // PopupMenu colors
    setColour(juce::PopupMenu::backgroundColourId, primaryColor.darker(0.2f));
    setColour(juce::PopupMenu::textColourId, textColor);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accentColor);
    setColour(juce::PopupMenu::highlightedTextColourId, backgroundColor);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(15);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(6.0f, radius * 0.4f);
    auto arcRadius = radius - lineW * 0.5f;
    
    // Draw glow effect
    drawGlowEffect(g, bounds, accentColor, 0.3f);
    
    // Draw background arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                               0.0f, rotaryStartAngle, rotaryEndAngle, true);
    
    g.setColour(primaryColor.brighter(0.1f));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw track groove
    g.setColour(primaryColor.darker(0.3f));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW * 0.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw value arc with gradient
    if (sliderPos > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                              0.0f, rotaryStartAngle, toAngle, true);
        
        // Create gradient for the value arc
        juce::ColourGradient gradient(accentColor.darker(0.2f), bounds.getCentreX() - arcRadius, bounds.getCentreY(),
                                    accentColor.brighter(0.3f), bounds.getCentreX() + arcRadius, bounds.getCentreY(), false);
        g.setGradientFill(gradient);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Add highlight
        g.setColour(accentColor.brighter(0.5f).withAlpha(0.6f));
        g.strokePath(valueArc, juce::PathStrokeType(lineW * 0.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    // Draw outer ring
    g.setColour(secondaryColor);
    g.drawEllipse(bounds.reduced(2), 1.5f);
    
    // Draw thumb with enhanced 3D effect
    auto thumbRadius = lineW * 1.2f;
    auto thumbX = bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi);
    auto thumbY = bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi);
    
    // Thumb shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillEllipse(thumbX - thumbRadius + 1, thumbY - thumbRadius + 1, thumbRadius * 2, thumbRadius * 2);
    
    // Thumb base
    juce::ColourGradient thumbGradient(textColor, thumbX, thumbY - thumbRadius,
                                      textColor.darker(0.3f), thumbX, thumbY + thumbRadius, false);
    g.setGradientFill(thumbGradient);
    g.fillEllipse(thumbX - thumbRadius, thumbY - thumbRadius, thumbRadius * 2, thumbRadius * 2);
    
    // Thumb highlight
    g.setColour(textColor.brighter(0.4f));
    g.fillEllipse(thumbX - thumbRadius * 0.6f, thumbY - thumbRadius * 0.8f, 
                 thumbRadius * 0.8f, thumbRadius * 0.4f);
    
    // Center dot
    g.setColour(primaryColor.darker(0.2f));
    g.fillEllipse(bounds.getCentreX() - 3, bounds.getCentreY() - 3, 6, 6);
    g.setColour(accentColor.withAlpha(0.8f));
    g.fillEllipse(bounds.getCentreX() - 2, bounds.getCentreY() - 2, 4, 4);
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (slider.isBar())
    {
        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillRect(slider.isHorizontal() ? juce::Rectangle<float>(static_cast<float>(x), y + 0.5f, sliderPos - x, height - 1.0f)
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
        
        // Background track with gradient
        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint);
        backgroundTrack.lineTo(endPoint);
        
        g.setColour(primaryColor.darker(0.3f));
        g.strokePath(backgroundTrack, { trackWidth + 2, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
        
        g.setColour(secondaryColor);
        g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
        
        // Value track
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
        
        valueTrack.startNewSubPath(minPoint);
        valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
        
        // Value track with gradient
        juce::ColourGradient valueGradient(accentColor.darker(0.2f), startPoint.x, startPoint.y,
                                          accentColor.brighter(0.2f), endPoint.x, endPoint.y, false);
        g.setGradientFill(valueGradient);
        g.strokePath(valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
        
        // Thumb
        if (!isTwoVal)
        {
            auto thumbWidth = getSliderThumbRadius(slider) * 2;
            juce::Point<float> thumbPos = isThreeVal ? thumbPoint : maxPoint;
            
            // Thumb shadow
            g.setColour(juce::Colours::black.withAlpha(0.3f));
            g.fillEllipse(thumbPos.x - thumbWidth * 0.5f + 1, thumbPos.y - thumbWidth * 0.5f + 1, 
                         thumbWidth, thumbWidth);
            
            // Thumb gradient
            juce::ColourGradient thumbGradient(textColor, thumbPos.x, thumbPos.y - thumbWidth * 0.5f,
                                              textColor.darker(0.3f), thumbPos.x, thumbPos.y + thumbWidth * 0.5f, false);
            g.setGradientFill(thumbGradient);
            g.fillEllipse(thumbPos.x - thumbWidth * 0.5f, thumbPos.y - thumbWidth * 0.5f, thumbWidth, thumbWidth);
            
            // Thumb highlight
            g.setColour(textColor.brighter(0.4f));
            g.fillEllipse(thumbPos.x - thumbWidth * 0.3f, thumbPos.y - thumbWidth * 0.4f, 
                         thumbWidth * 0.6f, thumbWidth * 0.3f);
        }
    }
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1);
    auto cornerSize = 6.0f;
    
    // Base color
    juce::Colour baseColour = backgroundColour;
    if (button.getToggleState())
        baseColour = accentColor;
    else if (shouldDrawButtonAsHighlighted)
        baseColour = backgroundColour.brighter(0.2f);
    
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
    
    // Button shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds.translated(1, 1), cornerSize);
    
    // Button gradient
    juce::ColourGradient gradient(baseColour.brighter(0.1f), 0, bounds.getY(),
                                 baseColour.darker(0.1f), 0, bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Button border
    g.setColour(button.getToggleState() ? accentColor.brighter(0.3f) : secondaryColor);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    
    // Inner highlight
    if (button.getToggleState() || shouldDrawButtonAsHighlighted)
    {
        g.setColour(textColor.withAlpha(0.2f));
        g.drawRoundedRectangle(bounds.reduced(1), cornerSize - 1, 1.0f);
    }
}

void CustomLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);
    
    juce::Colour textColour = textColor;
    if (button.getToggleState())
        textColour = backgroundColor;
    else if (shouldDrawButtonAsHighlighted)
        textColour = textColor.brighter(0.2f);
    
    g.setColour(textColour);
    
    const int yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
    const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;
    
    const int fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
    const int leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;
    
    if (textWidth > 0)
        g.drawFittedText(button.getButtonText(),
                        leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                        juce::Justification::centred, 2);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                    int buttonX, int buttonY, int buttonW, int buttonH,
                                    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    auto cornerSize = 4.0f;
    
    // Background gradient
    juce::ColourGradient background(primaryColor.brighter(0.1f), 0, 0,
                                   primaryColor.darker(0.1f), 0, height, false);
    g.setGradientFill(background);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Border
    g.setColour(box.hasKeyboardFocus(true) ? accentColor : secondaryColor);
    g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    
    // Arrow button area
    auto arrowBounds = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH).reduced(2);
    
    // Arrow background
    if (isButtonDown)
    {
        g.setColour(accentColor.withAlpha(0.3f));
        g.fillRoundedRectangle(arrowBounds, 2.0f);
    }
    
    // Draw arrow
    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    auto arrowX = arrowBounds.getCentreX();
    auto arrowY = arrowBounds.getCentreY();
    
    juce::Path arrow;
    arrow.addTriangle(arrowX - 4, arrowY - 2, arrowX + 4, arrowY - 2, arrowX, arrowY + 3);
    g.fillPath(arrow);
}

void CustomLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(4, 1, box.getWidth() - 30, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label& label)
{
    juce::ignoreUnused(label);
    return juce::Font(12.0f, juce::Font::bold);
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    juce::ignoreUnused(box);
    return juce::Font(11.0f);
}

void CustomLookAndFeel::drawGlowEffect(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour color, float intensity)
{
    for (int i = 0; i < 5; ++i)
    {
        g.setColour(color.withAlpha(intensity * (0.1f - i * 0.02f)));
        g.drawEllipse(bounds.expanded(i * 2), 1.0f);
    }
}
