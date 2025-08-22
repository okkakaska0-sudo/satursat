#pragma once

// Core JUCE modules
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

// Plugin client for VST3/AU support
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

// Plugin defines for compilation
#ifndef JucePlugin_Name
    #define JucePlugin_Name "Saturation"
#endif

#ifndef JucePlugin_Desc
    #define JucePlugin_Desc "Professional audio saturation plugin"
#endif

#ifndef JucePlugin_Manufacturer
    #define JucePlugin_Manufacturer "AudioDev"
#endif

#ifndef JucePlugin_ManufacturerCode
    #define JucePlugin_ManufacturerCode 0x41647631 // 'Adv1'
#endif

#ifndef JucePlugin_PluginCode
    #define JucePlugin_PluginCode 0x53617431 // 'Sat1'
#endif

#ifndef JucePlugin_AUExportPrefix
    #define JucePlugin_AUExportPrefix SaturationAU
#endif

// Ensure we're in the JUCE namespace
using namespace juce;