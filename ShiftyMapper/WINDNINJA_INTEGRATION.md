// Integrating Full WindNinja Solver into ShiftyMapper

This guide explains how to integrate the full WindNinja mass-consistent solver into ShiftyMapper for production-grade, physically accurate wind simulations.

## Current State vs Full WindNinja

### Current Implementation (v1.0 - Simplified Model)
The current ShiftyMapper release uses a simplified terrain-based wind flow approximation:
- ✅ Fast computation (real-time capable)
- ✅ No external dependencies
- ✅ Good for visualization and approximate effects
- ❌ Not physically accurate for complex terrain
- ❌ No mass-conservation guarantee
- ❌ Limited turbulence modeling

### Full WindNinja Solver
The complete WindNinja implementation provides:
- ✅ Mass-consistent wind field (∇·u = 0)
- ✅ Finite element solver with terrain-following mesh
- ✅ Accurate physics-based simulation
- ✅ Validated against field measurements
- ❌ Slower computation (seconds to minutes)
- ❌ Complex dependencies (GDAL, NetCDF, etc.)
- ❌ Higher integration complexity

## Integration Approach

### Option 1: In-Process Integration (Recommended for Production)

Integrate the WindNinja C++ code directly into the plugin.

#### Required Files from WindNinja

Extract these files from `src/ninja/`:

**Core Simulation:**
- `ninja.h`, `ninja.cpp` - Main simulation class
- `ninja_conv.h`, `ninja_conv.cpp` - Coordinate conversions
- `WindNinjaInputs.h`, `WindNinjaInputs.cpp` - Configuration
- `mesh.h`, `mesh.cpp` - 3D mesh generation
- `element.h`, `element.cpp` - Finite elements

**Data Structures:**
- `Elevation.h`, `Elevation.cpp` - DEM handling
- `ascii_grid.h`, `ascii_grid.cpp` - 2D grids (template)
- `Array2D.h`, `Array2D.cpp` - Array container
- `wn_3dArray.h`, `wn_3dArray.cpp` - 3D arrays
- `wn_3dScalarField.h`, `wn_3dScalarField.cpp` - 3D scalar fields
- `wn_3dVectorField.h`, `wn_3dVectorField.cpp` - 3D vector fields

**Initialization:**
- `initialize.h`, `initialize.cpp` - Base initialization
- `domainAverageInitialization.h`, `domainAverageInitialization.cpp` - Simple init
- `initializationFactory.h`, `initializationFactory.cpp` - Factory

**Terrain Analysis:**
- `Slope.h`, `Slope.cpp` - Slope calculation
- `Aspect.h`, `Aspect.cpp` - Aspect calculation

**Support:**
- `SurfProperties.h`, `SurfProperties.cpp` - Surface properties
- `ninjaUnits.h` - Unit conversions
- `ninjaException.h` - Error handling
- `ninjaMathUtility.h` - Math utilities
- `constants.h` - Physical constants

**Solver:**
- `preconditioner.h`, `preconditioner.cpp` - Preconditioner for solver

#### Adaptation Steps

1. **Create Unreal-Compatible Wrapper**
   ```cpp
   // WindNinjaCore/ShiftyWindNinjaBridge.h
   class SHIFTYMAPPER_API FShiftyWindNinjaBridge
   {
   public:
       // Convert Unreal types to WindNinja types
       static bool RunSimulation(
           const FShiftyDoubleGrid& ElevationGrid,
           const FWindFlowmapParams& Params,
           FShiftyFloatGrid& OutVelocity,
           FShiftyFloatGrid& OutAngle,
           FString& OutError
       );

   private:
       // Internal conversion helpers
       static void ConvertToWindNinjaGrid(...);
       static void ConvertFromWindNinjaGrid(...);
   };
   ```

2. **Replace STL with Unreal Types**
   - `std::string` → `FString`
   - `std::vector` → `TArray`
   - `std::map` → `TMap`
   - `std::shared_ptr` → `TSharedPtr`

3. **Handle GDAL Integration**
   ```cpp
   // For in-memory DEM only, minimal GDAL usage
   // Create virtual dataset from Unreal texture data
   GDALDatasetH CreateInMemoryDataset(const FShiftyDoubleGrid& Grid)
   {
       // Use /vsimem/ virtual filesystem
       // No file I/O needed
   }
   ```

