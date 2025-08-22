# AnalogIQ Plugin MVC Architectural Rewrite Plan

## 🚨 CRITICAL: THIS DOCUMENT IS A CONTRACT

### **By reading this document, you agree that:**
1. **Functionality preservation is NON-NEGOTIABLE**
2. **No architectural decision can compromise working features**
3. **If functionality is lost, the rewrite has FAILED**
4. **Clean architecture without working features is WORTHLESS**

### **This document will be referenced continuously during development.**
### **Every decision must be validated against these requirements.**
### **No exceptions, no compromises, no "but the architecture..."**

## 📚 DOCUMENTATION STRUCTURE

### **This Document (`REWRITE_PLAN.md`)**
- **High-level strategy** and implementation rules
- **Non-negotiable requirements** and success criteria
- **Phase-by-phase approach** and validation checkpoints
- **Risk mitigation** and process guidelines

### **Implementation Details (`NEW_SYSTEM.md`)**
- **Complete component specifications** with line-by-line functionality analysis
- **MVC architecture mapping** of all 13,673 lines of legacy code
- **Interface definitions** and data flow patterns
- **Integration points** and communication protocols

**For implementation specifics, always refer to `NEW_SYSTEM.md`.**

---

## 🎯 **QUICK REFERENCE GUIDE**

| **Need This?** | **Look Here** |
|----------------|---------------|
| **High-level strategy & rules** | `REWRITE_PLAN.md` (this document) |
| **Component specifications** | `NEW_SYSTEM.md` |
| **Interface definitions** | `NEW_SYSTEM.md` |
| **Implementation details** | `NEW_SYSTEM.md` |
| **File organization** | `NEW_SYSTEM.md` |
| **JUCE integration specifics** | `NEW_SYSTEM.md` |
| **State management patterns** | `NEW_SYSTEM.md` |
| **Testing approach** | `REWRITE_PLAN.md` (high-level) |
| **Git workflow** | `REWRITE_PLAN.md` |
| **Risk mitigation** | `REWRITE_PLAN.md` |

---

## Executive Summary

This document outlines the complete architectural rewrite of the AnalogIQ plugin from the current tightly-coupled architecture to a proper Model-View-Controller (MVC) pattern. The rewrite will maintain all existing functionality while providing robust state persistence, proper separation of concerns, and a maintainable codebase.

## 🚨 NON-NEGOTIABLE REQUIREMENTS

**This rewrite MUST achieve the following - there are NO exceptions:**

### **Functionality Preservation: 100% Required**
- ✅ **ALL existing plugin functionality must work identically**
- ✅ **ALL UI behavior must be preserved exactly**
- ✅ **ALL preset save/load operations must work**
- ✅ **ALL gear library browsing must work**
- ✅ **ALL rack operations must work**
- ✅ **ALL drag and drop must work**
- ✅ **ALL caching and offline functionality must work**

### **No "Shell" or "Framework" Development**
- ❌ **NO stub implementations**
- ❌ **NO placeholder code**
- ❌ **NO "TODO" comments in production code**
- ❌ **NO incomplete functionality**
- ❌ **NO broken dependency injection**

### **Working System at Every Phase**
- ✅ **Each phase must result in a fully functional plugin**
- ✅ **Plugin must be testable in DAW at every phase**
- ✅ **All existing features must work before moving to next phase**
- ✅ **No commits of broken or non-functional code**

## Current Architecture Problems

1. **Tight Coupling**: Editor owns Rack, making data persistence impossible when UI is closed
2. **State Loss**: Plugin state is destroyed when UI closes, violating producer expectations
3. **Architectural Flaws**: No clear separation between data, logic, and presentation
4. **Testing Complexity**: Components cannot be tested in isolation

## Target Architecture: MVC Pattern

**📋 COMPREHENSIVE IMPLEMENTATION DETAILS: See `NEW_SYSTEM.md`**

