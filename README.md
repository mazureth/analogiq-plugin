# AnalogIQ

AnalogIQ is a virtual rack and documentation plugin for DAWs that allows audio engineers to visualize and document their analog gear signal chain without affecting the audio signal.

## Features

- **Virtual Rack System**: Organize and visualize your outboard gear in a virtual rack environment
- **Gear Library**: Browse and add gear from a comprehensive library of analog equipment
- **Drag-and-Drop Interface**: Easily arrange gear in the rack via intuitive drag-and-drop
- **Session Notes**: Document patchbay connections, settings, and other important session details
- **Instance Management**: Save and load gear instances with their control states
- **Modern UI**: Clean, intuitive interface with tabbed navigation and responsive layout

## Building the Plugin

### Requirements

- CMake 3.15+
- C++17 compatible compiler
- JUCE framework (will be downloaded automatically if not present)

### Build Steps

1. Clone the repository:
   ```
   git clone https://github.com/mazureth/analogiq-client.git
   cd analogiq-client
   ```

2. Clone the JUCE framework into the project directory:
   ```
   git clone https://github.com/juce-framework/JUCE.git
   ```

3. Create a build directory and run CMake:
   ```
   mkdir build
   cd build
   cmake ..
   ```

4. Build the plugin:
   ```
   cmake --build .
   ```

5. The built plugins will be in the `build` directory, inside their respective format folders.

## Usage

1. Add the AnalogIQ plugin to a track in your DAW
2. Browse the gear library and drag items into the rack
3. Arrange gear in your preferred signal flow
4. Use the notes section to document critical details about your setup
5. Save your DAW project to preserve your rack configuration and notes

## Project Structure

- `Source/` - All source files
  - `PluginProcessor.*` - Core plugin functionality and state management
  - `PluginEditor.*` - Main UI components and layout
  - `GearLibrary.*` - Gear browser, library management, and filtering
  - `GearItem.*` - Individual gear item representation and controls
  - `Rack.*` - Virtual rack system and instance management
  - `RackSlot.*` - Individual rack slot and gear display
  - `NotesPanel.*` - Session notes functionality
  - `DraggableListBox.*` - Enhanced list box with drag-and-drop support
- `JUCE/` - JUCE framework (submodule)
- `build/` - Build output directory
- `CMakeLists.txt` - CMake build configuration

## Development Status

The project is currently in active development with the following features implemented:
- Core plugin architecture
- Virtual rack system
- Gear library with filtering
- Drag-and-drop interface
- Session notes
- Instance state management

Planned features:
- User-created gear support
- MIDI/OSC integration
- Additional rack formats
- Enhanced control automation

## Contributing

Contributions to AnalogIQ are welcome! Please feel free to submit pull requests, create issues or contact the maintainers.

## License

This project is licensed under the MIT License - see the LICENSE file for details. 