4. **Update FWindFlowmapGenerator::RunWindNinjaSimulation()**
   ```cpp
   bool FWindFlowmapGenerator::RunWindNinjaSimulation(...)
   {
       // Replace current implementation with:
       return FShiftyWindNinjaBridge::RunSimulation(
           ElevationGrid,
           Parameters,
           OutVelocityGrid,
           OutAngleGrid,
           OutErrorMessage
       );
   }
   ```

#### Build System Changes

Update `ShiftyMapper.Build.cs`:

```csharp
// Add WindNinja core source files
PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/WindNinjaCore"));

// Add OpenMP for parallel processing
if (Target.Platform == UnrealTargetPlatform.Win64)
{
    PublicDefinitions.Add("_OPENMP=1");
    PrivateDefinitions.Add("_OPENMP=1");

    // Add OpenMP compiler flags
    // Note: This requires modifying the build system
}

// GDAL minimal setup for in-memory only
SetupMinimalGDAL(Target);
```

### Option 2: External Process Integration

Call WindNinja as an external executable.

#### Pros:
- No code integration needed
- Use stock WindNinja builds
- Simpler dependency management

#### Cons:
- Slower (file I/O overhead)
- Requires WindNinja installation
- Not suitable for runtime use

#### Implementation:

```cpp
bool FWindFlowmapGenerator::RunWindNinjaExternal(...)
{
    // 1. Write heightmap to temporary file
    FString TempDEMPath = WriteTempDEM(ElevationGrid);

    // 2. Create WindNinja config file
    FString ConfigPath = CreateWindNinjaConfig(TempDEMPath, Parameters);

    // 3. Run WindNinja_cli
    FString WindNinjaExe = "WindNinja_cli";
    FString Args = FString::Printf(TEXT("\"%s\""), *ConfigPath);

    int32 ReturnCode;
    FString StdOut, StdErr;
    FPlatformProcess::ExecProcess(
        *WindNinjaExe, *Args,
        &ReturnCode, &StdOut, &StdErr
    );

    // 4. Read output grids
    if (ReturnCode == 0)
    {
        ReadWindNinjaOutput(OutVelocityGrid, OutAngleGrid);
        return true;
    }

    return false;
}
```

### Option 3: Hybrid Approach

Provide both simplified and full models:

```cpp
enum class EWindSimulationMode
{
    Simplified,  // Current fast approximation
    Full         // WindNinja mass-consistent solver
};

struct FWindFlowmapParams
{
    // ... existing params ...

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWindSimulationMode SimulationMode = EWindSimulationMode::Simplified;
};
```

## Dependencies

### GDAL (Required)

**Minimal GDAL Setup** (for in-memory only):
1. Build GDAL with minimal drivers
2. Only enable: GeoTIFF, GTiff drivers
3. Disable: All network drivers, external formats

**Build Configuration:**
```bash
cmake -DGDAL_BUILD_OPTIONAL_DRIVERS=OFF \
      -DOGR_BUILD_OPTIONAL_DRIVERS=OFF \
      -DGDAL_USE_GEOTIFF=ON \
      .
```

### Boost (Optional, can be replaced)

WindNinja uses Boost for:
- Date/time handling → Replace with Unreal's `FDateTime`
- Program options → Not needed (we provide our own API)
- Shared pointers → Replace with `TSharedPtr`

### OpenMP (Recommended)

For parallel processing:
- Windows: MSVC `/openmp` flag
- Linux: Link with `libgomp`
- Mac: Link with `libomp`

## Step-by-Step Integration

### Phase 1: Prepare WindNinja Code
1. ✅ Copy required source files to `ShiftyMapper/Source/ShiftyMapper/Private/WindNinjaCore/`
2. ✅ Create adaptation headers that map Unreal types
3. ✅ Remove GUI and CLI dependencies
4. ✅ Replace Boost with Unreal equivalents

### Phase 2: Minimal Integration
1. ✅ Implement `FShiftyWindNinjaBridge`
2. ✅ Handle in-memory DEM (no file I/O)
3. ✅ Configure domain-average initialization only
4. ✅ Test with simple terrain

### Phase 3: Full Integration
1. ✅ Add all initialization methods
2. ✅ Optimize for Unreal (texture formats, etc.)
3. ✅ Add progress callbacks for editor use
4. ✅ Implement caching/async generation

### Phase 4: Polish
1. ✅ Add solver parameter exposure
2. ✅ Create editor utilities
3. ✅ Optimize performance
4. ✅ Add validation tests

## Code Example: Full Integration