The complete architectural specification, including all component responsibilities, interfaces, data flow, and implementation details, is documented in `NEW_SYSTEM.md`. This document contains:

- **Complete Model Layer Analysis**: 13,673 lines of legacy functionality transformed into MVC architecture
- **Detailed View Layer Specifications**: Every UI component with exact behavior requirements
- **Comprehensive Controller Layer Design**: Event handling and coordination patterns
- **Shared/Library Layer Components**: Utilities, interfaces, and data structures
- **Integration Points**: How all layers communicate and coordinate

**This plan focuses on the high-level strategy and rules. For implementation specifics, refer to `NEW_SYSTEM.md`.**

### Model (Data & Business Logic)
**Key Components** (see `NEW_SYSTEM.md` for complete details):
- **AnalogIQProcessor**: Core audio processing, state management, instance coordination
- **Rack**: Virtual rack system with gear management and state persistence
- **GearItem**: Individual gear instances with control systems and image handling
- **GearLibrary**: Gear management with search, caching, and remote loading
- **PresetManager**: Preset save/load with complete rack state serialization
- **CacheManager**: Asset caching and file management system
- **FileSystem**: File system abstraction with cross-platform support
- **NetworkFetcher**: HTTP operations for gear data and asset retrieval
- **RackStateListener**: State change notifications and observer pattern

### View (User Interface)
**Key Components** (see `NEW_SYSTEM.md` for complete details):
- **AnalogIQEditor**: Main editor with three-panel layout and drag-and-drop
- **RackComponent**: Visual rack representation with scrollable viewport
- **RackSlot**: Individual slot system with control rendering and interaction
- **GearLibraryComponent**: Gear selection interface with tree view
- **NotesPanel**: Session notes interface for documentation
- **PresetUI**: Preset management dialogs and menus
- **DraggableListBox**: Advanced drag-and-drop interface

### Controller (Event Handling & Coordination)
**Key Components** (see `NEW_SYSTEM.md` for complete details):
- **MainController**: Main application coordination
- **GearLibraryController**: Gear management operations
- **RackController**: Rack operations and state changes
- **RackSlotController**: Individual slot management
- **NotesController**: Session notes management
- **PresetController**: Preset operations and validation

## Complete Rewrite Strategy: Copy and Transform

### **Core Principle: Incremental Progress with Clear Recovery**
- **Copy working business logic first** - don't refactor existing code
- **Implement new MVC structure** with working implementations
- **Allow incremental commits** - mark progress clearly
- **Plan for broken integration states** - this is unavoidable
- **Provide clear recovery paths** - know how to get back to working state

### **Implementation Approach: Copy → Implement → Commit → Integrate → Fix → Commit**

#### **Step 1: Copy Working Logic (Week 1)**
- **Copy all working business logic** from old files into new MVC structure
- **Keep old system completely intact** - don't modify it yet
- **Implement concrete classes** that actually work (not stubs)
- **Test each component** against old system behavior
- **Commit working components** individually as they're completed
- **Result**: New MVC system that works identically to old system

#### **Step 2: Implement Working MVC (Week 2)**
- **Copy working UI logic** from old components
- **Adapt to new MVC interfaces** while preserving behavior
- **Commit working UI components** as they're completed
- **Accept that integration will break** - this is expected and unavoidable
- **Result**: New UI that looks and behaves identically

#### **Step 3: Integration and Validation (Week 3)**
- **Connect MVC layers** with working implementations
- **Commit integration milestones** even if system is temporarily broken
- **Fix all integration issues** - system will be broken during this phase
- **Final commit** only when everything works
- **Result**: Complete working plugin with clean architecture

### **Key Insight: Incremental Progress with Clear Recovery**
- **During development**: System will be broken as we replace tightly coupled components
- **This is unavoidable**: Tight coupling means we can't maintain working state throughout
- **Accept the reality**: Plan for broken state, but provide clear recovery paths
- **Focus on progress**: Track incremental completion, not just final working state

## Component Implementation Strategy

