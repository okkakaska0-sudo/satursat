#include "FFTProcessor.h"

FFTProcessor::FFTProcessor() 
    : fft(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    fftBuffer.resize(fftSize * 2, 0.0f); // Complex pairs
    windowBuffer.resize(fftSize, 0.0f);
    magnitudeSpectrum.resize(fftSize / 2, 0.0f);
    frequencies.resize(fftSize / 2, 0.0f);
}

void FFTProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    
    // Calculate frequency bins
    for (size_t i = 0; i < frequencies.size(); ++i)
    {
        frequencies[i] = (static_cast<float>(i) / static_cast<float>(fftSize)) * sampleRate;
    }
    
    reset();
}

void FFTProcessor::reset()
{
    std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
    std::fill(windowBuffer.begin(), windowBuffer.end(), 0.0f);
    std::fill(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), 0.0f);
    
    bufferIndex = 0;
    bufferFull = false;
}

std::vector<float> FFTProcessor::getSpectrum(const juce::dsp::AudioBlock<const float>& block)
{
    const auto numSamples = block.getNumSamples();
    const auto numChannels = block.getNumChannels();
    
    // Mix down to mono and fill window buffer
    for (size_t sample = 0; sample < numSamples; ++sample)
    {
        float mixedSample = 0.0f;
        
        // Mix all channels
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            mixedSample += block.getSample(static_cast<int>(channel), static_cast<int>(sample));
        }
        mixedSample /= static_cast<float>(numChannels);
        
        // Fill circular buffer
        windowBuffer[bufferIndex] = mixedSample;
        bufferIndex = (bufferIndex + 1) % fftSize;
        
        if (bufferIndex == 0)
            bufferFull = true;
    }
    
    // Process FFT if we have enough data
    if (bufferFull)
    {
        processFFT();
        calculateMagnitudeSpectrum();
    }
    
    return magnitudeSpectrum;
}

std::vector<float> FFTProcessor::getFrequencies() const
{
    return frequencies;
}

float FFTProcessor::getMagnitudeAtFrequency(float frequency, const std::vector<float>& spectrum) const
{
    if (spectrum.empty() || frequency <= 0.0f)
        return 0.0f;
    
    // Find the closest bin
    float binWidth = sampleRate / static_cast<float>(fftSize);
    size_t binIndex = static_cast<size_t>(frequency / binWidth);
    binIndex = juce::jlimit(size_t(0), spectrum.size() - 1, binIndex);
    
    return spectrum[binIndex];
}

void FFTProcessor::processFFT()
{
    // Copy windowed data to FFT buffer (complex format)
    for (size_t i = 0; i < fftSize; ++i)
    {
        size_t readIndex = (bufferIndex + i) % fftSize;
        fftBuffer[i * 2] = windowBuffer[readIndex]; // Real part
        fftBuffer[i * 2 + 1] = 0.0f; // Imaginary part
    }
    
    // Apply window function
    window.multiplyWithWindowingTable(fftBuffer.data(), fftSize);
    
    // Perform FFT
    fft.performFrequencyOnlyForwardTransform(fftBuffer.data());
}

void FFTProcessor::calculateMagnitudeSpectrum()
{
    for (size_t i = 0; i < magnitudeSpectrum.size(); ++i)
    {
        float real = fftBuffer[i * 2];
        float imaginary = fftBuffer[i * 2 + 1];
        
        // Calculate magnitude
        float magnitude = std::sqrt(real * real + imaginary * imaginary);
        
        // Normalize and convert to useful range
        magnitude /= static_cast<float>(fftSize);
        magnitude *= 2.0f; // Account for negative frequencies
        
        magnitudeSpectrum[i] = magnitude;
    }
}
