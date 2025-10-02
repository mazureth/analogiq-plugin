# Analog Gear Units

This directory contains versioned JSON descriptions of analog audio gear units.

## Versioning

Each unit file follows semantic versioning (SemVer) in accordance with the schema requirements. The version number is:

1. Included in the filename in the format: `[unit-id]-[version].json` (e.g., `la2a-compressor-1.0.0.json`)
2. Included as a property in the JSON itself (e.g., `"version": "1.0.0"`)

### Semantic Versioning Rules

When updating a unit file, follow these rules for version numbers:

- **MAJOR** (x.0.0): Incompatible changes that might break implementations
- **MINOR** (0.x.0): Backwards-compatible additions or enhancements
- **PATCH** (0.0.x): Backwards-compatible bug fixes or adjustments

### Examples:

- `1.0.0`: Initial release of a unit
- `1.0.1`: Minor fixes to control values or descriptions
- `1.1.0`: Added new controls while maintaining compatibility
- `2.0.0`: Changed control positions or removed controls

## Unit Structure

Each unit file conforms to the schema defined in `../schemas/unit.json`. See the main README for detailed information on the schema structure. 
