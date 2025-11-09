# ShiftyMapper GPU Compute Implementation Guide

This document explains the GPU-accelerated terrain analysis infrastructure and how to complete the implementation.

## What Has Been Created

### 1. Type Definitions (`ShiftyMapperComputeTypes.h`)

Complete parameter structures for all generators:
- ✅ `FCurvatureParams` - Curvature calculation parameters
- ✅ `FRoughnessParams` - Terrain roughness parameters
- ✅ `FErosionParams` - Hydraulic erosion simulation parameters
- ✅ `FGroundwaterParams` - Groundwater/moisture parameters
- ✅ `FSoilFertilityParams` - Vegetation suitability parameters
- ✅ `FSunlightParams` - Sunlight/insolation parameters
- ✅ `FSnowfallParams` - Snowfall and melt parameters
- ✅ `FComputeMapResult` - Unified result structure

### 2. Landscape Input Utilities (`LandscapeInputUtils.h/cpp`)

Complete utilities for extracting heightmaps:
- ✅ Extract heightmap from Landscape actor
- ✅ Get all landscapes in world
- ✅ Query landscape bounds and cell size
- ✅ Read component heightmap data

### 3. Compute Shader Infrastructure (`ShiftyMapperComputeShaders.h`)

Base classes and shader declarations:
- ✅ `FShiftyMapperComputeShaderBase` - Base shader class
- ✅ `FCurvatureComputeShader` - Curvature shader declaration
- ✅ `FRoughnessComputeShader` - Roughness shader declaration
- ✅ `FErosionComputeShader` - Erosion shader declaration
- ✅ `FSunlightComputeShader` - Sunlight shader declaration
- ✅ `FShiftyMapperComputeHelper` - Static helper functions

### 4. HLSL Compute Shaders (`Shaders/Private/*.usf`)

Complete shader implementations:
- ✅ `CurvatureCompute.usf` - Plan, profile, mean, gaussian curvature
- ✅ `RoughnessCompute.usf` - Standard deviation, range, area ratio methods
- ✅ `ErosionCompute.usf` - Particle-based hydraulic erosion
- ✅ `SunlightCompute.usf` - Shadow-casting and diffuse lighting

### 5. Blueprint API (`ShiftyMapperComputeLibrary.h`)

Complete Blueprint-callable functions:
- ✅ All terrain analysis functions declared
- ✅ Landscape and Texture2D input support
- ✅ Curve utility functions
- ✅ Result structures

## What Needs To Be Implemented

### 1. Shader Registration and Implementation

Create `ShiftyMapperComputeShaders.cpp`:

```cpp
#include "ShiftyMapperComputeShaders.h"
#include "ShaderParameterUtils.h"
#include "RenderGraphUtils.h"

// Register shaders
IMPLEMENT_GLOBAL_SHADER(FCurvatureComputeShader, "/Plugin/ShiftyMapper/Private/CurvatureCompute.usf", "CurvatureCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FRoughnessComputeShader, "/Plugin/ShiftyMapper/Private/RoughnessCompute.usf", "RoughnessCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FErosionComputeShader, "/Plugin/ShiftyMapper/Private/ErosionCompute.usf", "ErosionCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FSunlightComputeShader, "/Plugin/ShiftyMapper/Private/SunlightCompute.usf", "SunlightCS", SF_Compute);

// Implement helper functions
void FShiftyMapperComputeHelper::ComputeCurvature(
    FRHICommandListImmediate& RHICmdList,
    UTexture2D* InputHeightmap,
    UTexture2D* OutputCurvature,
    float CellSize,
    float Scale,
    bool bCalculatePlan,
    bool bCalculateProfile)
{
    // Use FRDGBuilder for render graph
    FRDGBuilder GraphBuilder(RHICmdList);

    // Create shader parameters
    FCurvatureComputeShader::FParameters* Parameters = GraphBuilder.AllocParameters<FCurvatureComputeShader::FParameters>();

    // Set parameters
    Parameters->InputHeightmap = /* Create SRV from InputHeightmap */;
    Parameters->OutputCurvature = /* Create UAV from OutputCurvature */;
    Parameters->TextureSize = FVector2f(InputHeightmap->GetSizeX(), InputHeightmap->GetSizeY());
    Parameters->CellSize = CellSize;
    Parameters->Scale = Scale;
    Parameters->bCalculatePlan = bCalculatePlan ? 1 : 0;
    Parameters->bCalculateProfile = bCalculateProfile ? 1 : 0;

    // Get shader
    TShaderMapRef<FCurvatureComputeShader> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

    // Dispatch shader
    FComputeShaderUtils::AddPass(
        GraphBuilder,
        RDG_EVENT_NAME("ShiftyMapper::Curvature"),
        ComputeShader,
        Parameters,
        FComputeShaderUtils::GetGroupCount(Parameters->TextureSize, 8)
    );

    GraphBuilder.Execute();
}

// Repeat pattern for other compute functions...
```

