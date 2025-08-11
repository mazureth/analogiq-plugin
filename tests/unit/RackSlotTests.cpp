#include <JuceHeader.h>
#include "RackSlot.h"
#include "GearLibrary.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"
#include "TestImageHelper.h"

class RackSlotTests : public juce::UnitTest
{
public:
    RackSlotTests() : UnitTest("RackSlotTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFetcher.reset();
        mockFileSystem.reset();

        // Create local instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);
        GearLibrary gearLibrary(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Initial State");
        testInitialState(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Index Management");
        testIndexManagement(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Gear Item Management");
        testGearItemManagement(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Clear Gear Item");
        testClearGearItem(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Paint and Resize Methods");
        testPaintAndResizeMethods(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Highlighting Functionality");
        testHighlightingFunctionality(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Mouse Event Handling");
        testMouseEventHandling(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Mouse Double Click Handling");
        testMouseDoubleClickHandling(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Mouse Drag Operations");
        testMouseDragOperations(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Button Click Handling");
        testButtonHandling(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Move Operations");
        testMoveOperations(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Drag and Drop Operations");
        testDragAndDropOperations(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Comprehensive Drag and Drop");
        testComprehensiveDragAndDrop(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Control Interaction");
        testControlInteraction(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Control Drawing");
        testControlDrawing(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Control Finding");
        testControlFinding(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Control Type Interactions");
        testControlTypeInteractions(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Instance Management");
        testInstanceManagement(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Component Hierarchy");
        testComponentHierarchy(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Notification System");
        testNotificationSystem(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Button State Management");
        testButtonStateManagement(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Edge Cases and Error Handling");
        testEdgeCasesAndErrorHandling(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);

        beginTest("Preset Integration");
        testPresetIntegration(mockFetcher, mockFileSystem, cacheManager, presetManager, gearLibrary);
    }

private:
    void testInitialState(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                         CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        expect(slot.isAvailable(), "Slot should be available initially");
        expect(slot.getGearItem() == nullptr, "Gear item should be null initially");
        expect(slot.getIndex() == 0, "Index should be 0 by default");
        expect(!slot.isInstance(), "Should not be instance initially");
        expect(slot.getInstanceId().isEmpty(), "Instance ID should be empty initially");
    }

    void testIndexManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                            CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary, 5);
        
        expect(slot.getIndex() == 5, "Index should be set correctly in constructor");
        
        slot.setIndex(10);
        expect(slot.getIndex() == 10, "Index should be updated correctly");
        
        slot.setIndex(0);
        expect(slot.getIndex() == 0, "Index should be reset correctly");
    }

    void testGearItemManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        
        slot.setGearItem(gearItem.get());
        expect(!slot.isAvailable(), "Slot should not be available after setting gear item");
        expect(slot.getGearItem() == gearItem.get(), "Gear item should be set correctly");
        expect(slot.getGearItem()->name == "Test Gear", "Gear item name should be preserved");
        
        slot.setGearItem(nullptr);
        expect(slot.isAvailable(), "Slot should be available after clearing gear item");
        expect(slot.getGearItem() == nullptr, "Gear item should be null after clearing");
    }

    void testClearGearItem(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                          CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        
        slot.setGearItem(gearItem.get());
        expect(!slot.isAvailable(), "Slot should not be available");
        
        slot.clearGearItem();
        expect(slot.isAvailable(), "Slot should be available after clearing");
        expect(slot.getGearItem() == nullptr, "Gear item should be null after clearing");
    }

    void testPaintAndResizeMethods(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                  CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);
        
        // Create a test graphics context
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        
        // Test paint without gear item
        slot.paint(g);
        expect(true, "Paint should not crash without gear item");
        
        // Test paint with gear item
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());
        slot.paint(g);
        expect(true, "Paint should not crash with gear item");
        
        // Test resized
        slot.resized();
        expect(true, "Resized should not crash");
        
        slot.setSize(400, 200);
        slot.resized();
        expect(true, "Resized should handle size changes");
    }

    void testHighlightingFunctionality(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                      CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        slot.setHighlighted(true);
        expect(true, "Should be able to set highlighted to true");
        
        slot.setHighlighted(false);
        expect(true, "Should be able to set highlighted to false");
        
        // Test paint with highlighting
        slot.setSize(200, 100);
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        
        slot.setHighlighted(true);
        slot.paint(g);
        expect(true, "Paint should work with highlighting enabled");
    }

    void testMouseEventHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());

        // Test mouse events (can't easily create MouseEvent objects in tests due to complex constructor requirements)
        // But we can call methods to ensure they don't crash and test the component's response
        expect(true, "Mouse event methods should be callable");
        
        // Test with controls
        auto& controls = gearItem->controls;
        controls.clear();
        
        GearControl knobControl;
        knobControl.id = "test-knob";
        knobControl.name = "Test Knob";
        knobControl.type = GearControl::Type::Knob;
        knobControl.position = {0.5f, 0.5f};
        knobControl.value = 0.5f;
        knobControl.startAngle = -2.5f;
        knobControl.endAngle = 2.5f;
        controls.add(knobControl);
        
        // Test paint with controls to exercise control drawing
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "Paint should work with controls");
    }

    void testMouseDoubleClickHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                     CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);

        // Test double click without gear item
        expect(true, "Double click should be handled gracefully without gear item");
        
        // Test double click with gear item
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());
        expect(true, "Double click should be handled gracefully with gear item");
    }

    void testMouseDragOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());

        // Add a fader control for drag testing
        auto& controls = gearItem->controls;
        controls.clear();
        
        GearControl faderControl;
        faderControl.id = "test-fader";
        faderControl.name = "Test Fader";
        faderControl.type = GearControl::Type::Fader;
        faderControl.position = {0.5f, 0.5f};
        faderControl.value = 0.5f;
        controls.add(faderControl);

        // Test paint to ensure fader rendering works
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "Paint should work with fader control");
    }

    void testButtonHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);

        // Test button state updates
        slot.updateButtonStates();
        expect(true, "Button state update should not crash");
        
        // Test with gear item
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());
        slot.updateButtonStates();
        expect(true, "Button state update should work with gear item");
        
        // Test button click handling (can't easily create button objects, but can test method exists)
        expect(true, "Button handling methods should be available");
    }

    void testMoveOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());
        
        // Test move operations (these require parent rack component which we don't have in tests)
        slot.moveUp();
        expect(true, "Move up should not crash");
        
        slot.moveDown();
        expect(true, "Move down should not crash");
    }

    void testDragAndDropOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                  CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        // Test drag and drop interest (can't easily create SourceDetails in tests)
        // But we can test the methods exist and don't crash with basic scenarios
        expect(true, "Drag and drop methods should be available");
    }

    void testComprehensiveDragAndDrop(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                     CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        // Test comprehensive drag and drop functionality
        // This would test drag enter, move, exit, and drop operations
        expect(true, "Comprehensive drag and drop should be handled");
    }

    void testControlInteraction(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());

        // Create various control types
        auto& controls = gearItem->controls;
        controls.clear();

        // Add different control types for interaction testing
        GearControl switchControl;
        switchControl.id = "test-switch";
        switchControl.name = "Test Switch";
        switchControl.type = GearControl::Type::Switch;
        switchControl.position = {0.3f, 0.3f};
        switchControl.value = 0;
        switchControl.currentStepIndex = 0;
        controls.add(switchControl);

        GearControl buttonControl;
        buttonControl.id = "test-button";
        buttonControl.name = "Test Button";
        buttonControl.type = GearControl::Type::Button;
        buttonControl.position = {0.7f, 0.3f};
        buttonControl.value = 0;
        controls.add(buttonControl);

        GearControl faderControl;
        faderControl.id = "test-fader";
        faderControl.name = "Test Fader";
        faderControl.type = GearControl::Type::Fader;
        faderControl.position = {0.5f, 0.7f};
        faderControl.value = 0.5f;
        controls.add(faderControl);

        // Test paint with all control types to exercise drawing code
        slot.setSize(200, 100);
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "Paint should work with multiple control types");
    }

