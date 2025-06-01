#pragma once

#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearItem.h"
#include "GearLibrary.h"

class Rack : public juce::Component,
             public juce::DragAndDropTarget
{
public:
    Rack();
    ~Rack() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // DragAndDropTarget methods
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    // Swap items between two slots (for button-based reordering)
    void rearrangeGearAsSortableList(int sourceSlotIndex, int targetSlotIndex);

    // Find the nearest slot to a position (for drops from library)
    RackSlot *findNearestSlot(const juce::Point<int> &position);

    // Get the number of slots
    int getNumSlots() const { return slots.size(); }

    // Get a specific slot by index
    RackSlot *getSlot(int index) const { return (index >= 0 && index < slots.size()) ? slots[index] : nullptr; }

    // Set the gear library reference
    void setGearLibrary(GearLibrary *library) { gearLibrary = library; }

    // Schema management
    void fetchSchemaForGearItem(GearItem *item);
    void parseSchema(const juce::String &schemaData, GearItem *item);
    void fetchFaceplateImage(GearItem *item);
    void fetchKnobImage(GearItem *item, int controlIndex);
    void fetchFaderImage(GearItem *item, int controlIndex);
    void fetchSwitchSpriteSheet(GearItem *item, int controlIndex);

    // Instance management
    void createInstance(int slotIndex);
    void resetToSource(int slotIndex);
    bool isInstance(int slotIndex) const;
    juce::String getInstanceId(int slotIndex) const;
    void resetAllInstances();

    // Internal container class for rack slots
    class RackContainer : public juce::Component
    {
    public:
        RackContainer() { setComponentID("RackContainer"); }
        void paint(juce::Graphics &g) override { g.fillAll(juce::Colours::black); }
        Rack *rack = nullptr;
    };

private:
    // Configuration
    int numSlots = 16;
    int slotSpacing = 10;

    // UI Components
    std::unique_ptr<juce::Viewport> rackViewport;
    std::unique_ptr<RackContainer> rackContainer;
    juce::OwnedArray<RackSlot> slots;

    // Reference to the gear library (for drag and drop)
    GearLibrary *gearLibrary = nullptr;

    // Helper method to get slot height based on the contained item
    int getSlotHeight(int slotIndex) const;
    int getDefaultSlotHeight() const { return 150; } // Default height if not overridden

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Rack)
};