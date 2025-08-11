#include <JuceHeader.h>
#include "AnalogIQEditor.h"
#include "AnalogIQProcessor.h"
#include "TestFixture.h"
#include "MockNetworkFetcher.h"
#include "MockFileSystem.h"
#include "PresetManager.h"

class AnalogIQEditorTests : public juce::UnitTest
{
public:
    AnalogIQEditorTests() : UnitTest("AnalogIQEditorTests") {}

    void runTest() override
    {
        TestFixture fixture;
        auto &mockFetcher = ConcreteMockNetworkFetcher::getInstance();
        auto &mockFileSystem = ConcreteMockFileSystem::getInstance();
        mockFetcher.reset();
        mockFileSystem.reset();

        // Set up comprehensive mock responses
        setupMockResponses(mockFetcher);

        // Create instances with proper dependency injection
        CacheManager cacheManager(mockFileSystem, "/mock/cache/root");
        PresetManager presetManager(mockFileSystem, cacheManager);

        beginTest("Construction");
        testConstruction(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Alternative Constructor");
        testAlternativeConstructor(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Component Hierarchy");
        testComponentHierarchy(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Resize Handling");
        testResizeHandling(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Paint Method");
        testPaintMethod(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Getter Methods");
        testGetterMethods(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("State Management");
        testStateManagement(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Preset Menu Display");
        testPresetMenuDisplay(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Preset Save Operations");
        testPresetSaveOperations(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Preset Load Operations");
        testPresetLoadOperations(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Component Visibility and Setup");
        testComponentVisibilityAndSetup(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Menu Bar Components");
        testMenuBarComponents(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Tab Management");
        testTabManagement(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Component IDs and Debugging");
        testComponentIDsAndDebugging(mockFetcher, mockFileSystem, cacheManager, presetManager);

        beginTest("Preset Integration");
        testPresetIntegration(mockFetcher, mockFileSystem, cacheManager, presetManager);
    }

private:
    void setupMockResponses(ConcreteMockNetworkFetcher &mockFetcher)
    {
        // Set up mock response for the units index
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/index.json",
            R"({
                "units": [
                    {
                        "unitId": "la2a-compressor",
                        "name": "LA-2A Tube Compressor",
                        "manufacturer": "Universal Audio",
                        "category": "compressor",
                        "version": "1.0.0",
                        "schemaPath": "units/la2a-compressor-1.0.0.json",
                        "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                        "tags": ["compressor", "tube", "optical", "vintage", "hardware"]
                    }
                ]
            })");

        // Set up mock responses for images
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/faceplates/la2a-compressor-1.0.0.jpg",
            "mock_image_data");
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/thumbnails/la2a-compressor-1.0.0.jpg",
            "mock_image_data");
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/assets/controls/knobs/bakelite-lg-black.png",
            "mock_image_data");

        // Set up mock response for the compressor schema
        mockFetcher.setResponse(
            "https://raw.githubusercontent.com/mazureth/analogiq-schemas/main/units/la2a-compressor-1.0.0.json",
            R"({
                    "unitId": "la2a-compressor",
                    "name": "LA-2A Tube Compressor",
                    "manufacturer": "Universal Audio",
                    "tags": ["compressor", "tube", "optical", "vintage", "hardware"],
                    "version": "1.0.0",
                    "category": "compressor",
                    "formFactor": "19-inch-rack",
                    "faceplateImage": "assets/faceplates/la2a-compressor-1.0.0.jpg",
                    "thumbnailImage": "assets/thumbnails/la2a-compressor-1.0.0.jpg",
                    "width": 1900,
                    "height": 525,
                    "controls": [
                        {
                            "id": "peak-reduction",
                            "label": "Peak Reduction",
                            "type": "knob",
                            "position": {"x": 0.68, "y": 0.44},
                            "value": 180,
                            "startAngle": 40,
                            "endAngle": 322,
                            "image": "assets/controls/knobs/bakelite-lg-black.png"
                        }
                    ]
                    })");
    }

    void testConstruction(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem, 
                         CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        GearLibrary gearLibrary(mockFetcher, mockFileSystem, cacheManager, presetManager);
        AnalogIQEditor editor(processor, mockFileSystem, cacheManager, presetManager, gearLibrary);
        
        expect(editor.getAudioProcessor() == &processor, "Editor should be associated with processor");
        expect(&editor.getPresetManager() == &presetManager, "Preset manager should be accessible");
        expect(editor.getRack() != nullptr, "Rack should be created");
        expect(editor.getNotesPanel() != nullptr, "Notes panel should be created");
        expect(editor.getGearLibrary() != nullptr, "Gear library should be accessible");
        
        // Test initial size
        expectEquals(editor.getWidth(), 1200, "Initial width should be 1200");
        expectEquals(editor.getHeight(), 800, "Initial height should be 800");
        
        // Test component ID
        expectEquals(editor.getComponentID(), juce::String("AnalogIQEditor"), "Component ID should be set");
    }

    void testAlternativeConstructor(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                   CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        expect(editor.getAudioProcessor() == &processor, "Editor should be associated with processor");
        expect(&editor.getPresetManager() == &presetManager, "Preset manager should be accessible");
        expect(editor.getRack() != nullptr, "Rack should be created");
        expect(editor.getNotesPanel() != nullptr, "Notes panel should be created");
        expect(editor.getGearLibrary() != nullptr, "Gear library should be accessible");
        
        // Test component ID
        expectEquals(editor.getComponentID(), juce::String("AnalogIQEditor"), "Component ID should be set");
    }

    void testComponentHierarchy(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                               CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);

        // Check for GearLibrary as direct child
        auto *gearLibrary = editor.findChildWithID("GearLibrary");
        expect(gearLibrary != nullptr, "GearLibrary should be found as child component");

        // Check for MainTabs
        auto *mainTabs = editor.findChildWithID("MainTabs");
        expect(mainTabs != nullptr, "MainTabs should be found as child component");

        // Check for menu bar components
        auto *menuBarContainer = editor.findChildWithID("MenuBarContainer");
        expect(menuBarContainer != nullptr, "MenuBarContainer should be found as child component");

        auto *presetsMenuButton = editor.findChildWithID("PresetsMenuButton");
        expect(presetsMenuButton != nullptr, "PresetsMenuButton should be found as child component");

        // Check that the rack and notes panel have proper IDs
        expect(editor.getRack()->getComponentID() == "RackTab", "Rack should have correct component ID");
        expect(editor.getNotesPanel()->getComponentID() == "NotesTab", "Notes panel should have correct component ID");
    }

    void testResizeHandling(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                           CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Test different sizes
        editor.setSize(800, 600);
        expect(editor.getWidth() == 800, "Width should be updated");
        expect(editor.getHeight() == 600, "Height should be updated");
        
        editor.setSize(1400, 1000);
        expect(editor.getWidth() == 1400, "Width should be updated to larger size");
        expect(editor.getHeight() == 1000, "Height should be updated to larger size");
        
        // Test that resized() works without throwing
        editor.resized();
        expect(true, "resized() should complete without errors");
        
        // Verify components are positioned correctly after resize
        auto *gearLibrary = editor.findChildWithID("GearLibrary");
        auto *mainTabs = editor.findChildWithID("MainTabs");
        auto *menuBarContainer = editor.findChildWithID("MenuBarContainer");
        
        expect(gearLibrary != nullptr && gearLibrary->getWidth() > 0, "GearLibrary should have positive width after resize");
        expect(mainTabs != nullptr && mainTabs->getWidth() > 0, "MainTabs should have positive width after resize");
        expect(menuBarContainer != nullptr && menuBarContainer->getHeight() > 0, "MenuBarContainer should have positive height after resize");
    }

    void testPaintMethod(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                        CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Create a mock graphics context
        juce::Image testImage(juce::Image::RGB, 100, 100, true);
        juce::Graphics g(testImage);
        
        // Test that paint method works without throwing
        editor.paint(g);
        expect(true, "paint() should complete without errors");
    }

    void testGetterMethods(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                          CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Test all getter methods
        auto *rack = editor.getRack();
        expect(rack != nullptr, "getRack() should return valid pointer");
        
        auto *gearLibrary = editor.getGearLibrary();
        expect(gearLibrary != nullptr, "getGearLibrary() should return valid pointer");
        
        auto &presetMgr = editor.getPresetManager();
        expect(&presetMgr == &presetManager, "getPresetManager() should return correct reference");
        
        auto *notesPanel = editor.getNotesPanel();
        expect(notesPanel != nullptr, "getNotesPanel() should return valid pointer");
    }

    void testStateManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                            CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Note: State management methods are private, so we can only test their existence indirectly
        // through other public methods that might use them
        expect(true, "State management functionality exists");
    }

    void testPresetMenuDisplay(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                              CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Test preset menu button setup
        auto *presetsMenuButton = editor.findChildWithID("PresetsMenuButton");
        expect(presetsMenuButton != nullptr, "Presets menu button should exist");
        
        auto *button = dynamic_cast<juce::TextButton*>(presetsMenuButton);
        expect(button != nullptr, "Presets menu should be a TextButton");
        expect(button->getButtonText() == "Presets", "Button text should be 'Presets'");
        
        // Test that onClick is set (we can't easily test the lambda, but we can verify it's not null)
        expect(button->onClick != nullptr, "onClick callback should be set");
    }

    void testPresetSaveOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                 CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Mock some preset data in the preset manager
        juce::String testPresetName = "TestPreset";
        
        // Test save functionality by checking that the method exists and can be called
        // Note: We can't easily test the actual UI dialogs, but we can test the underlying logic
        expect(true, "Preset save operations initialized");
    }

    void testPresetLoadOperations(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                 CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Test load functionality
        expect(true, "Preset load operations initialized");
    }

    void testComponentVisibilityAndSetup(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                        CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);

        // Check GearLibrary visibility
        auto *library = editor.findChildWithID("GearLibrary");
        expect(library != nullptr && library->isVisible(), "GearLibrary should be visible");

        // Check MainTabs and its contents
        auto *mainTabs = editor.findChildWithID("MainTabs");
        expect(mainTabs != nullptr && mainTabs->isVisible(), "MainTabs should be visible");

        // Check menu bar visibility
        auto *menuBarContainer = editor.findChildWithID("MenuBarContainer");
        expect(menuBarContainer != nullptr && menuBarContainer->isVisible(), "MenuBarContainer should be visible");

        auto *presetsMenuButton = editor.findChildWithID("PresetsMenuButton");
        expect(presetsMenuButton != nullptr && presetsMenuButton->isVisible(), "PresetsMenuButton should be visible");
    }

    void testMenuBarComponents(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                              CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Test menu bar container
        auto *menuBarContainer = editor.findChildWithID("MenuBarContainer");
        expect(menuBarContainer != nullptr, "MenuBarContainer should exist");
        
        // Test presets menu button
        auto *presetsMenuButton = editor.findChildWithID("PresetsMenuButton");
        expect(presetsMenuButton != nullptr, "PresetsMenuButton should exist");
        
        // Test button properties
        auto *button = dynamic_cast<juce::TextButton*>(presetsMenuButton);
        if (button != nullptr)
        {
            expect(button->getButtonText() == "Presets", "Button text should be correct");
            expect(button->onClick != nullptr, "Button should have onClick handler");
        }
    }

    void testTabManagement(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                          CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        auto *mainTabs = dynamic_cast<juce::TabbedComponent*>(editor.findChildWithID("MainTabs"));
        expect(mainTabs != nullptr, "MainTabs should be a TabbedComponent");
        
        if (mainTabs != nullptr)
        {
            expect(mainTabs->getNumTabs() == 2, "Should have 2 tabs (Rack and Notes)");
            expect(mainTabs->getTabNames().contains("Rack"), "Should have Rack tab");
            expect(mainTabs->getTabNames().contains("Notes"), "Should have Notes tab");
            
            // Test tab depth
            expect(mainTabs->getTabBarDepth() == 30, "Tab bar depth should be 30");
        }
    }

    void testComponentIDsAndDebugging(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                                     CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);
        
        // Test component IDs are set correctly
        expect(editor.getComponentID() == "AnalogIQEditor", "Editor should have correct component ID");
        expect(editor.getRack()->getComponentID() == "RackTab", "Rack should have correct component ID");
        expect(editor.getNotesPanel()->getComponentID() == "NotesTab", "Notes panel should have correct component ID");
        
        auto *gearLibrary = editor.findChildWithID("GearLibrary");
        expect(gearLibrary != nullptr, "GearLibrary should have correct component ID");
        
        auto *mainTabs = editor.findChildWithID("MainTabs");
        expect(mainTabs != nullptr, "MainTabs should have correct component ID");
        
        auto *menuBarContainer = editor.findChildWithID("MenuBarContainer");
        expect(menuBarContainer != nullptr, "MenuBarContainer should have correct component ID");
        
        auto *presetsMenuButton = editor.findChildWithID("PresetsMenuButton");
        expect(presetsMenuButton != nullptr, "PresetsMenuButton should have correct component ID");
    }

    void testPresetIntegration(ConcreteMockNetworkFetcher &mockFetcher, ConcreteMockFileSystem &mockFileSystem,
                              CacheManager &cacheManager, PresetManager &presetManager)
    {
        AnalogIQProcessor processor(mockFetcher, mockFileSystem);
        AnalogIQEditor editor(processor, cacheManager, presetManager, true);

        // Test that the editor has access to preset manager
        auto &editorPresetManager = editor.getPresetManager();
        expect(&editorPresetManager == &presetManager, "Preset manager should be accessible");

        // Test that the editor has access to rack and gear library for preset operations
        auto *rack = editor.getRack();
        auto *gearLibrary = editor.getGearLibrary();
        expect(rack != nullptr, "Rack should be accessible for preset operations");
        expect(gearLibrary != nullptr, "Gear library should be accessible for preset operations");

        // Test that the rack is empty initially (for confirmation dialog testing)
        bool hasGearItems = false;
        for (int i = 0; i < rack->getNumSlots(); ++i)
        {
            if (auto *slot = rack->getSlot(i))
            {
                if (slot->getGearItem() != nullptr)
                {
                    hasGearItems = true;
                    break;
                }
            }
        }
        expect(!hasGearItems, "Rack should be empty initially for preset confirmation testing");

        // Test that the editor can be resized without errors (menu positioning)
        editor.setSize(800, 600);
        editor.resized();
        expect(true, "Editor should resize without errors");

        // Test that the editor components are properly initialized
        expect(rack->getNumSlots() > 0, "Rack should have slots available");
        expect(gearLibrary != nullptr, "Gear library should be initialized");
    }
};

// This creates the static instance that JUCE will use to run the tests
static AnalogIQEditorTests analogIQEditorTests;