### 2. Blueprint Library Implementation

Create `ShiftyMapperComputeLibrary.cpp`:

```cpp
#include "ShiftyMapperComputeLibrary.h"
#include "ShiftyMapperComputeShaders.h"
#include "LandscapeInputUtils.h"
#include "RenderingThread.h"

UTexture2D* UShiftyMapperComputeLibrary::GetHeightmapFromLandscape(ALandscape* Landscape, bool& bSuccess)
{
    FString ErrorMessage;
    UTexture2D* Result = nullptr;
    bSuccess = ULandscapeInputUtils::ExtractLandscapeHeightmap(Landscape, Result, ErrorMessage);
    return Result;
}

FComputeMapResult UShiftyMapperComputeLibrary::CalculateCurvature(
    UTexture2D* Heightmap,
    const FCurvatureParams& Params)
{
    FComputeMapResult Result;

    if (!Heightmap)
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("Heightmap is null");
        return Result;
    }

    // Create output texture
    UTexture2D* OutputTexture = UTexture2D::CreateTransient(
        Heightmap->GetSizeX(),
        Heightmap->GetSizeY(),
        PF_FloatRGBA
    );

    if (!OutputTexture)
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("Failed to create output texture");
        return Result;
    }

    // Dispatch compute shader on render thread
    ENQUEUE_RENDER_COMMAND(ComputeCurvature)(
        [Heightmap, OutputTexture, Params](FRHICommandListImmediate& RHICmdList)
        {
            FShiftyMapperComputeHelper::ComputeCurvature(
                RHICmdList,
                Heightmap,
                OutputTexture,
                30.0f, // Cell size - should be passed as parameter
                Params.Scale,
                Params.bCalculatePlanCurvature,
                Params.bCalculateProfileCurvature
            );
        }
    );

    // Wait for completion
    FlushRenderingCommands();

    Result.bSuccess = true;
    Result.OutputTexture = OutputTexture;
    return Result;
}

// Implement all other functions following this pattern...
```

### 3. Additional Shaders Needed

Create these HLSL files in `Shaders/Private/`:

**SlopeCompute.usf**:
```hlsl
// Calculate slope from heightmap
[numthreads(8, 8, 1)]
void SlopeCS(uint3 ThreadId : SV_DispatchThreadID)
{
    // Sample heights, calculate gradient, output slope in degrees
}
```

**AspectCompute.usf**:
```hlsl
// Calculate aspect (slope direction) from heightmap
[numthreads(8, 8, 1)]
void AspectCS(uint3 ThreadId : SV_DispatchThreadID)
{
    // Sample heights, calculate gradient direction, output 0-360 degrees
}
```

**GroundwaterCompute.usf**:
```hlsl
// Calculate groundwater accumulation
[numthreads(8, 8, 1)]
void GroundwaterCS(uint3 ThreadId : SV_DispatchThreadID)
{
    // Use flow accumulation + precipitation + permeability
}
```

**VegetationCompute.usf**:
```hlsl
// Calculate vegetation suitability masks
[numthreads(8, 8, 1)]
void VegetationCS(uint3 ThreadId : SV_DispatchThreadID)
{
    // Evaluate slope, moisture, aspect for grass/shrub/tree
    // Output RGB with suitability for each type
}
```

**SnowfallCompute.usf**:
```hlsl
// Calculate snowfall and melt
[numthreads(8, 8, 1)]
void SnowfallCS(uint3 ThreadId : SV_DispatchThreadID)
{
    // Temperature lapse rate + elevation + sunlight
}
```

### 4. Curve Implementations

Implement default curve creators in `ShiftyMapperComputeLibrary.cpp`:

```cpp
UCurveFloat* UShiftyMapperComputeLibrary::CreateDefaultSedimentCapacityCurve()
{
    UCurveFloat* Curve = NewObject<UCurveFloat>();

    // Sediment capacity increases with slope
    Curve->FloatCurve.AddKey(0.0f, 0.0f);     // Flat = low capacity
    Curve->FloatCurve.AddKey(15.0f, 0.5f);    // Moderate slope
    Curve->FloatCurve.AddKey(30.0f, 1.0f);    // Steep slope
    Curve->FloatCurve.AddKey(60.0f, 2.0f);    // Very steep

    return Curve;
}

UCurveFloat* UShiftyMapperComputeLibrary::CreateDefaultGrassSlopeCurve()
{
    UCurveFloat* Curve = NewObject<UCurveFloat>();

    // Grass prefers gentle slopes
    Curve->FloatCurve.AddKey(0.0f, 1.0f);     // Flat = ideal
    Curve->FloatCurve.AddKey(15.0f, 0.8f);    // Gentle slope = good
    Curve->FloatCurve.AddKey(30.0f, 0.3f);    // Moderate = poor
    Curve->FloatCurve.AddKey(45.0f, 0.0f);    // Steep = unsuitable

    return Curve;
}

// Similar for shrub and tree curves...
```

