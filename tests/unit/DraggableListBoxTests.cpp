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

            // Test model initialization
            expectEquals(model->getNumRows(), 0, "New model should have zero rows");

            // Test list box creation
            expect(listBox.getName() == "Test", "List box name should match constructor parameter");

            // Add some test items
            model->addItem("Item 1");
            model->addItem("Item 2");
            expectEquals(model->getNumRows(), 2, "Model should have two rows after adding items");

            // Test list box state
            // Note: isVisible() will be false until the component is added to a parent
            expect(!listBox.isVisible(), "List box should not be visible until added to a parent");
            expect(!listBox.isMouseButtonDown(), "List box should not have mouse button down initially");

            // Test component ID
            expectEquals(listBox.getComponentID(), juce::String("DraggableListBox"), "Component ID should be set correctly");
        }
        // Other tests can be re-enabled once DraggableListBox API is confirmed
    }
};

static DraggableListBoxTests draggableListBoxTests;