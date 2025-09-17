# Recently Used System Implementation

## Files containing Recently Used functionality:

### Source/Model/CacheManager.cpp
- Line 40: recentlyUsedFilePath initialization
- Line 49: loadRecentlyUsed() call
- Line 536-560: addToRecentlyUsed() method implementation
- Line 562-576: removeFromRecentlyUsed() method implementation  
- Line 578-581: isInRecentlyUsed() method implementation
- Line 583-597: getRecentlyUsed() method implementation
- Line 599-604: clearRecentlyUsed() method implementation
- Line 636-681: loadRecentlyUsed() method implementation
- Line 683-695: saveRecentlyUsed() method implementation
- Line 697-721: addToRecentlyUsedInternal() helper method implementation

### Source/Model/CacheManager.h
- Line 49-54: Recently used management method declarations
- Line 75-78: Recently used storage member variables (favoritesFilePath, recentlyUsedFilePath, recentlyUsed)
- Line 99-101: Recently used helper method declarations (loadRecentlyUsed, saveRecentlyUsed, addToRecentlyUsedInternal)

### Source/View/AnalogIQEditor.cpp
- Line 687-692: Recently used file deletion in `simulateFreshInstall()` method

### Source/View/GearLibraryTree.cpp
- Line 73: `createRecentlyUsedSection()` call in `populateTree()`
- Line 86-136: `createRecentlyUsedSection()` method implementation
- Line 94: `cacheManager.getRecentlyUsed()` call
- Line 97-101: Recently used debug logging
- Line 103-106: Empty recently used items handling
- Line 110-125: Recently used gear items population
- Line 239: `createRecentlyUsedSection()` call in `refreshTree()`
- Line 516-517: Recently used item type handling in `getDisplayText()`
- Line 537-538: Recently used item color in `getItemColour()`
- Line 578: "Remove from Recently Used" context menu option
- Line 592: `cacheManager.removeFromRecentlyUsed()` call in context menu
- Line 634-649: Recently used refresh in `onGearItemAdded()`
- Line 694-705: Recently used refresh in `onPresetLoaded()`

### Source/View/GearLibraryTree.h
- Line 6: Recently used items mention in class description
- Line 28: Recently used items mention in class description
- Line 123: `createRecentlyUsedSection()` method declaration
- Line 136: Recently used section mention in class description
- Line 149: `RecentlyUsed` item type enum value
- Line 479: Recently used item type handling in `itemClicked()`
- Line 487-490: Recently used item type handling in `mightContainSubItems()`

### Source/View/Rack.cpp
- Line 272: Recently used handling comment in `itemDropped()`
- Line 687: `cacheManager.addToRecentlyUsed()` call in `addGearToSlot()`

### Source/View/RackSlot.cpp
- Line 431-434: Recently used code removed from `itemDropped()` (moved to Rack)

### Source/Shared/ICacheManager.h
- Line 9: `MAX_RECENTLY_USED` constant definition
- Line 51-56: Recently used management method declarations
- Line 97-101: Recently used management dummy implementation

---

## Detailed System Analysis

### How the Recently Used System Works

The Recently Used system in AnalogIQ is a comprehensive LRU (Least Recently Used) implementation that tracks gear items when they are dropped into the rack and provides quick access to the last 20 items used. Here's how it works:

#### 1. **Core Data Management (CacheManager)**

The system is built around the `CacheManager` class which handles all persistence and data management:

- **Storage**: Uses a `juce::StringArray` called `recentlyUsed` to store unit IDs in order (most recent at the end)
- **Persistence**: Data is stored in `recently_used.json` in the cache directory
- **Capacity**: Limited to 20 items maximum (`MAX_RECENTLY_USED = 20`)

**Key Methods:**
- `addToRecentlyUsed(unitId)`: Adds an item to the recently used list
- `removeFromRecentlyUsed(unitId)`: Removes a specific item
- `getRecentlyUsed(maxCount)`: Retrieves the most recent items (returns in reverse order for display)
- `isInRecentlyUsed(unitId)`: Checks if an item is in the list
- `clearRecentlyUsed()`: Clears all items

#### 2. **LRU Algorithm Implementation**

The LRU logic is implemented in `addToRecentlyUsedInternal()`:

1. **Remove if exists**: If the item already exists in the list, it's removed first
2. **Add to end**: The item is added to the end of the array (most recent position)
3. **Enforce limit**: If the list exceeds 20 items, the oldest items (at the beginning) are removed

This ensures that:
- Recently used items appear at the end of the array
- Duplicates are moved to the most recent position
- The list never exceeds 20 items
- The most recently used item is always the last in the array

#### 3. **Data Persistence**

**Loading (`loadRecentlyUsed()`):**
- Checks if `recently_used.json` exists in the cache directory
- Reads the JSON file and parses it as an array
- Populates the `recentlyUsed` StringArray with unit IDs
- Handles errors gracefully with extensive logging