### 5. Module Startup

Register shader directory in `ShiftyMapperModule.cpp`:

```cpp
void FShiftyMapperModule::StartupModule()
{
    // Existing startup code...

    // Register shader directory
    FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ShiftyMapper"))->GetBaseDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/ShiftyMapper"), PluginShaderDir);

    UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Registered shader directory"));
}
```

## Usage Example

### Blueprint

```
Event BeginPlay
  └─► Get Actor of Class: Landscape
       └─► Calculate Curvature From Landscape
            ├─ Curvature Params:
            │   ├─ Scale: 1.0
            │   ├─ Calculate Plan: true
            │   └─ Calculate Profile: true
            └─► Branch (Success)
                 ├─ True: Use Output Texture
                 │        └─ Set Material Parameter
                 └─ False: Log Error
```

### C++

```cpp
// Get landscape
ALandscape* Landscape = GetLandscapeActor();

// Setup parameters
FCurvatureParams Params;
Params.Scale = 1.0f;
Params.bCalculatePlanCurvature = true;
Params.bCalculateProfileCurvature = true;

// Calculate
FComputeMapResult Result = UShiftyMapperComputeLibrary::CalculateCurvatureFromLandscape(
    Landscape,
    Params
);

if (Result.bSuccess)
{
    // Use Result.OutputTexture
    // R = Plan curvature
    // G = Profile curvature
    // B = Mean curvature
    // A = Gaussian curvature
}
```

## Testing Checklist

- [ ] Shaders compile correctly
- [ ] Textures created with correct formats
- [ ] Compute dispatches succeed
- [ ] Output textures contain valid data
- [ ] Landscape input works correctly
- [ ] Curve parameters affect output correctly
- [ ] Multi-output generators work (erosion, vegetation)
- [ ] Performance is acceptable for target heightmap sizes

## Performance Notes

### Thread Group Size

All shaders use `THREADGROUP_SIZE` = 8, meaning 8x8=64 threads per group.

For a 1024x1024 heightmap:
- Groups needed: ceil(1024/8) x ceil(1024/8) = 128 x 128 = 16,384 groups
- Total threads: 16,384 x 64 = 1,048,576 threads

### Optimization Tips

1. **Use appropriate texture formats**:
   - Heightmap: `PF_R16F` or `PF_G16` (16-bit)
   - Results: `PF_R8` for grayscale, `PF_FloatRGBA` for multi-channel

2. **Batch operations**:
   - Process multiple operations in single render graph pass
   - Reuse intermediate results

3. **Async execution**:
   - Don't block game thread waiting for GPU
   - Use callbacks for completion

4. **LOD support**:
   - Allow downsampling for faster preview
   - Full resolution for final export

## Integration with Editor Utility Widgets

The Blueprint API is designed for easy use in Editor Utility Widgets:

```cpp
UCLASS()
class UTerrainAnalysisWidget : public UEditorUtilityWidget
{
    UPROPERTY(EditAnywhere)
    ALandscape* TargetLandscape;

    UPROPERTY(EditAnywhere)
    FCurvatureParams CurvatureSettings;

    UFUNCTION(BlueprintCallable)
    void GenerateCurvatureMap()
    {
        FComputeMapResult Result = UShiftyMapperComputeLibrary::CalculateCurvatureFromLandscape(
            TargetLandscape,
            CurvatureSettings
        );

        // Display result in widget
    }
};
```

## Next Steps

1. Implement shader registration (`ShiftyMapperComputeShaders.cpp`)
2. Implement Blueprint library functions (`ShiftyMapperComputeLibrary.cpp`)
3. Create remaining HLSL shaders
4. Test with sample landscapes
5. Create example Editor Utility Widget
6. Optimize and profile performance

## Reference Materials

- **Unreal Compute Shaders**: Engine/Source/Runtime/Renderer examples
- **Render Graph**: `RenderGraphUtils.h` for modern RDG approach
- **Global Shaders**: `GlobalShader.h` for shader base classes
- **Texture Operations**: `TextureResource.h` for texture manipulation

## Support

For questions about implementing specific generators or optimizing shaders, refer to:
- Unreal Engine rendering documentation
- Global shader examples in Engine source
- HLSL terrain analysis algorithms literature
