# ShiftyMapper Usage Examples

This document provides detailed examples of using ShiftyMapper in various scenarios.

## Table of Contents
1. [Blueprint Examples](#blueprint-examples)
2. [C++ Examples](#c-examples)
3. [Material Examples](#material-examples)
4. [Real-World Use Cases](#real-world-use-cases)

---

## Blueprint Examples

### Example 1: Simple Wind Flowmap Generation

**Use Case:** Generate a basic wind flowmap for foliage animation.

```
Event BeginPlay
  └─► Get Landscape Heightmap
       └─► Generate Wind Flowmap
            ├─ Heightmap: [Landscape Heightmap Texture]
            ├─ Wind Speed: 10.0
            ├─ Wind Direction: 270.0 (West)
            ├─ Vegetation Type: Trees
            ├─ Cell Size: 30.0
            └─► Branch (Success)
                 ├─ True: Use Flowmap → Apply to Material
                 └─ False: Print Error
```

### Example 2: Runtime Wind Direction Changes

**Use Case:** Update wind flowmap based on time of day or weather system.

```
Custom Event: Update Wind Direction
  Parameters:
    - New Direction (Float)
    - New Speed (Float)

  └─► Generate Wind Flowmap
       ├─ Heightmap: [Cached Heightmap]
       ├─ Wind Speed: [New Speed]
       ├─ Wind Direction: [New Direction]
       └─► On Complete:
            └─► Update Dynamic Material Instance Parameter
                 └─ Parameter Name: "WindFlowmap"
```

### Example 3: Wind Strength Zones

**Use Case:** Create zones with different wind characteristics.

```
For Each Zone in Wind Zones
  └─► Generate Wind Flowmap Advanced
       ├─ Heightmap: [Zone Heightmap]
       ├─ Parameters:
       │    ├─ Wind Speed: [Zone.WindSpeed]
       │    ├─ Wind Direction: [Zone.WindDirection]
       │    ├─ Vegetation Type: [Zone.VegetationType]
       │    └─ Cell Size: 50.0
       └─► Store in Map
            ├─ Key: [Zone Name]
            └─ Value: [Generated Flowmap]
```

### Example 4: Pre-Compute Multiple Scenarios

**Use Case:** Generate flowmaps for different wind conditions at level load.

```
Event: Pre-Compute Wind Scenarios

Create Array: Wind Scenarios
  - Scenario 1: North Wind (0°, 15 m/s)
  - Scenario 2: East Wind (90°, 10 m/s)
  - Scenario 3: South Wind (180°, 12 m/s)
  - Scenario 4: West Wind (270°, 10 m/s)

For Each Scenario
  └─► Generate Wind Flowmap
       └─► Store in Flowmap Cache
            └─ Key: [Scenario Name]
            └─ Value: [Flowmap Texture]

At Runtime: Blend Between Cached Flowmaps
```

---

## C++ Examples

### Example 1: Procedural Terrain Wind

```cpp
#include "ShiftyMapperBlueprintLibrary.h"
#include "ProceduralMeshComponent.h"

class AProceduralTerrain : public AActor
{
public:
    UPROPERTY(EditAnywhere, Category = "Wind")
    float WindSpeed = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Wind")
    float WindDirection = 270.0f;

    void GenerateTerrainWind()
    {
        // Get heightmap from procedural mesh
        UTexture2D* Heightmap = CreateHeightmapFromMesh(ProceduralMesh);

        // Generate flowmap
        bool bSuccess;
        UTexture2D* Flowmap = UShiftyMapperBlueprintLibrary::GenerateWindFlowmap(
            Heightmap,
            WindSpeed,
            WindDirection,
            EShiftyVegetationType::Trees,
            6.1f,
            6.1f,
            30.0f,
            bSuccess
        );

        if (bSuccess)
        {
            // Apply to dynamic material
            UMaterialInstanceDynamic* DynMat = CreateDynamicMaterialInstance();
            DynMat->SetTextureParameterValue(FName("WindFlowmap"), Flowmap);
            ProceduralMesh->SetMaterial(0, DynMat);
        }
    }

private:
    UPROPERTY()
    UProceduralMeshComponent* ProceduralMesh;

    UTexture2D* CreateHeightmapFromMesh(UProceduralMeshComponent* Mesh);
};
```

### Example 2: Async Flowmap Generation

```cpp
#include "Async/Async.h"

void AMyTerrainManager::GenerateFlowmapAsync(UTexture2D* Heightmap)
{
    // Show loading indicator
    ShowLoadingIndicator();

    // Run generation on background thread
    Async(EAsyncExecution::ThreadPool, [this, Heightmap]()
    {
        FWindFlowmapParams Params;
        Params.WindSpeed = 15.0f;
        Params.WindDirection = 225.0f;
        Params.VegetationType = EShiftyVegetationType::Brush;

        bool bSuccess;
        FString ErrorMsg;

        UTexture2D* Flowmap = UShiftyMapperBlueprintLibrary::GenerateWindFlowmapAdvanced(
            Heightmap,
            Params,
            bSuccess,
            ErrorMsg
        );

        // Return to game thread
        AsyncTask(ENamedThreads::GameThread, [this, Flowmap, bSuccess, ErrorMsg]()
        {
            HideLoadingIndicator();

            if (bSuccess)
            {
                OnFlowmapGenerated(Flowmap);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Flowmap generation failed: %s"), *ErrorMsg);
            }
        });
    });
}
```

### Example 3: Custom Map Generator

```cpp
#include "MapGenerators/IMapGenerator.h"

class FSlopeMapGenerator : public IMapGenerator
{
public:
    virtual EShiftyMapType GetMapType() const override
    {
        return EShiftyMapType::SlopeMap;
    }

    virtual FString GetGeneratorName() const override
    {
        return TEXT("Slope Map Generator");
    }

    virtual FString GetGeneratorDescription() const override
    {
        return TEXT("Generates a slope angle map from heightmap");
    }

    virtual bool ValidateInput(UTexture2D* HeightmapTexture, FString& OutErrorMessage) const override
    {
        return FTextureUtils::ValidateHeightmapTexture(HeightmapTexture, OutErrorMessage);
    }

    virtual void Generate(UTexture2D* HeightmapTexture, FShiftyMapResult& OutResult) override
    {
        // Extract heightmap
        FShiftyDoubleGrid ElevationGrid;
        if (!FTextureUtils::ExtractHeightmapData(HeightmapTexture, ElevationGrid, 30.0, OutResult.ErrorMessage))
        {
            return;
        }

        // Compute slope
        FShiftyFloatGrid SlopeGrid = ComputeSlope(ElevationGrid);

        // Convert to texture
        OutResult.OutputTexture = FTextureUtils::CreateGrayscaleTexture(SlopeGrid, OutResult.ErrorMessage);
        OutResult.bSuccess = (OutResult.OutputTexture != nullptr);
    }

private:
    FShiftyFloatGrid ComputeSlope(const FShiftyDoubleGrid& Elevation)
    {
        FShiftyFloatGrid Slope(Elevation.NumCols, Elevation.NumRows,
                               Elevation.XLLCorner, Elevation.YLLCorner,
                               Elevation.CellSize, -9999.0f);

        // Compute slope using central differences
        for (int32 Row = 1; Row < Elevation.NumRows - 1; ++Row)
        {
            for (int32 Col = 1; Col < Elevation.NumCols - 1; ++Col)
            {
                double dZ_dX = (Elevation(Row, Col + 1) - Elevation(Row, Col - 1)) / (2.0 * Elevation.CellSize);
                double dZ_dY = (Elevation(Row - 1, Col) - Elevation(Row + 1, Col)) / (2.0 * Elevation.CellSize);

                double SlopeMag = FMath::Sqrt(dZ_dX * dZ_dX + dZ_dY * dZ_dY);
                Slope(Row, Col) = FMath::RadiansToDegrees(FMath::Atan(SlopeMag));
            }
        }

        return Slope;
    }
};
```

---

## Material Examples

### Example 1: Wind-Driven Foliage

**Material Setup:**
```
Material: M_WindFoliage

Parameters:
  - WindFlowmap (Texture2D)
  - WindStrength (Scalar, default 1.0)
  - WindSpeed (Scalar, default 1.0)

World Position Offset:
  1. Sample WindFlowmap at World Position
  2. Unpack RG channels:
     R → X direction (remap 0-1 to -1 to 1)
     G → Y direction (remap 0-1 to -1 to 1)
  3. Multiply by WindStrength
  4. Add time-based oscillation:
     Offset = FlowDirection * WindStrength * sin(Time * WindSpeed)
  5. Multiply by VertexColor.R (mask effect by height)

Output → World Position Offset
```

**Blueprint Usage:**
```
On Wind Changed:
  └─► Set Material Scalar Parameter: WindStrength
  └─► Set Material Texture Parameter: WindFlowmap
```

### Example 2: Particle Wind Influence

**Niagara System:**
```
User Parameters:
  - Wind Flowmap (Texture2D)

Particle Update:
  1. Sample Flowmap at Particle.Position
  2. Extract velocity (B channel) and direction (RG channels)
  3. Apply force:
     Force = FlowDirection * FlowMagnitude * WindInfluenceStrength
  4. Add to Particle.Velocity
```

### Example 3: Cloud/Fog Movement

**Material: M_VolumetricFog**
```
UV Offset:
  1. Sample WindFlowmap at World Position
  2. Extract direction (RG)
  3. Accumulate over time:
     UVOffset += FlowDirection * Time * FlowSpeed
  4. Use offset UVs for noise sampling

Result: Fog flows according to terrain-influenced wind
```

---

## Real-World Use Cases

### Use Case 1: Forest Fire Simulation

**Scenario:** Visualize how wind affects fire spread in a forested area.

**Implementation:**
```cpp
// 1. Generate wind flowmap for current conditions
FWindFlowmapParams Params;
Params.WindSpeed = GetCurrentWindSpeed();
Params.WindDirection = GetCurrentWindDirection();
Params.VegetationType = EShiftyVegetationType::Trees;

UTexture2D* WindFlowmap = GenerateWindFlowmap(TerrainHeightmap, Params);

// 2. Use flowmap to influence fire spread
for (auto& FireParticle : ActiveFireParticles)
{
    FVector2D FlowDirection = SampleFlowmapDirection(WindFlowmap, FireParticle.Location);
    float FlowSpeed = SampleFlowmapSpeed(WindFlowmap, FireParticle.Location);

    // Fire spreads faster in wind direction
    FireParticle.SpreadRate = BaseSpreadRate * (1.0 + FlowSpeed * 2.0);
    FireParticle.SpreadDirection = FlowDirection;
}
```

### Use Case 2: Agricultural Planning

**Scenario:** Visualize wind patterns for pesticide spray planning.

**Implementation:**
```cpp
// Generate flowmaps for different times of day
TArray<UTexture2D*> DailyWindFlowmaps;

for (int32 Hour = 0; Hour < 24; ++Hour)
{
    FWindFlowmapParams Params;
    Params.WindSpeed = GetWindSpeedAtHour(Hour);
    Params.WindDirection = GetWindDirectionAtHour(Hour);
    Params.VegetationType = EShiftyVegetationType::Grass;

    UTexture2D* Flowmap = GenerateWindFlowmap(FieldHeightmap, Params);
    DailyWindFlowmaps.Add(Flowmap);
}

// Visualize best spray times (low wind, good coverage)
AnalyzeWindPatterns(DailyWindFlowmaps);
```

### Use Case 3: Outdoor Concert/Event Planning

**Scenario:** Plan speaker placement and sound propagation.

**Implementation:**
```cpp
// Generate wind flowmap for event location
UTexture2D* VenueWindFlowmap = GenerateWindFlowmap(
    VenueHeightmap,
    TypicalWindSpeed,
    PrevailingWindDirection
);

// Use for sound propagation simulation
for (auto& Speaker : SpeakerArray)
{
    FVector2D WindAtSpeaker = SampleWindVector(VenueWindFlowmap, Speaker.Location);

    // Adjust sound propagation model
    PropagateSound(Speaker, AudienceArea, WindAtSpeaker);
}
```

### Use Case 4: Flight Simulator Terrain Effects

**Scenario:** Model realistic wind effects near terrain for flight sim.

**Implementation:**
```cpp
// Generate high-resolution wind flowmap
FWindFlowmapParams Params;
Params.CellSize = 10.0f;  // High resolution
Params.WindSpeed = CurrentWindSpeed;
Params.WindDirection = CurrentWindDirection;
Params.NumVerticalLayers = 50;  // For vertical wind variations

UTexture2D* AirflowMap = GenerateWindFlowmap(TerrainHeightmap, Params);

// Apply to aircraft physics
void UpdateAircraftPhysics(AAircraft* Aircraft)
{
    FVector Location = Aircraft->GetActorLocation();

    // Sample wind at aircraft location
    FVector2D LocalWind = SampleFlowmapDirection(AirflowMap, Location);
    float WindSpeed = SampleFlowmapSpeed(AirflowMap, Location);

    // Apply wind force to aircraft
    FVector WindForce = FVector(LocalWind.X, LocalWind.Y, 0) * WindSpeed;
    Aircraft->AddForce(WindForce * AirDensity);
}
```

### Use Case 5: Open World Weather System

**Scenario:** Dynamic weather with location-specific wind patterns.

**Implementation:**
```cpp
class AWeatherManager : public AActor
{
    // Cache flowmaps for different weather conditions
    TMap<FString, UTexture2D*> FlowmapCache;

    void InitializeWeatherSystem()
    {
        // Pre-generate flowmaps for common weather patterns
        GenerateWeatherFlowmaps();
    }

    void GenerateWeatherFlowmaps()
    {
        TArray<FWeatherPattern> Patterns = {
            { "Clear", 5.0f, 270.0f },
            { "Storm", 25.0f, 180.0f },
            { "Hurricane", 50.0f, 90.0f }
        };

        for (const auto& Pattern : Patterns)
        {
            FWindFlowmapParams Params;
            Params.WindSpeed = Pattern.Speed;
            Params.WindDirection = Pattern.Direction;

            UTexture2D* Flowmap = GenerateWindFlowmap(WorldHeightmap, Params);
            FlowmapCache.Add(Pattern.Name, Flowmap);
        }
    }

    void UpdateWeather(const FString& WeatherType, float Intensity)
    {
        // Blend between cached flowmaps
        UTexture2D* FlowmapA = FlowmapCache["Clear"];
        UTexture2D* FlowmapB = FlowmapCache[WeatherType];

        // Apply blended flowmap to world
        ApplyWindFlowmap(FlowmapA, FlowmapB, Intensity);
    }
};
```

---

## Performance Tips

### Tip 1: Cache Common Scenarios
```cpp
// Pre-generate at level load
void ALevel::BeginPlay()
{
    PreGenerateCommonWindConditions();
}

void PreGenerateCommonWindConditions()
{
    // Common wind directions
    TArray<float> CommonDirections = { 0, 90, 180, 270 };

    for (float Direction : CommonDirections)
    {
        UTexture2D* Flowmap = GenerateWindFlowmap(
            LevelHeightmap,
            10.0f,      // Standard speed
            Direction,
            EShiftyVegetationType::Trees
        );

        CacheFlowmap(Direction, Flowmap);
    }
}
```

### Tip 2: Use Appropriate Resolution
```cpp
// Higher cell size = faster generation
FWindFlowmapParams FastParams;
FastParams.CellSize = 100.0f;  // Low detail, fast

FWindFlowmapParams DetailParams;
DetailParams.CellSize = 10.0f;  // High detail, slow

// Use fast params for background/distant terrain
// Use detail params for player-nearby areas
```

### Tip 3: Async Generation
```cpp
// Don't block game thread
GenerateFlowmapAsync(Heightmap, [this](UTexture2D* Result)
{
    // Called when complete
    ApplyFlowmapToWorld(Result);
});
```

---

## Troubleshooting Common Issues

### Issue: Flowmap appears uniform
**Solution:** Check heightmap has actual elevation variation
```cpp
// Validate heightmap
FShiftyDoubleGrid Grid;
FTextureUtils::ExtractHeightmapData(Heightmap, Grid, 30.0, Error);

float Min, Max;
FTextureUtils::NormalizeGrid(Grid, Min, Max);

if (Max - Min < 1.0f)
{
    UE_LOG(LogTemp, Warning, TEXT("Heightmap has minimal variation: %f"), Max - Min);
}
```

### Issue: Wind direction seems inverted
**Solution:** Check meteorological vs Cartesian convention
```cpp
// WindNinja uses meteorological convention:
// 0° = wind FROM north (blowing south)
// In Unreal, you may need to flip:
float UnrealDirection = (WindNinjaDirection + 180.0f) % 360.0f;
```

### Issue: Performance is slow
**Solution:** Optimize parameters
```cpp
FWindFlowmapParams OptimizedParams;
OptimizedParams.CellSize = 50.0f;              // Larger cells
OptimizedParams.NumVerticalLayers = 10;        // Fewer layers
OptimizedParams.bUseMultiThreading = true;     // Enable MT
```

---

For more examples and use cases, join the ShiftyMapper community or check the plugin documentation.
