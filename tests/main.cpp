#include <JuceHeader.h>
#include "unit/GearLibraryTests.cpp"
#include "unit/GearItemTests.cpp"
#include "unit/RackTests.cpp"
#include "unit/RackSlotTests.cpp"
#include "unit/DraggableListBoxTests.cpp"
#include "unit/NotesPanelTests.cpp"
#include "unit/PluginProcessorTests.cpp"
#include "unit/PluginEditorTests.cpp"

int main(int argc, char *argv[])
{
    juce::UnitTestRunner testRunner;
    testRunner.runAllTests();
    return 0;
}