# ShiftyMapper Plugin for Unreal Engine 5.6

**General-purpose landscape heightmap analysis and map generation plugin**

ShiftyMapper provides tools to generate various analysis maps from landscape heightmaps, including wind flowmaps, normal maps, curvature maps, erosion maps, and more.

## Features

### Current Features (v1.0)
- ✅ **Wind Flowmap Generation**: Generate realistic wind flow patterns from heightmap data
  - Terrain-based wind flow modeling
  - Accounts for slope, aspect, and surface roughness
  - Multiple vegetation types (grass, brush, trees)
  - Configurable wind speed and direction
  - Multi-threaded computation

### Planned Features
- 🔄 **Full WindNinja Integration**: Mass-consistent wind solver for highly accurate simulations
- 📋 **Normal Map Generation**: Calculate surface normals from heightmap
- 📋 **Curvature Analysis**: Compute terrain curvature
- 📋 **Erosion Simulation**: Simulate water erosion patterns
- 📋 **Flow Accumulation**: Calculate water flow paths
- 📋 **Slope & Aspect Maps**: Generate slope and aspect analysis maps

## Installation

1. Copy the `ShiftyMapper` folder to your project's `Plugins` directory
2. If your project doesn't have a `Plugins` folder, create one in the project root
3. Restart Unreal Editor
4. Enable the plugin in Edit → Plugins → Terrain → ShiftyMapper
5. Restart the editor when prompted

## Dependencies & Requirements

**v1.0 (Simplified Model) - Ready to Use:**
- ✅ **No external dependencies required**
- ✅ Works out-of-the-box with Unreal Engine 5.6
- ✅ Uses only built-in Unreal APIs
- ✅ Fast, real-time capable wind simulation

**Future (Full WindNinja Solver) - Optional:**
- GDAL library (for advanced DEM processing)
- To enable: Set `bEnableFullWindNinjaSolver = true` in `ShiftyMapper.Build.cs`
- See `WINDNINJA_INTEGRATION.md` for integration guide

**The current v1.0 release does NOT require GDAL and works immediately after installation.**

## Quick Start - Blueprint

### Generate a Wind Flowmap

1. In your Blueprint, right-click and search for "Generate Wind Flowmap"
2. Connect a Texture2D heightmap input
3. Set wind parameters (speed, direction, vegetation type)
4. The output is a flowmap Texture2D

**Simple Example:**
```
Heightmap Texture → Generate Wind Flowmap → Use Flowmap in Material
                    ├─ Wind Speed: 10.0
                    ├─ Wind Direction: 270.0 (West)
                    └─ Vegetation Type: Trees
```

### Flowmap Output Format

The generated flowmap texture uses the following encoding:
- **R Channel**: X component of wind direction (0-1, remapped from -1 to 1)
- **G Channel**: Y component of wind direction (0-1, remapped from -1 to 1)
- **B Channel**: Wind speed magnitude (0-1, normalized)
- **A Channel**: Always 1.0 (opaque)

## Quick Start - C++

### Include the Plugin Module

In your module's Build.cs file:
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "ShiftyMapper"  // Add this
});
```

### Generate a Flowmap

```cpp
#include "ShiftyMapperBlueprintLibrary.h"

// Generate a wind flowmap
UTexture2D* Heightmap = /* your heightmap */;
bool bSuccess = false;

UTexture2D* Flowmap = UShiftyMapperBlueprintLibrary::GenerateWindFlowmap(
    Heightmap,
    10.0f,      // Wind speed (m/s)
    270.0f,     // Wind direction (degrees from North)
    EShiftyVegetationType::Trees,  // Vegetation type
    6.1f,       // Input wind height (m)
    6.1f,       // Output wind height (m)
    30.0f,      // Cell size (m)
    bSuccess
);

if (bSuccess)
{
    // Use the flowmap
}
```

### Advanced Usage with Parameters

```cpp
#include "ShiftyMapperTypes.h"
#include "ShiftyMapperBlueprintLibrary.h"

// Create custom parameters
FWindFlowmapParams Params;
Params.WindSpeed = 15.0f;
Params.WindDirection = 225.0f;  // Southwest
Params.VegetationType = EShiftyVegetationType::Brush;
Params.CellSize = 50.0f;
Params.NumVerticalLayers = 20;
Params.bUseMultiThreading = true;

// Generate with advanced parameters
FString ErrorMessage;
bool bSuccess = false;

