#include <JuceHeader.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // Ensure JUCE GUI + threading systems are initialized
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::UnitTestRunner testRunner;

    // For now, no tests are implemented in the new MVC architecture
    // Tests will be added here as we implement components
    std::cout << "No tests implemented yet in new MVC architecture.\n";
    std::cout << "Tests will be added as components are implemented.\n";

    // JUCE will run all tests (including theirs) automatically
    // We want to explicitly only run our tests
    juce::StringArray testsToRun;
    // testsToRun.add("FileSystemTests");        // Will be added
    // testsToRun.add("NetworkFetcherTests");    // Will be added
    // testsToRun.add("GearControlTests");       // Will be added
    // testsToRun.add("GearItemTests");          // Will be added
    // testsToRun.add("AnalogIQProcessorTests"); // Will be added

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

    if (selectedTests.isEmpty())
    {
        std::cout << "No tests selected to run.\n";
    }
    else
    {
        testRunner.runTests(selectedTests);
    }

    // COMPREHENSIVE CLEANUP to prevent JUCE leak detection at program exit

    // 1. Force early LookAndFeel cleanup to break Image references
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    // Create a temporary scope to force destruction of any remaining components
    {
        // This ensures any lingering component destructors run before leak detection
        juce::Component tempComponent;
        tempComponent.setLookAndFeel(nullptr);
    }

    // 2. Force cleanup of any cached JUCE resources
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