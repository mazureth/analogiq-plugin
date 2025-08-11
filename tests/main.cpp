#include <JuceHeader.h>
#include "../Source/CacheManager.h"
#include "../Source/PresetManager.h"
#include "unit/MockNetworkFetcher.h"
#include "unit/MockFileSystem.h"
#include <iostream>

int main(int argc, char *argv[])
{
    // Ensure JUCE GUI + threading systems are initialized
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::UnitTestRunner testRunner;

    // JUCE will run all tests (including theirs) automatically
    // We want to explicitly only run our tests
    juce::StringArray testsToRun;
    testsToRun.add("CacheManagerTests");
    testsToRun.add("DraggableListBoxTests");
    testsToRun.add("FileSystemTests");
    testsToRun.add("GearItemTests");
    testsToRun.add("GearLibraryTests");
    testsToRun.add("NetworkFetcherTests");
    testsToRun.add("NotesPanelTests");
    testsToRun.add("AnalogIQEditorTests");
    testsToRun.add("AnalogIQProcessorTests");
    testsToRun.add("PresetManagerTests");
    testsToRun.add("PresetIntegrationTests");
    testsToRun.add("RackSlotTests");
    testsToRun.add("RackTests");

    // Build a list of test pointers by name
    juce::Array<juce::UnitTest *> selectedTests;
    auto &registeredTests = juce::UnitTest::getAllTests();
    for (auto *test : registeredTests)
        if (testsToRun.contains(test->getName()))
            selectedTests.add(test);

    // List selected tests
    std::cout << "Running the following tests:\n";
    for (auto *test : selectedTests)
        std::cout << " - " << test->getName() << std::endl;

    testRunner.runTests(selectedTests);

    // COMPREHENSIVE CLEANUP to prevent JUCE leak detection at program exit

    // 1. Clear all singleton mock state completely
    ConcreteMockNetworkFetcher::getInstance().reset();
    ConcreteMockFileSystem::getInstance().reset();

    // 2. Force early LookAndFeel cleanup to break Image references
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    // Create a temporary scope to force destruction of any remaining components
    {
        // This ensures any lingering component destructors run before leak detection
        juce::Component tempComponent;
        tempComponent.setLookAndFeel(nullptr);
    }

    // 3. Force cleanup of any cached JUCE resources
    {
        // Create and destroy temporary objects to trigger internal cleanup
        juce::Image tempImage(juce::Image::RGB, 1, 1, true);
        tempImage = juce::Image();

        juce::StringArray tempArray;
        tempArray.add("cleanup");
        tempArray.clear();

        // Process any remaining messages
        if (auto *mm = juce::MessageManager::getInstance())
        {
            juce::Thread::sleep(10);
            mm->deliverBroadcastMessage(juce::String());
        }
    }

    // First pass: Try to release unused images
    juce::ImageCache::releaseUnusedImages();
    juce::ImageCache::setCacheTimeout(0);

    // Clear any remaining LookAndFeel references
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    // Final aggressive image cache cleanup
    juce::ImageCache::releaseUnusedImages();

    // Last-ditch effort: Force deletion of all cached objects
    juce::DeletedAtShutdown::deleteAll();

    // Final ImageCache cleanup after DeletedAtShutdown
    juce::ImageCache::releaseUnusedImages();

    return 0;
}