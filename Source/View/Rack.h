/**
 * @file Rack.h
 * @brief Header file for the Rack class which manages a virtual rack of audio gear.
 *
 * This file defines the Rack class which provides a visual interface for managing
 * audio gear items in a virtual rack system. It handles the layout, drag-and-drop
 * functionality, and resource management for gear items and their controls.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../Shared/INetworkFetcher.h"
#include "../Shared/IFileSystem.h"
#include "../Shared/ICacheManager.h"
#include "../Model/PresetManager.h"
#include "../Model/GearLibrary.h"
#include "RackSlot.h"
#include <memory>
#include <vector>

class Rack : public juce::Component,
             public juce::DragAndDropTarget,
             public juce::ComponentListener
{
public:
    // Constructor
    Rack(INetworkFetcher &networkFetcher,
         IFileSystem &fileSystem,
         ICacheManager &cacheManager,
         PresetManager &presetManager,
         GearLibrary &gearLibrary);

    ~Rack() override;

    // JUCE Component overrides
    void paint(juce::Graphics &g) override;
    void resized() override;

    // Drag and Drop
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails &dragSourceDetails) override;

    // Rack management
    void addRackSlot(int slotIndex);
    void insertRackSlot(int slotIndex);
    void removeRackSlot(int slotIndex);
    void clearAllSlots();
    int getSlotCount() const;
    RackSlot *getSlot(int slotIndex) const;

    // Gear management
    bool addGearToSlot(int slotIndex, const juce::String &gearId);
    bool removeGearFromSlot(int slotIndex);
    bool moveGearBetweenSlots(int fromSlot, int toSlot);
    juce::String getGearInSlot(int slotIndex) const;

    // State persistence
    void saveRackState();
    void loadRackState();
    juce::ValueTree getRackState() const;
    void setRackState(const juce::ValueTree &state);

    // Layout management
    void setSlotLayout(int numSlots);
    void setSlotSize(int width, int height);
    void setSlotSpacing(int spacing);

    // Visual customization
    void setBackgroundColor(juce::Colour color);
    void setSlotBackgroundColor(juce::Colour color);
    void setSlotBorderColor(juce::Colour color);
    void setShowSlotNumbers(bool show);
    void setShowGrid(bool show);

    // Event handling
    void addRackStateListener(juce::Component *listener);
    void removeRackStateListener(juce::Component *listener);

    // Utility methods
    bool isSlotOccupied(int slotIndex) const;
    int getFirstEmptySlot() const;
    int getLastOccupiedSlot() const;
    void compactSlots();                    // Remove gaps between occupied slots
    int getSlotHeight(int slotIndex) const; // Get dynamic height for slot
    int getDefaultSlotHeight() const;       // Get default slot height

    // Component listener override
    void componentMovedOrResized(juce::Component &component, bool wasMoved, bool wasResized) override;

private:
    // Member variables
    INetworkFetcher &networkFetcher;
    IFileSystem &fileSystem;
    ICacheManager &cacheManager;
    PresetManager &presetManager;
    GearLibrary &gearLibrary;

    // Rack configuration
    int numSlots;
    int slotWidth;
    int slotHeight;
    int slotSpacing;

    // Visual properties
    juce::Colour backgroundColor;
    juce::Colour slotBackgroundColor;
    juce::Colour slotBorderColor;
    bool showSlotNumbers;
    bool showGrid;

    // Rack slots
    std::vector<std::unique_ptr<RackSlot>> rackSlots;

    // UI components
    std::unique_ptr<juce::Viewport> viewport;
    std::unique_ptr<juce::Component> rackContainer;

    // State management
    juce::ValueTree rackState;
    juce::Array<juce::Component *> stateListeners;

    // Private helper methods
    void initializeRack();
    void layoutSlots();
    void createSlot(int slotIndex);
    void updateSlotPositions();
    void notifyStateChanged();

    // Drag and drop helpers
    int getSlotIndexFromPosition(juce::Point<int> position) const;
    bool canDropGearInSlot(int slotIndex, const juce::String &gearId) const;

    // State persistence helpers
    void serializeRackState();
    void deserializeRackState();

    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Rack)
};
