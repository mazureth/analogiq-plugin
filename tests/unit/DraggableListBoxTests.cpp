#include <JuceHeader.h>
#include "DraggableListBox.h"

class MockListBoxModel : public juce::ListBoxModel
{
public:
    int getNumRows() override { return items.size(); }

    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override
    {
        if (rowNumber >= 0 && rowNumber < items.size())
        {
            g.fillAll(rowIsSelected ? juce::Colours::lightblue : juce::Colours::white);
            g.setColour(juce::Colours::black);
            g.drawText(items[rowNumber], 0, 0, width, height, juce::Justification::centredLeft);
        }
    }

    void addItem(const juce::String &item) { items.add(item); }
    void clear() { items.clear(); }

private:
    juce::StringArray items;
};

class DraggableListBoxTests : public juce::UnitTest
{
public:
    DraggableListBoxTests() : UnitTest("DraggableListBox Tests") {}

    void runTest() override
    {
        beginTest("Construction");
        {
            auto model = std::make_unique<MockListBoxModel>();
            DraggableListBox listBox("Test", model.get());
            expect(listBox.getListBoxModel() == nullptr);
        }

        beginTest("Set Model");
        {
            auto model = std::make_unique<MockListBoxModel>();
            model->addItem("Test Item");

            DraggableListBox listBox("Test", model.get());
            listBox.setModel(model.get());
            expect(listBox.getListBoxModel() == model.get());
            expect(model->getNumRows() == 1);
        }

        beginTest("Row Selection");
        {
            auto model = std::make_unique<MockListBoxModel>();
            model->addItem("Item 1");
            model->addItem("Item 2");

            DraggableListBox listBox("Test", model.get());
            listBox.setModel(model.get());
            listBox.selectRow(1);

            expect(listBox.getSelectedRow() == 1);
        }

        beginTest("Update Content");
        {
            auto model = std::make_unique<MockListBoxModel>();
            DraggableListBox listBox("Test", model.get());
            listBox.setModel(model.get());

            model->addItem("New Item");
            listBox.updateContent();

            expect(model->getNumRows() == 1);
        }
    }
};

// This creates the static instance that JUCE will use to run the tests
static DraggableListBoxTests draggableListBoxTests;