**📋 IMPLEMENTATION DETAILS: See `NEW_SYSTEM.md`**

All specific component implementation details, including:
- **Exact functionality requirements** for each component
- **Interface specifications** and data flow patterns  
- **Integration points** and communication protocols
- **Error handling** and validation requirements

Are comprehensively documented in `NEW_SYSTEM.md`.

**This plan focuses on the high-level strategy. For implementation specifics, refer to `NEW_SYSTEM.md`.**

## Implementation Rules

### **REALITY CHECK: System Will Be Broken During Development**
- ⚠️ **Accept temporary broken state** - this is unavoidable with tight coupling
- ⚠️ **Plan for integration issues** - expect problems when connecting components
- ⚠️ **Budget time for fixing** - integration will take longer than implementation

### **ALLOWED (Realistic for Large Rewrites)**
- ✅ **Incremental commits** - commit working components as they're completed
- ✅ **Integration commits** - commit progress even if system is temporarily broken
- ✅ **Broken integration states** - expected during connection phases
- ✅ **Progress markers** - clear indication of what's working vs. what's broken

### **ABSOLUTELY FORBIDDEN**
- ❌ **Committing stub implementations** - components must work before commit
- ❌ **Placeholder code** - no "TODO" or "will implement later"
- ❌ **Interface-only development** - must have working concrete classes
- ❌ **Final broken result** - system must work when complete

### **MANDATORY REQUIREMENTS**
- ✅ **Copy working logic first** - preserve all functionality
- ✅ **Implement concrete classes** - not just interfaces
- ✅ **Clear progress tracking** - know what's working vs. what's broken
- ✅ **Recovery paths** - know how to get back to working state
- ✅ **Final working system** - no features broken when complete

## Copy and Implement Phases

### **CRITICAL: Each Phase Must Result in Working Functionality**

### Phase 1: Business Logic Copy (Week 1)
1. **Copy working business logic** from all old files
2. **Implement concrete classes** that actually work
3. **Test each component** against old system behavior
4. **Validate functionality** is identical
5. **Result**: New MVC system that works identically to old system

### Phase 2: UI Component Copy (Week 2)
1. **Copy working UI logic** from old components
2. **Adapt to new MVC interfaces** while preserving behavior
3. **Test UI functionality** matches old system exactly
4. **Validate visual appearance** is identical
5. **Result**: New UI that looks and behaves identically

### Phase 3: Integration and Testing (Week 3)
1. **Connect MVC layers** with working implementations
2. **Test full plugin functionality** in DAW
3. **Verify no regression** from old system
4. **Validate performance** is maintained
5. **Result**: Complete working plugin with clean architecture

## Validation Checkpoints

### **During Development (Allow Broken Integration)**
1. **Individual components work** - each piece functions correctly
2. **Business logic preserved** - no functionality lost in components
3. **Tests pass for components** - individual pieces are testable
4. **No compilation errors** - clean builds for each component
5. **Progress is trackable** - clear indication of what's complete

### **Before Each Component Commit (Require Working Component)**
1. **Component builds successfully** - no compilation errors
2. **Component works correctly** - no functionality lost
3. **Component tests pass** - no test failures
4. **Component is self-contained** - doesn't break other parts

### **Before Integration Commits (Allow Broken Integration)**
1. **Components work individually** - no regression in components
2. **Integration progress is clear** - know what's connected vs. what's not
3. **Recovery path exists** - know how to get back to working state
4. **No compilation errors** - system builds even if not functional

### **Before Final Commit (Require Working System)**
1. **System builds successfully** - no compilation errors
2. **System loads in DAW** - no crashes or errors
3. **All functionality works** - no regression from old system
4. **All tests pass** - no test failures
5. **Debug build succeeds** - no hidden compilation issues

### **Human Testing Requirements (Minimize Bottlenecks)**
1. **Automated testing** - run tests before requiring human verification
2. **Component-level testing** - test individual pieces without full DAW testing
3. **Integration testing** - test component connections before full system test
4. **Final validation only** - human testing only required for final working system