```cpp
// ShiftyWindNinjaBridge.cpp
bool FShiftyWindNinjaBridge::RunSimulation(
    const FShiftyDoubleGrid& ElevationGrid,
    const FWindFlowmapParams& Params,
    FShiftyFloatGrid& OutVelocity,
    FShiftyFloatGrid& OutAngle,
    FString& OutError)
{
    try
    {
        // Create ninja instance
        ninja Sim;

        // Set up inputs
        Sim.input.dem = ConvertToElevation(ElevationGrid);
        Sim.input.inputSpeed = Params.WindSpeed;
        Sim.input.inputDirection = Params.WindDirection;
        Sim.input.inputWindHeight = Params.InputWindHeight;
        Sim.input.outputWindHeight = Params.OutputWindHeight;
        Sim.input.vegetation = ConvertVegetationType(Params.VegetationType);
        Sim.input.initializationMethod = WindNinjaInputs::domainAverageInitializationFlag;

        // Set mesh parameters
        Sim.set_meshResolution(Params.CellSize, lengthUnits::meters);
        Sim.set_numVertLayers(Params.NumVerticalLayers);

        // Disable file output
        Sim.input.googOutFlag = false;
        Sim.input.shpOutFlag = false;
        Sim.input.asciiOutFlag = false;
        Sim.input.vtkOutFlag = false;

        // Keep grids in memory
        Sim.keepOutputGridsInMemory(true);

        // Run simulation
        if (!Sim.simulate_wind())
        {
            OutError = TEXT("WindNinja simulation failed");
            return false;
        }

        // Extract results
        ConvertFromAsciiGrid(Sim.VelocityGrid, OutVelocity);
        ConvertFromAsciiGrid(Sim.AngleGrid, OutAngle);

        return true;
    }
    catch (const std::exception& e)
    {
        OutError = FString::Printf(TEXT("Exception: %s"), ANSI_TO_TCHAR(e.what()));
        return false;
    }
}
```

## Testing Integration

Create test cases in `ShiftyMapper/Source/ShiftyMapper/Private/Tests/`:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWindNinjaIntegrationTest,
    "ShiftyMapper.WindNinja.BasicSimulation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FWindNinjaIntegrationTest::RunTest(const FString& Parameters)
{
    // Create simple test heightmap
    FShiftyDoubleGrid TestDEM(100, 100, 0, 0, 30.0, -9999.0);

    // Add a simple ridge
    for (int32 i = 0; i < 100; i++)
    {
        for (int32 j = 0; j < 100; j++)
        {
            TestDEM(i, j) = 100.0 + 50.0 * FMath::Sin(i * 0.1);
        }
    }

    // Run simulation
    FWindFlowmapParams Params;
    Params.WindSpeed = 10.0f;
    Params.WindDirection = 270.0f;

    FShiftyFloatGrid VelGrid, AngGrid;
    FString Error;

    bool bSuccess = FShiftyWindNinjaBridge::RunSimulation(
        TestDEM, Params, VelGrid, AngGrid, Error);

    TestTrue("Simulation completed", bSuccess);
    TestTrue("Velocity grid populated", VelGrid.GetArraySize() > 0);
    TestTrue("Angle grid populated", AngGrid.GetArraySize() > 0);

    return true;
}
```

## Performance Optimization

### Caching
```cpp
class FWindFlowmapCache
{
    // Cache results based on heightmap hash + parameters
    TMap<uint64, TSharedPtr<FShiftyMapResult>> Cache;

    uint64 ComputeHash(const FShiftyDoubleGrid& Grid,
                       const FWindFlowmapParams& Params);
};
```

### Async Generation
```cpp
void FWindFlowmapGenerator::GenerateAsync(
    UTexture2D* Heightmap,
    const FWindFlowmapParams& Params,
    TFunction<void(const FShiftyMapResult&)> Callback)
{
    Async(EAsyncExecution::ThreadPool, [=]()
    {
        FShiftyMapResult Result;
        GenerateWindFlowmap(Heightmap, Params, Result);

        // Call back on game thread
        AsyncTask(ENamedThreads::GameThread, [=]()
        {
            Callback(Result);
        });
    });
}
```

## Validation

Compare results with reference WindNinja:
1. Generate flowmap with ShiftyMapper
2. Generate same scenario with WindNinja CLI
3. Compare velocity/angle grids
4. Compute error metrics (RMSE, MAE, etc.)

## Support & Resources

- **WindNinja Documentation**: https://weather.firelab.org/windninja/
- **WindNinja Source**: https://github.com/firelab/windninja
- **WindNinja Papers**: See technical documentation for algorithm details

## Contact

For integration assistance, contact the plugin maintainers or refer to the WindNinja development team.
