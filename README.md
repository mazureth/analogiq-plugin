# AnalogIQ

AnalogIQ is a virtual rack and documentation plugin for DAWs that allows audio engineers to visualize and document their analog gear signal chain without affecting the audio signal.

## Features

- **Virtual Rack System**: Organize and visualize your outboard gear in a virtual 500-series and 19" rack environment
- **Gear Library**: Browse and add gear from a comprehensive library of popular analog equipment
  - Hierarchical tree view for easy navigation
  - Search functionality
  - Filter by type and category
  - Remote gear library with automatic updates
- **Drag-and-Drop Interface**: Easily arrange gear in the rack via intuitive drag-and-drop
- **Gear Controls**: Interactive controls for each piece of gear
  - Knobs with customizable ranges and steps
  - Faders with vertical/horizontal orientation
  - Switches with multiple states
  - Buttons with momentary/latching behavior
- **Session Notes**: Document patchbay connections, settings, and other important session details
- **Instance Management**: Create and manage multiple instances of the same gear
  - Save and load instance states
  - Reset instances to source settings
  - Track instance IDs and relationships
- **Remote Resource Management**: Automatic downloading and caching of gear resources
  - Faceplate images
  - Control images (knobs, faders, switches, buttons)
  - Gear schemas and metadata
- **User-Created Gear**: Import your own gear images and create custom controls (coming soon)
- **MIDI/OSC Integration**: Control your digital-enabled analog gear directly from the plugin (coming soon)

## Building the Plugin

### Requirements

- CMake 3.15+
- C++17 compatible compiler
- JUCE framework (will be downloaded automatically by CMake)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/mazureth/analogiq-client.git
   cd analogiq-client
   ```

2. Create a build directory and run CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

The plugin will be built in the `build` directory. The first build might take longer as it downloads JUCE automatically.

## Usage

1. Add the AnalogIQ plugin to a track in your DAW
2. Browse the gear library and drag items into the rack
3. Arrange gear in your preferred signal flow
4. Interact with gear controls to document settings
5. Use the notes section to document critical details about your setup
6. Save your DAW project to preserve your rack configuration and notes

## Project Structure

- `Source/` - All source files
  - `PluginProcessor.*` - Core plugin functionality
  - `PluginEditor.*` - Main UI components
  - `GearLibrary.*` - Gear browser and library management
  - `GearItem.*` - Individual gear item representation
  - `Rack.*` - Virtual rack system
  - `RackSlot.*` - Individual rack slot
  - `NotesPanel.*` - Session notes functionality
- `Assets/` - Images and other resources
- `CMakeLists.txt` - CMake build configuration

## Contributing

Contributions to AnalogIQ are welcome! Please feel free to submit pull requests, create issues or contact the maintainers.

## License

This project is licensed under the MIT License - see the LICENSE file for details. 