## Progress Tracking

### **Component Status Indicators**
- 🟢 **Complete**: Component works identically to old version
- 🟡 **In Progress**: Component being implemented
- 🔴 **Broken**: Component has issues that need fixing
- ⚫ **Not Started**: Component not yet implemented

### **Integration Status Indicators**
- 🟢 **Fully Integrated**: All components work together
- 🟡 **Partially Integrated**: Some components connected, some broken
- 🔴 **Integration Broken**: System doesn't work due to connection issues
- ⚫ **Not Integrated**: Components exist but not connected

### **Commit Strategy**
1. **Component commits**: Commit working components as they're completed
2. **Progress commits**: Commit integration progress even if system is broken
3. **Recovery commits**: Commit fixes that restore functionality
4. **Final commit**: Commit only when everything works

## What This Rewrite IS and IS NOT

### **This IS:**
- ✅ **A complete rewrite** that preserves 100% functionality
- ✅ **A copy and transform** approach that maintains working code
- ✅ **A clean architecture** that actually works
- ✅ **A production-ready system** that users can rely on
- ✅ **A maintainable codebase** with proper separation of concerns

### **This is NOT:**
- ❌ **A "framework" or "shell"** for future development
- ❌ **A "proof of concept"** or "prototype"
- ❌ **An incremental refactor** of tightly coupled code
- ❌ **A "starting point"** for development
- ❌ **A system that loses functionality** during the process

### **Key Difference from Previous Failed Approach:**
- **Previous**: Created interfaces first, lost functionality, ended with broken shell
- **New**: Copy working logic first, implement clean architecture, maintain functionality

## Success Criteria

### **MANDATORY: Functional Requirements (100% Required)**
1. **Plugin loads and runs in DAW** - identical to current version
2. **All presets save and load correctly** - no data loss
3. **Gear library displays and searches** - identical functionality
4. **Rack accepts gear items** - drag and drop works perfectly
5. **All controls respond and save state** - no functionality loss
6. **Offline mode works** - cached assets load correctly
7. **UI looks and behaves identically** - no visual regression

### **MANDATORY: Implementation Requirements**
1. **No stub implementations** - every method must work
2. **No placeholder code** - no "TODO" or "will implement later"
3. **Working concrete classes** - not just interfaces
4. **Continuous functionality** - no broken states during development
5. **Identical behavior** - no changes to user experience

### **MANDATORY: Quality Requirements**
1. **Test coverage** meets defined targets
2. **Code quality** follows modern C++ standards
3. **Architecture** clean and maintainable
4. **Performance** equal to or better than current
5. **No compilation errors** - clean builds throughout

### **MANDATORY: Integration Requirements**
1. **DAW compatibility** maintained - test in multiple DAWs
2. **Build system** unchanged - same build process
3. **Test harness** preserved - all tests pass
4. **Documentation tools** functional

### **ABSOLUTELY FORBIDDEN**
- ❌ **Non-functional plugin at any phase**
- ❌ **Stub implementations in production code**
- ❌ **Broken dependency injection**
- ❌ **Interface conflicts between components**
- ❌ **Test compilation failures**

## File Organization Strategy

**📋 COMPLETE FOLDER STRUCTURE: See `NEW_SYSTEM.md`**

The complete file organization, including all component files, interfaces, and shared utilities, is documented in `NEW_SYSTEM.md`.

**Key Principles:**
- **Model/View/Controller separation** with clear boundaries
- **Keep old files** in `Source/OLD_FILES_BACKUP/` for reference
- **Maintain git history** by moving files rather than deleting
- **Reference existing code** for business logic and UI behavior

## State Management Strategy

**📋 COMPLETE STATE MANAGEMENT DETAILS: See `NEW_SYSTEM.md`**

All state management patterns, data flow, event systems, and persistence mechanisms are comprehensively documented in `NEW_SYSTEM.md`.

