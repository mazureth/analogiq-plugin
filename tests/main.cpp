#include <JuceHeader.h>

int main(int argc, char *argv[])
{

    // Ensure JUCE GUI + threading systems are initialized
    juce::ScopedJuceInitialiser_GUI guiInit;

    juce::UnitTestRunner testRunner;

    // JUCE will run all tests (including theirs) automatically
    // We want to explicitly only run our tests
    juce::StringArray testsToRun;
    testsToRun.add("DraggableListBoxTests");
    testsToRun.add("GearItemTests");
    testsToRun.add("GearLibraryTests");
    testsToRun.add("NotesPanelTests");
    testsToRun.add("PluginEditorTests");
    testsToRun.add("PluginProcessorTests");
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

    // Explicit teardown for lingering threads/resources
    juce::DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();

    return 0;
}