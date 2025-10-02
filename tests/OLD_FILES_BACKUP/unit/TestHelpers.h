#pragma once

#include <JuceHeader.h>
#include <memory>

/**
 * @brief Recursively clears LookAndFeel from a component and all its children
 * @param component The component to clear LookAndFeel from
 */
inline void clearLookAndFeelRecursive(juce::Component *component)
{
    if (component == nullptr)
        return;

    // Clear this component's LookAndFeel
    component->setLookAndFeel(nullptr);

    // Recursively clear all children
    for (int i = 0; i < component->getNumChildComponents(); ++i)
    {
        clearLookAndFeelRecursive(component->getChildComponent(i));
    }
}

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
            // Clear any LookAndFeel references to prevent assertion failures
            editor->setLookAndFeel(nullptr);

            // Clear LookAndFeel from all child components recursively
            clearLookAndFeelRecursive(editor);

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
            // Clear any LookAndFeel references to prevent assertion failures
            component->setLookAndFeel(nullptr);

            // Clear LookAndFeel from all child components recursively
            clearLookAndFeelRecursive(component);

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

/**
 * @brief RAII wrapper for JUCE Image
 *
 * Ensures proper cleanup of image resources when the wrapper goes out of scope.
 */
class ScopedImage
{
public:
    explicit ScopedImage(const juce::Image &img = juce::Image()) : image(img) {}

    ~ScopedImage()
    {
        // Clear the image to free memory
        image = juce::Image();
    }

    // Prevent copying
    ScopedImage(const ScopedImage &) = delete;
    ScopedImage &operator=(const ScopedImage &) = delete;

    // Allow moving
    ScopedImage(ScopedImage &&other) noexcept : image(std::move(other.image))
    {
        other.image = juce::Image();
    }

    ScopedImage &operator=(ScopedImage &&other) noexcept
    {
        if (this != &other)
        {
            image = std::move(other.image);
            other.image = juce::Image();
        }
        return *this;
    }

    juce::Image &get() { return image; }
    const juce::Image &get() const { return image; }
    juce::Image &operator*() { return image; }
    const juce::Image &operator*() const { return image; }

private:
    juce::Image image;
};

/**
 * @brief RAII wrapper for JUCE StringArray
 *
 * Ensures proper cleanup of string array resources when the wrapper goes out of scope.
 */
class ScopedStringArray
{
public:
    explicit ScopedStringArray() = default;
    explicit ScopedStringArray(const juce::StringArray &arr) : stringArray(arr) {}

    ~ScopedStringArray()
    {
        // Clear the array to free memory
        stringArray.clear();
    }

    // Prevent copying
    ScopedStringArray(const ScopedStringArray &) = delete;
    ScopedStringArray &operator=(const ScopedStringArray &) = delete;

    // Allow moving
    ScopedStringArray(ScopedStringArray &&other) noexcept : stringArray(std::move(other.stringArray))
    {
        other.stringArray.clear();
    }

    ScopedStringArray &operator=(ScopedStringArray &&other) noexcept
    {
        if (this != &other)
        {
            stringArray = std::move(other.stringArray);
            other.stringArray.clear();
        }
        return *this;
    }

    juce::StringArray &get() { return stringArray; }
    const juce::StringArray &get() const { return stringArray; }
    juce::StringArray &operator*() { return stringArray; }
    const juce::StringArray &operator*() const { return stringArray; }

private:
    juce::StringArray stringArray;
};