UTexture2D* Flowmap = UShiftyMapperBlueprintLibrary::GenerateWindFlowmapAdvanced(
    Heightmap,
    Params,
    bSuccess,
    ErrorMessage
);

if (!bSuccess)
{
    UE_LOG(LogTemp, Error, TEXT("Flowmap generation failed: %s"), *ErrorMessage);
}
```

## Wind Parameters

### Wind Speed
- Units: meters per second (m/s)
- Typical range: 1-50 m/s
- Examples:
  - Light breeze: 3-5 m/s
  - Moderate wind: 10-15 m/s
  - Strong wind: 20-30 m/s
  - Hurricane-force: 33+ m/s

### Wind Direction
- Units: degrees clockwise from North (meteorological convention)
- Range: 0-360 degrees
- Examples:
  - 0° = North wind (wind FROM the north)
  - 90° = East wind
  - 180° = South wind
  - 270° = West wind

### Vegetation Types
- **Grass**: Short vegetation, low roughness (fields, lawns)
- **Brush**: Medium vegetation (shrublands, bushes)
- **Trees**: Tall vegetation, high roughness (forests)

### Cell Size
- Units: meters
- The real-world size of each heightmap pixel
- Smaller values = finer detail but slower computation
- Typical values: 10-100 meters

## Using Flowmaps in Materials

Flowmaps can be used in materials for various effects:

### Example: Animated Grass/Foliage

```
Material:
  Flowmap Texture (RGB)
  ├─ Unpack: (R, G) → Vector2D
  ├─ Remap from [0,1] to [-1,1]
  ├─ Multiply by Wind Strength
  └─ Use for World Position Offset

B Channel (velocity magnitude):
  └─ Use to modulate animation intensity
```

### Example: Wind Direction Visualization

```
Material:
  Flowmap R & G → Direction Vector
  ├─ Visualize with color gradient
  └─ Or use with noise for turbulence
```

## Architecture & Extensibility

ShiftyMapper is designed to be easily extended with new map generators:

```
IMapGenerator (Interface)
├── FWindFlowmapGenerator (Implemented)
├── FNormalMapGenerator (Future)
├── FCurvatureMapGenerator (Future)
└── ... (Add your own!)
```

### Creating a Custom Map Generator

1. Inherit from `IMapGenerator`
2. Implement required methods:
   - `GetMapType()`
   - `GetGeneratorName()`
   - `GetGeneratorDescription()`
   - `ValidateInput()`
   - `Generate()`
3. Register in `FMapGeneratorFactory::CreateGenerator()`

See `WindFlowmapGenerator.h` for a complete example.

## Performance Considerations

- **Multi-threading**: Enabled by default for faster generation
- **Heightmap Size**: Larger heightmaps take longer to process
  - 512x512: ~0.1-0.5 seconds
  - 1024x1024: ~0.5-2 seconds
  - 2048x2048: ~2-8 seconds
  - 4096x4096: ~8-30 seconds
- **Cell Size**: Smaller cell sizes mean more detail but slower computation

## Current Limitations

### Simplified Wind Model (v1.0)
The current release uses a simplified terrain-based wind flow model that:
- ✅ Accounts for terrain slope and aspect
- ✅ Applies realistic speed-up/slow-down factors
- ✅ Considers surface roughness
- ❌ Does not use full mass-consistent physics solver
- ❌ Does not model complex turbulence

**For production use requiring highly accurate wind simulation**, see [WINDNINJA_INTEGRATION.md](WINDNINJA_INTEGRATION.md) for instructions on integrating the full WindNinja solver.

## Troubleshooting

### Flowmap appears flat/uniform
- Check that your heightmap has actual height variation
- Ensure wind speed is not too low (< 1 m/s)
- Verify cell size matches your terrain scale

### Generation fails with "texture too large" error
- Maximum supported size: 8192x8192
- Try downsampling your heightmap

### Slow generation times
- Enable multi-threading in parameters
- Increase cell size for faster computation
- Use smaller heightmaps for real-time generation

## Credits

This plugin is based on [WindNinja](https://weather.firelab.org/windninja/), developed by the USDA Forest Service Missoula Fire Sciences Laboratory.

### WindNinja License
The WindNinja core algorithms are in the public domain (US Government work).

## Support

- Report issues on GitHub
- Check documentation in the `Docs` folder
- See `WINDNINJA_INTEGRATION.md` for advanced integration

## Version History

### v1.0.0 (Initial Release)
- Terrain-based wind flowmap generation
- Blueprint and C++ API
- Multi-threaded computation
- Three vegetation types
- Extensible architecture for future generators
