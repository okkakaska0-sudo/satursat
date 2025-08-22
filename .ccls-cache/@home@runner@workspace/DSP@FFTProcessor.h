#pragma once

#include <JuceHeader.h>

class FFTProcessor
{
public:
    FFTProcessor();
    ~FFTProcessor() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    std::vector<float> getSpectrum(const juce::dsp::AudioBlock<const float>& block);
    std::vector<float> getFrequencies() const;
    float getMagnitudeAtFrequency(float frequency, const std::vector<float>& spectrum) const;

private:
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    
    std::vector<float> fftBuffer;
    std::vector<float> windowBuffer;
    std::vector<float> magnitudeSpectrum;
    std::vector<float> frequencies;
    
    float sampleRate = 44100.0f;
    static constexpr size_t fftSize = 2048;
    static constexpr size_t fftOrder = 11; // 2^11 = 2048
    
    size_t bufferIndex = 0;
    bool bufferFull = false;
    
    void processFFT();
    void calculateMagnitudeSpectrum();
};
