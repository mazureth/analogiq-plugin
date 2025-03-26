# AnalogIQ

AnalogIQ is a virtual rack and documentation plugin for DAWs that allows audio engineers to visualize and document their analog gear signal chain without affecting the audio signal.

## Features

- **Virtual Rack System**: Organize and visualize your outboard gear in a virtual 500-series and 19" rack environment
- **Gear Library**: Browse and add gear from a comprehensive library of popular analog equipment
- **Drag-and-Drop Interface**: Easily arrange gear in the rack via intuitive drag-and-drop
- **Session Notes**: Document patchbay connections, settings, and other important session details
- **User-Created Gear**: Import your own gear images and create custom controls (coming soon)
- **MIDI/OSC Integration**: Control your digital-enabled analog gear directly from the plugin (coming soon)

## Building the Plugin

### Requirements

- CMake 3.15+
- C++17 compatible compiler
- JUCE framework (will be downloaded automatically if not present)

### Build Steps

1. Clone the JUCE framework into the project directory:
   ```
   git clone https://github.com/juce-framework/JUCE.git
   ```

2. Create a build directory and run CMake:
   ```
   mkdir build
   cd build
   cmake ..
   ```

3. Build the plugin:
   ```
   cmake --build .
   ```

4. The built plugins will be in the `build` directory, inside their respective format folders.

## Usage

1. Add the AnalogIQ plugin to a track in your DAW
2. Browse the gear library and drag items into the rack
3. Arrange gear in your preferred signal flow
4. Use the notes section to document critical details about your setup
5. Save your DAW project to preserve your rack configuration and notes

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