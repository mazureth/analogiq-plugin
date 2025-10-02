# Rack System Rewrite Plan

## Overview

This document outlines the complete rewrite of the Rack and RackSlot system to properly implement MVC architecture and solve the fundamental "UI must be open to save" problem.

## CRITICAL REQUIREMENTS

**⚠️ MANDATORY: 100% Functionality Preservation ⚠️**

- **Maintain ALL existing business logic exactly**
- **Maintain ALL existing UI/UX exactly** 
- **User experience must be identical**
- **No exceptions - 100% functionality preservation**
- **Only refactor architecture to fix DAW issues**

This is a **pure architectural refactor** - we're moving data from View to Model layer to fix the "UI must be open to save" problem, but everything else (functionality, UI, UX, business logic, error handling) must remain **exactly the same**.

The user should not notice any difference in behavior - only the underlying architecture changes to support proper state persistence when the UI is closed.

## Current State Analysis

### Problem Statement

The current rack system violates MVC principles by storing data in the View layer, making state persistence dependent on UI availability. This causes data loss when the plugin UI is closed during DAW save operations.

### Current Architecture (WRONG)

```
User Action → Rack (View) → Data stored in View → Lost when UI closes
```

## Analysis of Current Files

### 1. Rack.h - View Component with Data Storage

**Current Structure (WRONG):**
```cpp
class Rack : public juce::Component {
    // DATA STORED IN VIEW LAYER - VIOLATES MVC
    std::vector<std::unique_ptr<RackSlot>> rackSlots;        // View components
    std::vector<std::unique_ptr<GearItem>> gearItemInstances; // Data storage
    juce::ValueTree rackState;                               // State storage
    
    // Methods that store/manage data
    bool addGearToSlot(int slotIndex, const juce::String &gearId);
    void serializeRackToJSON() const;
    bool deserializeRackFromJSON(const juce::String &jsonString);
};
```

**Problems:**
- **Data stored in View layer** - `gearItemInstances` vector stores all gear data
- **State persistence in View** - `rackState` ValueTree lives in View component
- **Serialization in View** - JSON serialization methods in View layer
- **UI dependency** - All data operations require UI component to exist

### 2. Rack.cpp - Data Management in View Layer

**Key Data Storage Patterns:**
```cpp
// Line 137: Data stored in View
std::vector<std::unique_ptr<GearItem>> gearItemInstances;

// Line 144: State stored in View
juce::ValueTree rackState;

// Line 633: Creating gear instances in View
auto gearItem = std::make_unique<GearItem>(gearItemTemplate->createInstance());
gearItemInstances[static_cast<size_t>(slotIndex)] = std::move(gearItem);

// Line 1038: Serialization in View
juce::String Rack::serializeRackToJSON() const {
    // Serializes data from View layer
    for (size_t i = 0; i < rackSlots.size(); ++i) {
        auto gearItem = slot->getGearItem();
        // ... serialize gear data
    }
}
```

**Problems:**
- **All gear data** stored in `gearItemInstances` vector in View
- **State serialization** happens in View layer
- **Data lifecycle** tied to UI component lifecycle
- **No persistence** when UI is destroyed

### 3. RackSlot.h - View Component with Data Storage

**Current Structure (WRONG):**
```cpp
class RackSlot : public juce::Component {
    // DATA STORED IN VIEW LAYER - VIOLATES MVC
    GearItem *gearItem = nullptr;  // Data storage in View
    
    // Methods that manage data
    void setGearItem(GearItem *item);
    GearItem *getGearItem() const;
    void clearGearItem();
};
```

**Problems:**
- **Gear data stored in View** - `gearItem` pointer stores data
- **Data operations in View** - Setting/getting gear happens in View
- **No Model separation** - No abstraction between data and display

### 4. RackController.h - Controller References View

**Current Structure (WRONG):**
```cpp
class RackController {
    Rack &rack;  // References VIEW Rack, not Model!
    
    // All operations go through View
    bool addGearToSlot(int slotIndex, GearItem *gearItem);
    bool removeGearFromSlot(int slotIndex);
};
```

**Problems:**
- **References View Rack** - Controller talks to View, not Model
- **No Model layer** - No separation of data from presentation
- **UI dependency** - All operations require UI to exist

### 5. RackSlotController.h - Controller References View

**Current Structure (WRONG):**
```cpp
class RackSlotController {
    RackSlot &rackSlot;  // References VIEW RackSlot, not Model!
    Rack &parentRack;    // References VIEW Rack, not Model!
    
    // All operations go through View
    bool insertGear(GearItem *gearItem);
    GearItem *removeGear();
};
```

**Problems:**
- **References View components** - No Model layer separation
- **Data operations in View** - All gear management happens in View
- **UI dependency** - Operations fail when UI is closed

## Current Data Flow (WRONG)

```
User Action → RackSlot (View) → Rack (View) → Data stored in View
                ↓
            Data lost when UI closes
```

## Current State Persistence (BROKEN)

```
DAW Save → getStateInformation() → saveInstanceState() → getActiveEditor() → null
                ↓
            Empty state saved - DATA LOST
```

## Rewrite Plan

### Phase 1: Create RackModel (Model Layer)