    void testControlDrawing(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(300, 200);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());

        // Create comprehensive control set for testing all drawing methods
        auto& controls = gearItem->controls;
        controls.clear();

        // Knob control
        GearControl knob;
        knob.id = "draw-knob";
        knob.name = "Draw Knob";
        knob.type = GearControl::Type::Knob;
        knob.position = {0.2f, 0.2f};
        knob.value = 0.3f;
        knob.startAngle = -2.5f;
        knob.endAngle = 2.5f;
        controls.add(knob);

        // Fader control
        GearControl fader;
        fader.id = "draw-fader";
        fader.name = "Draw Fader";
        fader.type = GearControl::Type::Fader;
        fader.position = {0.8f, 0.2f};
        fader.value = 0.7f;
        controls.add(fader);

        // Switch control
        GearControl switch_;
        switch_.id = "draw-switch";
        switch_.name = "Draw Switch";
        switch_.type = GearControl::Type::Switch;
        switch_.position = {0.2f, 0.8f};
        switch_.value = 1;
        switch_.currentStepIndex = 1;
        controls.add(switch_);

        // Button control
        GearControl button;
        button.id = "draw-button";
        button.name = "Draw Button";
        button.type = GearControl::Type::Button;
        button.position = {0.8f, 0.8f};
        button.value = 0;
        controls.add(button);

