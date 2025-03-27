#pragma once

#include <JuceHeader.h>
#include "GearItem.h"
#include "DraggableListBox.h"

// Forward declarations
class GearListBoxModel;

class GearLibrary : public juce::Component,
                    public juce::ListBoxModel
{
public:
    GearLibrary();
    ~GearLibrary() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // ListBoxModel methods
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int /*rowNumber*/, bool /*isRowSelected*/, juce::Component* /*existingComponentToUpdate*/) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/) override;

    // Load gear library from remote or local cache
    void loadLibraryAsync();
    void parseGearLibrary(const juce::String& jsonData);
    
    // Access gear items
    GearItem* getGearItem(int index);
    int getGearItemCount() const { return gearItems.size(); }

    // Add mouse event forwarding
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    std::unique_ptr<GearListBoxModel> gearListModel;
    std::unique_ptr<DraggableListBox> gearListBox;
    juce::TextEditor searchBox;
    juce::ComboBox filterBox;
    juce::TextButton refreshButton;
    juce::TextButton addUserGearButton;

    juce::Array<GearItem> gearItems;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearLibrary)
}; 