**Saving (`saveRecentlyUsed()`):**
- Converts the `recentlyUsed` StringArray to JSON
- Writes the JSON to `recently_used.json`
- Includes comprehensive logging for debugging

#### 4. **UI Integration (GearLibraryTree)**

The Recently Used section is displayed in the gear library tree:

**Creation (`createRecentlyUsedSection()`):**
- Creates a "Recently Used" tree node
- Retrieves recently used items from CacheManager
- For each item, looks up the corresponding GearItem from the library
- Creates tree items for display (or shows "No recently used items" if empty)
- Handles missing gear items gracefully

**Display Order:**
- Items are displayed in reverse order (most recent first) for better UX
- Uses `getRecentlyUsed()` which returns items in reverse chronological order

**Context Menu:**
- Right-clicking on gear items shows a context menu
- Includes "Remove from Recently Used" option
- Calls `cacheManager.removeFromRecentlyUsed()` when selected

#### 5. **Trigger Points**

The system is triggered when gear items are added to the rack:

**Rack Gear Addition:**
- When gear is successfully added to any slot in `Rack::addGearToSlot()`
- Calls `cacheManager.addToRecentlyUsed(gearId)` at line 687
- Captures ALL gear additions regardless of drop method:
  - Direct drops on empty rack
  - Drops on empty slots
  - Drops on occupied slots (which create new slots)
  - Programmatic additions

**UI Refresh:**
- When gear is added to the rack, `GearLibraryTree::onGearItemAdded()` is called
- Triggers `refreshTree()` to update the Recently Used section
- Uses `juce::MessageManager::callAsync()` for thread safety
- Also refreshes when presets are loaded

#### 6. **Error Handling & Resilience**

The system includes comprehensive error handling:

- **Missing Files**: Gracefully handles missing `recently_used.json`
- **Invalid JSON**: Handles malformed JSON files
- **Missing Gear Items**: Logs but doesn't crash when gear items are missing
- **Thread Safety**: Uses proper async calls for UI updates
- **Logging**: Extensive debug logging throughout for troubleshooting

#### 7. **Fresh Install Simulation**

The system supports resetting to a fresh state:

- `AnalogIQEditor::simulateFreshInstall()` deletes `recently_used.json`
- This allows testing the system from a clean state
- Part of the debug functionality for development

#### 8. **Data Flow Summary**

1. **User drops gear into rack** → `Rack::itemDropped()` or `RackSlot::itemDropped()` (delegates to Rack)
2. **Add gear to slot** → `Rack::addGearToSlot()`
3. **Add to recently used** → `CacheManager::addToRecentlyUsed()` (line 687)
4. **Update internal list** → `addToRecentlyUsedInternal()` (LRU logic)
5. **Save to disk** → `saveRecentlyUsed()` → `recently_used.json`
6. **Notify UI** → `GearLibraryTree::onGearItemAdded()`
7. **Refresh display** → `refreshTree()` with manual state preservation → preserves expansion state
8. **Show in tree** → Recently Used section with gear items (folders stay expanded)

#### 9. **Key Features**

- **Automatic Tracking**: No user intervention required
- **LRU Behavior**: Most recently used items appear first
- **Persistent**: Survives application restarts
- **Limited Size**: Never exceeds 20 items
- **Thread Safe**: Proper async UI updates
- **Error Resilient**: Handles missing files and data gracefully
- **Debug Friendly**: Extensive logging for troubleshooting
- **User Control**: Can manually remove items via context menu

This system provides a seamless way for users to quickly access their most recently used gear items, improving workflow efficiency in the AnalogIQ plugin.

#### 10. **Recent Fixes**

**Fix 1: Trigger Point Correction**
- **Problem**: The original implementation only triggered Recently Used when dropping onto empty slots, missing many drop scenarios.
- **Solution**: Moved the Recently Used trigger from `RackSlot::itemDropped()` to `Rack::addGearToSlot()` at line 687.
- **Benefits**: Captures ALL gear additions to the rack, no missed drops regardless of drop method.

**Fix 2: Tree Expansion State Preservation**
- **Problem**: When `refreshTree()` was called, it would collapse all expanded folders, forcing users to manually re-expand sections.
- **Solution**: Added recursive expansion state capture and restoration to `refreshTree()` method (lines 235-307).
- **Implementation**: 
  - **Recursive Capture**: Uses `std::function` lambdas to recursively traverse the entire tree structure
  - **Path-Based Storage**: Stores expansion states using hierarchical paths (e.g., "Categories/Compressors")
  - **Deep Preservation**: Captures expansion state of ALL levels (main sections, sub-categories, and nested items)
  - **Recursive Restoration**: Restores expansion states using the same path-based approach
- **Benefits**: 
  - Preserves expansion state of ALL folders at ALL levels during tree refresh
  - Works for Recently Used, Favorites, and Categories with full depth
  - Handles complex nested tree structures automatically
  - Compatible with all JUCE versions