**Key Principles:**
- **Observer pattern** for state change notifications
- **Event-driven architecture** with clear data flow
- **Model-driven UI updates** for consistency
- **Background state persistence** for performance

## Testing Strategy

### Test Harness Preservation
- **No changes** to `run_tests.sh`
- **No changes** to CMake test configuration
- **No changes** to coverage tools
- **Maintain** existing test infrastructure

### Test File Migration
- **Remove old tests**: Delete existing test files that rely on legacy architecture
- **Write new tests from scratch**: Create clean tests for new MVC components
- **Reference old tests**: Use `tests/OLD_FILES_BACKUP/` for coverage guidance
- **Mock objects**: Create new mocks for new interfaces
- **Test helpers**: Preserve and adapt where applicable

### Test Strategy Decision
**Decision**: Remove old tests and write new ones from scratch
**Rationale**: 
- Old tests were written for tightly-coupled architecture
- New tests will be cleaner and more focused on behavior
- Avoids maintaining two sets of test logic
- Faster development than refactoring incompatible tests

### Test Coverage Goals
- **Minimum coverage**: 70% across all layers
- **Target coverage**: 
  - **Model layer**: 85% (business logic is most critical)
  - **Controller layer**: 80% (coordination logic)
  - **View layer**: 75% (UI behavior)
  - **Integration**: 70% (component interaction)
- **Coverage philosophy**: Stop when new tests yield diminishing returns
- **Diminishing returns defined**: 
  - **Stop if**: 10 new tests yield less than 2% coverage improvement
  - **Stop if**: 5 new tests yield less than 1% coverage improvement  
  - **Stop if**: 3 new tests yield less than 0.5% coverage improvement
- **Focus on value**: Test critical paths and edge cases, not every trivial method
- **Quality over quantity**: Better to have 70% meaningful coverage than 90% trivial coverage
- **Coverage efficiency**: Aim for at least 0.2% coverage improvement per test on average

## Build System Impact

### CMakeLists.txt Changes
- **Add new source directories** for Model/View/Controller
- **Update include paths** for new structure
- **Maintain** existing build targets
- **Add** new component targets if needed

### Dependencies
- **No new external dependencies**
- **Maintain** existing JUCE integration
- **Preserve** compiler flags and settings
- **Keep** existing build scripts

## Schema Reference Requirements

**📋 SCHEMA INTEGRATION DETAILS: See `NEW_SYSTEM.md`**

All schema validation, compliance requirements, and integration patterns are documented in `NEW_SYSTEM.md`.

**Key Principle:**
- **Maintain schema compatibility** - all changes must validate against existing schemas
- **Local schema references** available in `schemas/` and `units/` folders

## GitHub Actions and CI/CD Impact

### Existing Workflows
- **Current workflows**: Will be modified after migration completion
- **Coverage thresholds**: Will be updated based on actual coverage data from new architecture
- **Build paths**: May need updates for new folder structure
- **Test execution**: Path updates for new test organization

### Post-Migration Updates
- **Coverage thresholds**: Set based on realistic coverage data from new MVC system
- **Build validation**: Ensure new folder structure works with existing workflows
- **Test execution**: Verify new test organization integrates properly
- **Coverage reporting**: Update paths and thresholds based on new architecture

### Implementation Strategy
- **Keep existing workflows** during migration (avoid breaking CI/CD)
- **Update workflows** after migration is complete and stable
- **Set realistic thresholds** based on actual coverage data, not arbitrary targets
- **Test workflow changes** in development branch before merging

## Documentation Generator Impact

### Doxygen Configuration
- **Update** source paths in Doxyfile
- **Maintain** existing documentation structure
- **Add** new component documentation
- **Preserve** existing build scripts

### Documentation Structure
- **Model layer**: API documentation
- **View layer**: UI component documentation
- **Controller layer**: Event handling documentation
- **Integration**: Plugin usage documentation