**New File: Source/Model/RackModel.h**
```cpp
class RackModel {
    // Data storage in Model layer
    struct SlotData {
        juce::String gearId;
        juce::String instanceId;
        juce::Array<GearControl> controls;
        // ... all gear state
    };
    
    juce::Array<SlotData> slots;
    juce::String notesContent;
    
    // Data operations
    void addGearToSlot(int slotIndex, const juce::String& gearId);
    void removeGearFromSlot(int slotIndex);
    void updateControlValue(int slotIndex, int controlIndex, float value);
    
    // State persistence
    juce::ValueTree serializeToValueTree() const;
    void deserializeFromValueTree(const juce::ValueTree& state);
};
```

### Phase 2: Refactor Rack (View Layer)

**New Structure:**
```cpp
class Rack : public juce::Component {
    RackModel* rackModel;  // Reference to Model data
    
    // Display methods only
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // No data storage - only display
};
```

### Phase 3: Refactor RackSlot (View Layer)

**New Structure:**
```cpp
class RackSlot : public juce::Component {
    RackModel* rackModel;  // Reference to Model
    int slotIndex;         // Which slot this displays
    
    // Display methods only
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // No data storage - only display
};
```

### Phase 4: Rewrite Controllers

**New RackController:**
```cpp
class RackController {
    RackModel& rackModel;  // References Model, not View
    
    // Operations on Model data
    bool addGearToSlot(int slotIndex, const juce::String& gearId);
    void updateControlValue(int slotIndex, int controlIndex, float value);
};
```

### Phase 5: Fix State Persistence

**New State Persistence:**
```cpp
void AnalogIQProcessor::saveInstanceState() {
    // Direct access to RackModel - no UI dependency
    auto instanceTree = state.state.getOrCreateChildWithName("instances", undoManager.get());
    
    // Save directly from RackModel
    for (int i = 0; i < rackModel->getSlotCount(); ++i) {
        auto slotData = rackModel->getSlotData(i);
        // Serialize slotData to ValueTree
    }
}
```

## Future Architecture (CORRECT)

### Data Flow
```
User Action → Rack (View) → RackController → RackModel (Model) → Persistent Storage
                ↓
            Display only - no data storage
```

### State Persistence
```
DAW Save → getStateInformation() → saveInstanceState() → RackModel → Data saved
                ↓
            Works regardless of UI state
```

## Implementation Steps

1. **Create RackModel class** (1 day)
2. **Move all data from Rack to RackModel** (1 day)
3. **Refactor Rack to be view-only** (1 day)
4. **Refactor RackSlot to be view-only** (1 day)
5. **Rewrite RackController to use RackModel** (1 day)
6. **Update state persistence to use RackModel** (1 day)
7. **Implement Model-View communication** (1 day)

**Total: 7 days** for complete architectural refactor.

## Implementation Constraints

**⚠️ CRITICAL CONSTRAINTS ⚠️**

- **Phase-by-phase approach** - Implement one phase at a time, get approval before proceeding
- **100% completion required** - No stubs, placeholders, or TODOs allowed
- **Each phase must be fully complete** before moving to the next phase
- **Expect build failures** - Build may fail during transition phases
- **No backward compatibility** - Plugin is pre-release, can break interfaces
- **No testing focus** - Skip testing during implementation, focus on architecture
- **Preserve error handling** - Keep existing validation and recovery methods
- **Maintain integration points** - Keep existing connections to GearLibrary, PresetManager, etc.

**The goal is to fix the DAW state persistence problem while maintaining identical user experience.**

## Benefits

- **Data independence** - Rack data persists regardless of UI state
- **True MVC separation** - Clear separation of concerns
- **Reliable state persistence** - Works when UI is closed
- **Maintainable architecture** - Easier to test and modify
- **Scalable design** - Easy to add new features

## Build System Notes

**⚠️ CRITICAL: Adding New Files to Build System ⚠️**

When adding new files to the project, **ALWAYS** update both CMakeLists.txt files:

1. **Root CMakeLists.txt** - Add `.cpp` files to the main target:
   ```cmake
   target_sources(AnalogIQ PRIVATE
       Source/Model/RackModel.cpp  # Add new .cpp files here
   )
   ```

2. **Source/Model/CMakeLists.txt** - Add both `.h` and `.cpp` files to the library:
   ```cmake
   set(MODEL_SOURCES
       RackModel.h      # Add new .h files here
       RackModel.cpp    # Add new .cpp files here
   )
   ```

**Why this matters:** The build system uses separate CMakeLists.txt files for different components. Missing either update will cause linker errors where the compiler can't find the implementation of new classes.

## File Editing Best Practices

**⚠️ CRITICAL: File Truncation Prevention ⚠️**

The MultiEdit tool can sometimes truncate files to 1 line when encountering errors. To prevent this:

1. **Prefer individual search_replace operations** over MultiEdit when possible
2. **Be extremely precise with string matching** - ensure old_string patterns match exactly (whitespace, line endings, indentation)
3. **Test small changes first** before attempting large refactors
4. **Use git restore** to recover truncated files: `git restore <filename>`
5. **Consider using sed commands** for targeted replacements when MultiEdit fails

**File truncation symptoms:**
- Files suddenly become 1 line long
- Build errors about missing class definitions
- "No such file or directory" errors for existing files

**Recovery process:**
1. `git restore <filename>` to recover the file
2. Use individual search_replace operations instead of MultiEdit
3. Verify file integrity after each edit