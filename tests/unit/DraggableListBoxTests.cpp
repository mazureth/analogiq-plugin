#include <JuceHeader.h>
#include "../../Source/DraggableListBox.h"

// TODO: If DraggableListBox or ListBoxModel are abstract, create a concrete test class or mock.

class MockListBoxModel : public juce::ListBoxModel
{
public:
    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {}
    void addItem(const juce::String &item) { items.add(item); }
    void clear() { items.clear(); }

private:
    juce::StringArray items;
};

class DraggableListBoxTests : public juce::UnitTest
{
public:
    DraggableListBoxTests() : UnitTest("DraggableListBoxTests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            auto model = std::make_unique<MockListBoxModel>();
            DraggableListBox listBox("Test", model.get());
            // TODO: Add more construction tests if needed
        }
        // Other tests can be re-enabled once DraggableListBox API is confirmed
    }
};

static DraggableListBoxTests draggableListBoxTests;