## Risk Mitigation

### Technical Risks
1. **State corruption**: Implement validation and error recovery
2. **Performance degradation**: Profile and optimize critical paths
3. **UI regression**: Maintain existing behavior through testing
4. **Integration issues**: Test with multiple DAWs
5. **Functionality loss**: **ABSOLUTELY FORBIDDEN**

### Process Risks
1. **Scope creep**: Stick to defined phases
2. **Quality issues**: Maintain test coverage throughout
3. **Timeline delays**: Buffer time for unexpected issues
4. **Knowledge loss**: Document all architectural decisions
5. **Broken functionality**: **IMMEDIATE ROLLBACK REQUIRED**

## AI Implementation Permissions

### .cursorrules Requirements
- **Autonomous rewrite permissions**: Allow AI to make architectural decisions
- **File creation/deletion**: Enable new folder structure creation
- **Code refactoring**: Permit large-scale architectural changes
- **Limited scope**: Restrict operations to project folder only

### Required Permissions
- **File system access**: Create/delete/move files within project
- **Architectural decisions**: Make design choices without user input
- **Code generation**: Implement complete new components
- **Testing**: Run and debug tests during development

### Security Considerations
- **Project isolation**: AI cannot access files outside project directory
- **Version control**: All changes tracked through git
- **Rollback capability**: Changes can be reverted if needed
- **Code review**: All generated code subject to review

## Git Branching Strategy

### Branch Structure
```
main (stable, current architecture)
└── mvc-rewrite (new branch for complete architectural rewrite)
```

### Branch Creation
- **Source**: Create `mvc-rewrite` branch from `main` after ensuring stability
- **Timing**: Branch created after all current work is merged and stable
- **Purpose**: Isolate rewrite work from main development

### Development Workflow
- **All rewrite work** happens directly on `mvc-rewrite` branch
- **Regular commits** for each phase completion
- **Linear development**: Complete each phase before moving to the next
- **No sub-branching**: Keep development on single `mvc-rewrite` branch
- **No direct commits** to `main` during rewrite process

### Integration Strategy
- **Phase completion**: Commit completed phases directly to `mvc-rewrite` branch
- **Testing**: Each phase tested before committing and moving to next phase
- **Linear progression**: Complete rewrite evolves linearly on `mvc-rewrite` branch
- **Final integration**: User manually merges `mvc-rewrite` to `main` after thorough review
- **Review process**: User reviews code, builds, tests UI, tests in DAW before merge
- **Rollback plan**: `main` remains stable throughout process

### Commit Guidelines
- **Descriptive messages**: Clear indication of architectural changes
- **Phase markers**: Commit messages indicate which phase is being implemented
- **Component focus**: Each commit focuses on specific MVC layer or component
- **Testing notes**: Include test results and coverage information

### Merge Strategy
- **Squash merges**: Consolidate phase work into single commits
- **Clean history**: Maintain clear architectural evolution in git history
- **Conflict resolution**: Handle any conflicts with existing `main` changes
- **Documentation**: Update documentation with each major merge

## JUCE Integration Strategy

**📋 JUCE INTEGRATION DETAILS: See `NEW_SYSTEM.md`**

All JUCE usage patterns, module dependencies, and integration strategies are documented in `NEW_SYSTEM.md`.

**Key Principle:**
- **Maintain JUCE 8.0.8 compatibility** - preserve all existing JUCE patterns and build system

## Conclusion

This rewrite will transform the AnalogIQ plugin from a tightly-coupled, state-loss-prone architecture to a robust, maintainable MVC pattern. The implementation will preserve all existing functionality while providing the state persistence and architectural quality that producers expect.

The phased approach minimizes risk while ensuring quality, and the preservation of existing tools and infrastructure ensures continuity throughout the process.

**REMINDER: Functionality preservation is NON-NEGOTIABLE.**
**No architectural decision can compromise working features.**
**If functionality is lost, the rewrite has FAILED.**
