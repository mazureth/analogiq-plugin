#pragma once

#include <JuceHeader.h>
#include "GearItem.h"
#include "DraggableListBox.h"

// Forward declarations
class GearListBoxModel;

class GearLibrary : public juce::Component,
                    public juce::ListBoxModel,
                    public juce::Button::Listener
{
public:
    enum class FilterCategory
    {
        All,
        Type,
        Category
    };

    struct FilterOption {
        juce::String displayName;
        FilterCategory category;
        juce::String value;
    };

    using FilterType = std::pair<FilterCategory, juce::String>;

    GearLibrary();
    ~GearLibrary() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // ListBoxModel methods
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;

    // Button::Listener method
    void buttonClicked(juce::Button* button) override;

    // Load gear library from remote or local cache
    void loadLibraryAsync();
    void loadFiltersAsync();
    void loadGearItemsAsync();
    void saveLibraryAsync();
    void addItem(const juce::String& name, const juce::String& category, const juce::String& description, const juce::String& manufacturer);
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
    juce::Label titleLabel;
    juce::TextEditor searchBox;
    juce::ComboBox filterBox;
    juce::TextButton refreshButton;
    juce::TextButton addUserGearButton;

    juce::Array<GearItem> gearItems;
    juce::Array<FilterOption> filterOptions;
    FilterType currentFilter = {FilterCategory::All, ""};
    juce::String currentSearchText;
    
    bool shouldShowItem(const GearItem& item) const;
    void updateFilteredItems();
    void parseFilterOptions(const juce::String& jsonData);
    void updateFilterBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GearLibrary)
}; 