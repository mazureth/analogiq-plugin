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
- Doxygen (for building documentation)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/mazureth/analogiq-client.git
   cd analogiq-client
   ```

2. Build the project using one of the provided scripts:
   ```bash
   # Normal build
   ./build.sh
   
   # Clean build (removes build directory first)
   ./clean_build.sh
   ```

   Both scripts will accept flags that will be passed through to the build command

   ```
   # Build with verbose output
   ./build.sh --verbose
   
   # Build with specific CMake flags
   ./build.sh --parallel 4
   ```

3. Run the tests:
   ```bash
   ./run_tests.sh
   ```

4. Open your IDE _after_ you build so JUCE is correctly downloaded. This will help with linter issues.

5. To build the documentation:
   ```bash
   cmake --build build --target docs
   ```
   The documentation will be generated in the `docs/html` directory.

The plugin will be built in the `build` directory. The first build might take longer as it downloads JUCE automatically.

## Building and Viewing Documentation Locally

You can easily build and view the Doxygen-generated documentation using the provided helper script:

```sh
./build_docs.sh
```

This script will:
- Configure the project with CMake
- Build the documentation
- Prompt you to open the generated docs in your browser

The documentation will be generated in `docs/html/index.html`.

## Documentation

The project documentation is automatically generated using Doxygen and is available in two places:

1. **Local Documentation**: After building, you can find the documentation in the `docs/html` directory. Open `index.html` in your web browser to view it.

2. **Online Documentation**: The latest documentation is automatically built and deployed to GitHub Pages whenever changes are pushed to the main branch. You can find it at: https://mazureth.github.io/analogiq-client/

The documentation includes:
- Detailed API documentation
- Class hierarchies and relationships
- Source code cross-references
- Call graphs and dependency diagrams

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