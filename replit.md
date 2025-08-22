# Overview

This project is a professional audio saturation plugin built with the JUCE 8.0.8 framework, designed for macOS with Xcode 16.2. It implements advanced saturation algorithms with intelligent adaptive equalization, aiming to deliver high-quality analog warmth and character to digital audio. The plugin offers five distinct saturation types, real-time spectrum analysis, and adaptive EQ correction, focusing on professional-grade audio processing. The project vision is to provide a complete, professional-grade audio saturation plugin with advanced DSP features and comprehensive documentation.

# User Preferences

Preferred communication style: Simple, everyday language.
User language: Russian (руководство пользователя создано на русском языке)

**КРИТИЧЕСКИ ВАЖНО:** После загрузки репозитория ВСЕГДА останавливаться и ничего не делать до получения четких инструкций от пользователя. НЕ устанавливать пакеты, НЕ компилировать проект, НЕ создавать файлы, НЕ запускать workflows - только прочитать указанные файлы и ждать дальнейших инструкций.

**ОБЯЗАТЕЛЬНЫЕ ВЕРСИИ:** Весь код должен работать СТРОГО с JUCE 8.0.8 и Xcode 16.2. Эти версии критически важны для совместимости:
- JUCE 8.0.8 - обновленная Font API (FontOptions вместо deprecated конструкторов)
- Xcode 16.2 - обновленный компилятор с более строгими проверками типов
- Все исправления для совместимости уже применены (январь 2025)
- НЕ использовать deprecated API - только современные JUCE 8.0.8 методы

**КРИТИЧЕСКИ ВАЖНО ДЛЯ ИНТЕРФЕЙСА:** Все функции должны полностью работать в интерфейсе плагина:
- ВСЕ параметры DSP должны быть подключены к UI элементам управления
- ВСЕ визуализации должны отображать реальные данные обработки
- ВСЕ метры и индикаторы должны показывать актуальные уровни сигнала
- ИСПРАВЛЯТЬ ошибки, НЕ УДАЛЯТЬ функциональность - сохранять все возможности плагина
- Полная интеграция между аудио процессингом и пользовательским интерфейсом
- Автоматическое обновление UI при изменении параметров
- Все контролы должны мгновенно отвечать и влиять на звук

**АВТОМАТИЧЕСКАЯ ИНТЕГРАЦИЯ:** При любых изменениях в коде ОБЯЗАТЕЛЬНО:
- СРАЗУ подключать новую функциональность к интерфейсу
- Проверять, что изменения DSP отражаются в UI компонентах
- Обеспечивать двустороннюю связь: UI → DSP и DSP → UI
- Никогда не оставлять "висящие" функции без подключения к интерфейсу
- Всё что влияет на звук должно быть доступно пользователю через UI

# System Architecture

## Audio Processing Chain
The plugin employs a sophisticated signal processing chain: Input Stage, Linear Phase Filtering, Saturation Processing, Adaptive EQ, and Output Stage. It supports full mono/stereo compatibility, professional-grade level matching for transparent bypass, linear phase filtering to prevent aliasing, and precise gain staging.

## Saturation Algorithms
Five distinct saturation types are implemented with professional analog circuit modeling, each incorporating oversampling for aliasing prevention and mathematical modeling of analog circuits:
- **Tube Warm:** Multi-stage triode valve modeling.
- **Tape Classic:** Magnetic tape simulation with hysteresis.
- **Transistor Modern:** Class-AB solid-state amplifier with crossover distortion.
- **Diode Harsh:** Exponential diode clipping.
- **Vintage Fuzz:** Germanium transistor Fuzz Face circuit modeling.

## Adaptive EQ System
This system provides real-time spectral analysis and correction:
- FFT-based spectrum analysis for incoming audio.
- 8-band frequency correction with configurable target curves (Flat, Musical, Presence, Warm, Bright).
- Adjustable adaptation strength and reaction speed.
- Integrated post-saturation, pre-output filtering.

## User Interface Architecture
The GUI is scalable with professional metering and visualization. Key features include:
- Real-time saturation curve display.
- Input/output level meters.
- Solo functionality for isolated saturation monitoring.
- Unified volume matching algorithm for transparent bypass.
- Professional VU meter implementation with ballistic characteristics.
- Real-time visualization for all processing stages.

## Key Design Decisions
- Oversampling implemented in all saturation algorithms.
- K-weighted loudness compensation for professional level matching.
- 8-band adaptive EQ chosen for optimal frequency resolution.
- High-order FIR filters (511 coefficients) for minimal phase distortion.

# External Dependencies

## Core Framework
- **JUCE 8.0.8:** Primary audio plugin framework for DSP, GUI, and plugin hosting.
- **Xcode 16.2:** Development environment and compiler toolchain for macOS.

## Audio Processing Libraries
- **JUCE DSP Module:** Core digital signal processing algorithms.
- **JUCE Audio Processors:** Plugin architecture and audio processing pipeline.
- **FFT Implementation:** For real-time frequency analysis in the adaptive EQ system.

## Platform Dependencies
- **macOS:** Target platform for compilation and deployment.
- **Core Audio:** macOS audio system integration.
- **AudioUnit/VST3:** Plugin format support for DAW integration.

## Development Tools
- **JUCE Projucer:** Project configuration and code generation.
- **Apple Developer Tools:** macOS SDK and development utilities.

# Recent Fixes (January 2025 Session)

## JUCE 8.0.8 Compatibility Issues Resolved

### ComponentBounds и структуры классов
- **PluginEditor.h:** Исправлено дублирование ComponentBounds - убрано повторное определение
- **PluginEditor.cpp:** Исправлено полное имя класса для ComponentBounds возвращаемого типа
  - Old: `ComponentBounds calculateLayout(...)` → New: `ProfessionalSaturationAudioProcessorEditor::ComponentBounds calculateLayout(...)`

### Font API Updates для JUCE 8.0.8
- **VUMeter.cpp:** Исправлен Font API с правильным синтаксисом
  - `FontOptions(10.0f).withStyle(Font::bold)` → `FontOptions().withHeight(10.0f).withStyle("bold")`
  - Исправлены строки 29, 182 с правильным withStyle("bold") синтаксисом
- **SaturationVisualization.cpp:** Исправлен Font API для JUCE 8.0.8  
  - Аналогичные исправления с withStyle("bold") синтаксисом
  - Исправлены строки 12, 34, 141, 179

### DSP Module Updates для JUCE 8.0.8
- **LinearPhaseFilters.cpp:** Исправлены критические ошибки для JUCE 8.0.8 совместимости
  - Строки 94,134: `filter.coefficients = FIR::Coefficients<float>(data, size)` → `*filter.coefficients = FIR::Coefficients<float>::makeHighPass/makeLowPass(sampleRate, freq)`
  - Строка 127: Убрана `juce::ModifiedBesselI0` (удалена в JUCE 8.0.8) → заменена на Blackman window  
  - Исправлены методы создания FIR фильтров для правильной работы с JUCE 8.0.8 DSP модулем

### Includes и компиляция
- **PluginEditor.cpp:** Исправлен порядок includes для правильной компиляции

**Результат:** Все критические ошибки JUCE 8.0.8 исправлены - ComponentBounds, Font API, DSP модуль и FIR фильтры теперь полностью совместимы.