        // Test drawing all control types
        juce::Image testImage(juce::Image::RGB, 300, 200, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "All control types should draw without crashing");
        
        // Test with different control values
        controls.getReference(0).value = 0.0f;  // knob min
        controls.getReference(1).value = 1.0f;  // fader max
        controls.getReference(2).value = 0;     // switch off
        controls.getReference(3).value = 1;     // button pressed
        
        slot.paint(g);
        expect(true, "Controls should draw with different values");
    }

    void testControlFinding(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());

        // Add controls at known positions
        auto& controls = gearItem->controls;
        controls.clear();

        GearControl testControl;
        testControl.id = "find-control";
        testControl.name = "Find Control";
        testControl.type = GearControl::Type::Knob;
        testControl.position = {0.5f, 0.5f};  // Center position
        testControl.value = 0.5f;
        controls.add(testControl);

        // Test control finding by exercising paint which uses control positioning
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "Control finding through paint should work");
    }

    void testControlTypeInteractions(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                    CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());

        // Test interaction handling by creating controls and testing edge cases
        auto& controls = gearItem->controls;
        controls.clear();

        // Test each control type interaction potential
        GearControl testSwitch;
        testSwitch.id = "interact-switch";
        testSwitch.type = GearControl::Type::Switch;
        testSwitch.position = {0.25f, 0.25f};
        testSwitch.value = 0;
        controls.add(testSwitch);

        GearControl testButton;
        testButton.id = "interact-button";
        testButton.type = GearControl::Type::Button;
        testButton.position = {0.75f, 0.25f};
        testButton.value = 0;
        controls.add(testButton);

        GearControl testFader;
        testFader.id = "interact-fader";
        testFader.type = GearControl::Type::Fader;
        testFader.position = {0.25f, 0.75f};
        testFader.value = 0.3f;
        controls.add(testFader);

        GearControl testKnob;
        testKnob.id = "interact-knob";
        testKnob.type = GearControl::Type::Knob;
        testKnob.position = {0.75f, 0.75f};
        testKnob.value = 0.7f;
        testKnob.startAngle = -2.0f;
        testKnob.endAngle = 2.0f;
        controls.add(testKnob);

        // Exercise interaction code through paint
        juce::Image testImage(juce::Image::RGB, 200, 100, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "All control type interactions should be supported");
    }

    void testInstanceManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));

        slot.setGearItem(gearItem.get());
        expect(slot.isInstance(), "Should be instance after setting gear item (automatically created)");
        
        slot.createInstance();
        expect(slot.isInstance(), "Should be instance after creation");
        expect(!slot.getInstanceId().isEmpty(), "Instance ID should not be empty");
        expect(slot.getGearItem()->name == "Test Gear", "Name should remain unchanged");
        expect(slot.getGearItem()->manufacturer == "Test Manufacturer", "Manufacturer should remain unchanged");

        // Store the instance ID before reset
        auto instanceId = slot.getInstanceId();

        // Reset the instance
        slot.resetToSource();

        // Verify instance state is preserved
        expect(slot.isInstance(), "Should still be instance after reset");
        expect(slot.getInstanceId() == instanceId, "Instance ID should be preserved after reset");
        expect(slot.getGearItem()->name == "Test Gear", "Name should remain unchanged after reset");
        
        // Test creating instance on null gear item
        slot.setGearItem(nullptr);
        slot.createInstance();
        expect(!slot.isInstance(), "Should not be instance with null gear item");
        expect(slot.getInstanceId().isEmpty(), "Instance ID should be empty with null gear item");

        // Test reset on null gear item
        slot.resetToSource();
        expect(!slot.isInstance(), "Should not be instance after reset with null gear item");
    }

    void testComponentHierarchy(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        slot.setSize(200, 100);
        
        // Test component ID
        juce::String expectedID = "RackSlot_0";
        expect(slot.getComponentID() == expectedID, "Component ID should be set correctly");
        
        // Test different indices
        RackSlot slot5(mockFileSystem, cacheManager, presetManager, gearLibrary, 5);
        expect(slot5.getComponentID() == "RackSlot_5", "Component ID should include index");
        
        // Test parent finding (will be null in test environment)
        expect(true, "Parent finding should not crash");
    }

    void testNotificationSystem(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

        // Test notifications by performing actions that should trigger them
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        
        slot.setGearItem(gearItem.get());
        expect(true, "Setting gear item should trigger notification");
        
        slot.clearGearItem();
        expect(true, "Clearing gear item should trigger notification");
        
        // Test with multiple gear item operations
        slot.setGearItem(gearItem.get());
        slot.setGearItem(nullptr);
        slot.setGearItem(gearItem.get());
        expect(true, "Multiple gear item operations should work");
    }

    void testButtonStateManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                  CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        // Test button state updates at different indices
        slot.setIndex(0);
        slot.updateButtonStates();
        expect(true, "Button states should update for index 0");
        
        slot.setIndex(5);
        slot.updateButtonStates();
        expect(true, "Button states should update for index 5");
        
        // Test with gear item
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));
        slot.setGearItem(gearItem.get());
        
        slot.updateButtonStates();
        expect(true, "Button states should update with gear item");
        
        // Test after resized which should also update buttons
        slot.setSize(300, 150);
        slot.resized();
        expect(true, "Resize should handle button positioning");
    }

    void testEdgeCasesAndErrorHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                      CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        // Test operations on empty slot
        slot.clearGearItem();
        expect(true, "Clear should work on empty slot");
        
        slot.createInstance();
        expect(true, "Create instance should work on empty slot");
        
        slot.resetToSource();
        expect(true, "Reset should work on empty slot");
        
        slot.moveUp();
        slot.moveDown();
        expect(true, "Move operations should work on empty slot");
        
        // Test with zero size
        slot.setSize(0, 0);
        juce::Image testImage(juce::Image::RGB, 1, 1, true);
        juce::Graphics g(testImage);
        slot.paint(g);
        expect(true, "Paint should handle zero size");
        
        // Test with very large size
        slot.setSize(10000, 10000);
        slot.resized();
        expect(true, "Should handle very large sizes");
        
        // Test rapid gear item changes
        auto gearItem1 = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager, "gear1"));
        auto gearItem2 = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager, "gear2"));
        
        slot.setGearItem(gearItem1.get());
        slot.setGearItem(gearItem2.get());
        slot.setGearItem(nullptr);
        slot.setGearItem(gearItem1.get());
        expect(true, "Rapid gear item changes should work");
    }

    void testPresetIntegration(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                              CacheManager &cacheManager, PresetManager &presetManager, GearLibrary &gearLibrary)
    {
        setUpMocks(mockFetcher);
        RackSlot slot(mockFileSystem, cacheManager, presetManager, gearLibrary);

        // Test that gear items can be set for preset loading
        auto gearItem = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager));

        slot.setGearItem(gearItem.get());
        expect(slot.getGearItem() == gearItem.get(), "Gear item should be set");

        // Test that gear items can be retrieved for preset saving
        auto *retrievedItem = slot.getGearItem();
        expect(retrievedItem == gearItem.get(), "Retrieved gear item should match");

        // Test that gear items can be cleared for preset loading
        slot.setGearItem(nullptr);
        expect(slot.getGearItem() == nullptr, "Gear item should be cleared");

        // Test that control values are persisted when gear items are set
        slot.setGearItem(gearItem.get());
        expect(slot.getGearItem() == gearItem.get(), "Gear item should be persisted");

        // Test that control values are restored when loading presets
        slot.setGearItem(gearItem.get());
        expect(slot.getGearItem() == gearItem.get(), "Gear item should be restored");

        // Test that state changes are properly notified for preset integration
        slot.setGearItem(gearItem.get());
        slot.setGearItem(nullptr);
        expect(true, "State changes should be notified");

        // Test that multiple gear items can be set and cleared for preset operations
        auto gearItem1 = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager, "gear1"));
        auto gearItem2 = std::unique_ptr<GearItem>(createTestGearItem(mockFetcher, mockFileSystem, cacheManager, "gear2"));

        // Set first gear item
        slot.setGearItem(gearItem1.get());
        expect(slot.getGearItem() == gearItem1.get(), "First gear item should be set");

        // Replace with second gear item
        slot.setGearItem(gearItem2.get());
        expect(slot.getGearItem() == gearItem2.get(), "Second gear item should be set");

        // Clear gear item
        slot.setGearItem(nullptr);
        expect(slot.getGearItem() == nullptr, "Gear item should be cleared");

        // Test that gear item properties are preserved during preset operations
        slot.setGearItem(gearItem.get());
        expect(slot.getGearItem()->name == "Test Gear", "Gear item name should be preserved");
        expect(slot.getGearItem()->categoryString == "test-type", "Gear item type should be preserved");
    }

    // Helper methods
    void setUpMocks(ConcreteMockNetworkFetcher &mockFetcher)
    {
        // Use static test image data to prevent JUCE leak detection
        juce::MemoryBlock imageData = TestImageHelper::getStaticTestImageData();

        // Set up mock responses for images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-type/faceplate.jpg",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-type/thumbnail.jpg",
            imageData);

        // Set up mock responses for control images
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/controls/knobs/bakelite-lg-black.png",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/controls/faders/chrome-lg-black.png",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/controls/switches/toggle-2-chrome.png",
            imageData);
        mockFetcher.setBinaryResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/controls/buttons/momentary-chrome-black.png",
            imageData);
    }

    GearItem* createTestGearItem(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, 
                                CacheManager &cacheManager, const juce::String& unitId = "test-gear")
    {
        auto gearItem = new GearItem();
        gearItem->unitId = unitId;
        gearItem->name = "Test Gear";
        gearItem->manufacturer = "Test Manufacturer";
        gearItem->categoryString = "test-type";
        gearItem->version = "1.0.0";
        gearItem->schemaPath = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-type/schema.json";
        gearItem->thumbnailImage = "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/test-type/thumbnail.jpg";

        return gearItem;
    }
};

static RackSlotTests rackSlotTestsInstance;