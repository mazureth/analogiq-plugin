#include <JuceHeader.h>
#include "DraggableListBox.h"
#include "TestFixture.h"

// TODO: If DraggableListBox or ListBoxModel are abstract, create a concrete test class or mock.

class MockListBoxModel : public juce::ListBoxModel
{
public:
    int getNumRows() override { return items.size(); }
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override 
    {
        // Add some basic painting for testing
        g.fillAll(rowIsSelected ? juce::Colours::blue : juce::Colours::white);
        if (rowNumber < items.size())
        {
            g.setColour(juce::Colours::black);
            g.drawText(items[rowNumber], 0, 0, width, height, juce::Justification::centredLeft);
        }
    }
    
    void addItem(const juce::String &item) { items.add(item); }
    void clear() { items.clear(); }
    void removeItem(int index) { if (index >= 0 && index < items.size()) items.remove(index); }
    juce::String getItem(int index) const { return (index >= 0 && index < items.size()) ? items[index] : ""; }

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
        testConstruction();

        beginTest("Component Properties");
        testComponentProperties();

        beginTest("Model Integration");
        testModelIntegration();

        beginTest("Basic Functionality");
        testBasicFunctionality();

        beginTest("Edge Cases");
        testEdgeCases();
    }

private:
    void testConstruction()
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
        expect(!listBox.isVisible(), "List box should not be visible until added to a parent");
        expect(!listBox.isMouseButtonDown(), "List box should not have mouse button down initially");

        // Test component ID
        expectEquals(listBox.getComponentID(), juce::String("DraggableListBox"), "Component ID should be set correctly");
        
        // Test that keyboard focus is wanted
        expect(listBox.getWantsKeyboardFocus(), "List box should want keyboard focus");
    }

    void testComponentProperties()
    {
        auto model = std::make_unique<MockListBoxModel>();
        DraggableListBox listBox("PropertyTest", model.get());
        
        // Test component properties
        expect(listBox.getWantsKeyboardFocus(), "Should want keyboard focus");
        expectEquals(listBox.getComponentID(), juce::String("DraggableListBox"), "Component ID should be correct");
        expectEquals(listBox.getName(), juce::String("PropertyTest"), "Name should match constructor parameter");
        
        // Test that it's properly configured as a ListBox
        expect(dynamic_cast<juce::ListBox*>(&listBox) != nullptr, "Should be a ListBox");
        
        // Test model assignment
        expect(listBox.getListBoxModel() == model.get(), "Model should be properly assigned");
        
        // Test sizing
        listBox.setSize(200, 100);
        expect(listBox.getWidth() == 200, "Width should be set correctly");
        expect(listBox.getHeight() == 100, "Height should be set correctly");
    }

    void testModelIntegration()
    {
        auto model = std::make_unique<MockListBoxModel>();
        DraggableListBox listBox("ModelTest", model.get());
        
        // Test with empty model
        expectEquals(model->getNumRows(), 0, "Should have 0 rows with empty model");
        
        // Add items and test
        model->addItem("Model Item 1");
        model->addItem("Model Item 2");
        model->addItem("Model Item 3");
        
        expectEquals(model->getNumRows(), 3, "Should have 3 rows after adding items");
        
        // Update content and test selection within bounds
        listBox.updateContent();
        listBox.selectRow(1);
        expectEquals(listBox.getSelectedRow(), 1, "Should select valid row");
        
        // Test selection out of bounds
        listBox.selectRow(10);
        expect(listBox.getSelectedRow() != 10, "Should not select out-of-bounds row");
        
        // Test model changes
        model->removeItem(1);
        expectEquals(model->getNumRows(), 2, "Should have 2 rows after removing item");
        
        // Test getting row containing position
        listBox.setSize(200, 100);
        int row = listBox.getRowContainingPosition(50, 25);
        expect(row >= -1, "Row position should be valid");
        
        // Test invalid positions
        int invalidRow = listBox.getRowContainingPosition(-10, -10);
        expect(invalidRow == -1, "Should return -1 for invalid position");
    }

    void testBasicFunctionality()
    {
        auto model = std::make_unique<MockListBoxModel>();
        model->addItem("Item 1");
        model->addItem("Item 2");
        
        DraggableListBox listBox("FunctionTest", model.get());
        listBox.setSize(200, 100);
        listBox.updateContent();
        
        // Test row selection methods
        listBox.selectRow(0);
        expectEquals(listBox.getSelectedRow(), 0, "Should select first row");
        
        listBox.deselectAllRows();
        expectEquals(listBox.getSelectedRow(), -1, "Should deselect all rows");
        
        // Test row height
        listBox.setRowHeight(25);
        expect(listBox.getRowHeight() == 25, "Row height should be set correctly");
        
        // Test visible rows
        int visibleRows = listBox.getNumRowsOnScreen();
        expect(visibleRows >= 0, "Visible rows should be non-negative");
        
        // Test scrolling
        listBox.scrollToEnsureRowIsOnscreen(0);
        // No assertion here as this is just testing it doesn't crash
    }

    void testEdgeCases()
    {
        auto model = std::make_unique<MockListBoxModel>();
        DraggableListBox listBox("EdgeTest", model.get());
        
        // Test with no model (nullptr)
        DraggableListBox emptyListBox("Empty", nullptr);
        // Test that it doesn't crash with null model
        emptyListBox.setSize(100, 50);
        
        // Test with empty model
        expectEquals(listBox.getSelectedRow(), -1, "Should have no selection with empty model");
        
        // Test invalid selections
        listBox.selectRow(-1);
        expectEquals(listBox.getSelectedRow(), -1, "Should handle invalid negative row");
        
        listBox.selectRow(999);
        expectEquals(listBox.getSelectedRow(), -1, "Should handle invalid large row");
        
        // Test resize with no model
        emptyListBox.setSize(0, 0);
        expect(emptyListBox.getWidth() == 0, "Should handle zero width");
        expect(emptyListBox.getHeight() == 0, "Should handle zero height");
        
        // Test getting row position with no items
        int emptyRow = listBox.getRowContainingPosition(50, 25);
        expect(emptyRow == -1, "Should return -1 for position when no items");
        
        // Test listbox model methods through the listbox
        expect(listBox.getListBoxModel() == model.get(), "Should return correct model");
        
        // Test updating display
        listBox.updateContent();
        listBox.repaint();
        // These should not crash
    }
};

static DraggableListBoxTests draggableListBoxTests;