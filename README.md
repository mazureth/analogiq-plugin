# AnalogIQ

AnalogIQ is a virtual rack and documentation plugin for DAWs that allows audio engineers to visualize and document their analog gear signal chain without affecting the audio signal.

## Features

- **Virtual Rack System**: Organize and visualize your outboard gear in a virtual 500-series and 19" rack environment
- **Gear Library**: Browse and add gear from a comprehensive library of popular analog equipment
  - Hierarchical tree view for easy navigation
  - Search functionality
  - Search by name, manufacturer, category, and tags
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
- **Preset System**: Save and load complete rack configurations
  - Save current rack state as named presets
  - Load presets to restore complete configurations
  - Delete unwanted presets
  - Automatic conflict detection and validation
  - Timestamp tracking for preset management
  - User-friendly preset naming with validation
  - Confirmation dialogs for destructive operations
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

### Preset Management

The preset system allows you to save and load complete rack configurations:

#### Saving Presets
1. Configure your rack with the desired gear and settings
2. Click the "Presets" menu button in the top menu bar
3. Select "Save Preset" from the dropdown menu
4. Enter a name for your preset (names are validated for compatibility)
5. Click "OK" to save the preset

#### Loading Presets
1. Click the "Presets" menu button in the top menu bar
2. Either:
   - Select "Load Preset" and choose from the list of available presets, or
   - Click directly on a preset name in the menu to load it immediately
3. If your rack contains gear items, you'll be prompted to confirm the operation
4. The preset will be loaded, replacing your current rack configuration

#### Managing Presets
- **Delete Preset**: Select "Delete Preset" from the presets menu and choose which preset to remove
- **Preset Names**: Must be 1-255 characters long and cannot contain special characters like `/`, `\`, `:`, `*`, `?`, `"`, `<`, `>`, or `|`
- **Timestamps**: Presets are automatically timestamped for easy identification
- **Conflict Detection**: The system prevents overwriting existing presets without explicit confirmation

## Project Structure

- `Source/` - All source files
  - `AnalogIQProcessor.*` - Core plugin functionality
  - `AnalogIQEditor.*` - Main UI components
  - `GearLibrary.*` - Gear browser and library management

## Automated Releases

This project uses GitHub Actions to automatically build and release the plugin when a new version tag is pushed.

### Creating a Release

1. **Update the version** in `CMakeLists.txt` (optional - will be auto-updated):
   ```bash
   # Edit CMakeLists.txt and change the VERSION line
   project(AnalogIQ VERSION 1.0.0)
   ```

2. **Create and push a new tag**:
   ```bash
   git tag -a v1.0.0 -m "[some message]"
   git push origin v1.0.0
   ```

3. **The GitHub Action will automatically**:
   - Update the version in `CMakeLists.txt` to match the tag
   - Build the plugin for both macOS and Windows
   - Create a GitHub release with downloadable packages
   - Include all plugin formats (VST3, AAX, AU, Standalone)

### Release Artifacts

Each release includes:
- **AnalogIQ-macos-v1.0.0.zip** - macOS package with VST3, AU, AAX, and Standalone
- **AnalogIQ-windows-v1.0.0.zip** - Windows package with VST3, AAX, and Standalone

### Supported Plugin Formats

- **VST3**: Compatible with most modern DAWs (Cubase, Reaper, etc.)
- **AAX**: Pro Tools compatibility
- **Audio Unit (AU)**: Native macOS plugin format
- **Standalone**: Run without a DAW

### Build Configuration

The automated builds use the following CMake options:
- `ANALOGIQ_BUILD_AAX=ON` - Build AAX plugins
- `ANALOGIQ_BUILD_VST3=ON` - Build VST3 plugins  
- `ANALOGIQ_BUILD_AU=ON` (macOS only) - Build Audio Unit plugins
- `ANALOGIQ_BUILD_STANDALONE=ON` - Build standalone application
- `ANALOGIQ_BUILD_TESTS=OFF` - Skip tests for faster builds
- `ANALOGIQ_COPY_PLUGINS=ON` - Copy plugins to artifacts directory