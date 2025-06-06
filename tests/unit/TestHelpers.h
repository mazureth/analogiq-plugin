#pragma once

#include <JuceHeader.h>
#include <memory>

/**
 * @brief RAII wrapper for JUCE AudioProcessorEditor
 *
 * Ensures proper cleanup of editor resources when the wrapper goes out of scope.
 */
class ScopedEditor
{
public:
    explicit ScopedEditor(juce::AudioProcessor *processor)
        : editor(processor->createEditor())
    {
    }

    ~ScopedEditor()
    {
        if (editor != nullptr)
        {
            delete editor;
            editor = nullptr;
        }
    }

    // Prevent copying
    ScopedEditor(const ScopedEditor &) = delete;
    ScopedEditor &operator=(const ScopedEditor &) = delete;

    // Allow moving
    ScopedEditor(ScopedEditor &&other) noexcept
        : editor(other.editor)
    {
        other.editor = nullptr;
    }

    ScopedEditor &operator=(ScopedEditor &&other) noexcept
    {
        if (this != &other)
        {
            if (editor != nullptr)
                delete editor;
            editor = other.editor;
            other.editor = nullptr;
        }
        return *this;
    }

    juce::AudioProcessorEditor *get() { return editor; }
    juce::AudioProcessorEditor *operator->() { return editor; }
    juce::AudioProcessorEditor &operator*() { return *editor; }

private:
    juce::AudioProcessorEditor *editor;
};

/**
 * @brief RAII wrapper for JUCE Component
 *
 * Ensures proper cleanup of component and its children when the wrapper goes out of scope.
 */
class ScopedComponent
{
public:
    explicit ScopedComponent(juce::Component *component)
        : component(component)
    {
    }

    ~ScopedComponent()
    {
        if (component != nullptr)
        {
            component->removeAllChildren();
            delete component;
            component = nullptr;
        }
    }

    // Prevent copying
    ScopedComponent(const ScopedComponent &) = delete;
    ScopedComponent &operator=(const ScopedComponent &) = delete;

    // Allow moving
    ScopedComponent(ScopedComponent &&other) noexcept
        : component(other.component)
    {
        other.component = nullptr;
    }

    ScopedComponent &operator=(ScopedComponent &&other) noexcept
    {
        if (this != &other)
        {
            if (component != nullptr)
            {
                component->removeAllChildren();
                delete component;
            }
            component = other.component;
            other.component = nullptr;
        }
        return *this;
    }

    juce::Component *get() { return component; }
    juce::Component *operator->() { return component; }
    juce::Component &operator*() { return *component; }

private:
    juce::Component *component;
};