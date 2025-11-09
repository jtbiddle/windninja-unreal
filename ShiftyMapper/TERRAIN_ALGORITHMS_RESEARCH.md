# Terrain Generation Algorithms Research

Research and implementation of industry-standard terrain analysis algorithms from professional tools like Gaea, World Machine, and similar software.

## Industry Tools Overview

### Gaea (QuadSpinner)
- **Focus**: GPU-accelerated, real-time terrain generation
- **Strengths**: Fast iteration, node-based workflow, multiple erosion types
- **Key Features**:
  - Hydraulic erosion (streampower-based)
  - Thermal erosion (talus angle)
  - Wind erosion
  - Coastal erosion
  - Flow and sediment maps
  - Real-time previews

### World Machine (Software Inc)
- **Focus**: Photo-realistic terrain generation
- **Strengths**: Physical accuracy, layered workflow
- **Key Features**:
  - Multi-pass hydraulic erosion
  - Precipitation-based erosion
  - Sediment deposition modeling
  - Snow accumulation
  - Vegetation placement based on environmental factors
  - Macro/Micro features

### Houdini Heightfield Tools
- **Focus**: Procedural terrain with VFX integration
- **Strengths**: Extreme flexibility, physically-based
- **Key Features**:
  - Velocity-based erosion
  - Debris flow simulation
  - Advanced sediment transport
  - Multi-layer terrain (bedrock + sediment)

## Optimal Algorithms by Feature

### 1. Hydraulic Erosion

**Current Implementation**: Particle-based (Musgrave method)
**Optimal Implementation**: Pipe Model + Stream Power Law

#### Pipe Model (Mei et al. 2007)
Used by: Gaea, Houdini

**Algorithm**:
```
For each cell:
1. Water increment from precipitation
2. Calculate outflow flux to neighbors using pipe model
3. Update water height
4. Calculate velocity field from water flux
5. Erosion/deposition based on sediment capacity
6. Transport sediment with water flow
7. Evaporate water
```

**Advantages**:
- More physically accurate than particles
- Handles large-scale flow patterns
- Conserves mass (water and sediment)
- GPU-friendly (grid-based)

**Implementation**:
```hlsl
// Pipe model - 4 pipes per cell (N, S, E, W)
float4 outflow; // Flux to each neighbor

// 1. Calculate pressure difference
float heightDiff = (waterHeight + terrainHeight)[cell] - (waterHeight + terrainHeight)[neighbor];

// 2. Update flux using Navier-Stokes approximation
outflow = max(0, outflow + deltaTime * (gravity * heightDiff / pipeLength));

// 3. Scale to prevent negative water
float totalOutflow = sum(outflow);
if (totalOutflow > waterHeight) {
    outflow *= waterHeight / totalOutflow;
}

// 4. Calculate velocity from flux
velocity = (inflowTotal - outflowTotal) / cellArea;

// 5. Erosion using stream power law
sedimentCapacity = Kc * sqrt(velocityMagnitude) * slope;
erosionAmount = Kr * (sedimentCapacity - sediment);
```

#### Stream Power Law
Used by: Gaea, World Machine

**Formula**: E = K * A^m * S^n

Where:
- E = erosion rate
- A = drainage area (flow accumulation)
- S = slope
- K = erodibility coefficient
- m, n = empirical constants (typically m=0.5, n=1.0)

### 2. Thermal Erosion

**Optimal Implementation**: Talus Angle Method
Used by: All major tools

**Algorithm**:
```
For each cell:
1. Calculate slope to each neighbor
2. If slope > talus angle:
   - Calculate material to move
   - Distribute to lower neighbors
3. Update heights
```

**Key Parameters**:
- Talus angle: 30-45° for most materials
- Material hardness: affects erosion rate
- Iteration count: 10-100 passes

**Implementation**:
```hlsl
const float talusAngle = radians(35.0); // Typical for rock/soil
const float talusTangent = tan(talusAngle);

for each neighbor:
    float heightDiff = height[cell] - height[neighbor];
    float distance = cellSize * (isDiagonal ? 1.414 : 1.0);
    float slope = heightDiff / distance;

    if (slope > talusTangent) {
        float excess = (slope - talusTangent) * distance * 0.5;
        height[cell] -= excess;
        height[neighbor] += excess;
    }
```

### 3. Flow Accumulation

**Current Implementation**: None
**Optimal Implementation**: D8 or D-Infinity

#### D8 (8-Direction Flow)
Used by: Most GIS tools, simple and fast

**Algorithm**:
```
1. Create direction map (flow to steepest descent)
2. Sort cells by elevation (high to low)
3. Accumulate flow downstream
```

**Advantages**:
- Simple, fast
- Good for most terrain
- GPU-friendly

**Limitations**:
- Can create artifacts on flat terrain
- Flow limited to 8 directions

#### D-Infinity (Continuous Direction)
Used by: World Machine, Gaea (optional)

**Algorithm**:
```
1. Calculate gradient direction (0-360°)
2. Distribute flow to two neighbors based on angle
3. Accumulate with fractional contributions
```

**Advantages**:
- More accurate flow patterns
- No directional bias
- Better for subtle terrain

**Implementation**:
```hlsl
// D8 - Simpler, faster
int flowDir = 0;
float maxSlope = 0;
for (int i = 0; i < 8; i++) {
    float slope = (height - neighborHeight[i]) / distance[i];
    if (slope > maxSlope) {
        maxSlope = slope;
        flowDir = i;
    }
}

// D-Infinity - More accurate
float2 gradient = calculateGradient(height, neighbors);
float flowAngle = atan2(gradient.y, gradient.x);
// Distribute to two neighbors based on angle fraction
```

### 4. Sediment Transport

**Optimal Implementation**: Suspension Load + Bed Load
Used by: Houdini, advanced World Machine

**Components**:

#### Suspension Load (fine particles)
```
capacity = Ks * velocity^2
if (sediment > capacity):
    deposit = Kd * (sediment - capacity)
else:
    erode = Ke * (capacity - sediment) * (1 - sediment/capacity)
```

#### Bed Load (coarse particles)
```
criticalVelocity = sqrt(particleSize * (density - waterDensity) * gravity)
if (velocity > criticalVelocity):
    transport = Kb * (velocity - criticalVelocity)^2
```

### 5. Curvature Analysis

**Current Implementation**: Good (plan, profile, mean, gaussian)
**Enhancements**: Add shape classification

#### Shape Index
```
shapeIndex = (2/PI) * atan((kMax + kMin) / (kMax - kMin))

Ranges:
 1.0 = Dome/peak
 0.5 = Ridge
 0.0 = Saddle
-0.5 = Valley
-1.0 = Pit/depression
```

#### Curvedness
```
curvedness = sqrt((kMax^2 + kMin^2) / 2)
```

Combines with shape index for terrain classification.

### 6. Vegetation/Biome Distribution

**Optimal Implementation**: Multi-Factor Suitability
Used by: Gaea, World Machine

**Factors**:
1. **Climate** (Temperature + Precipitation)
2. **Slope** (Stability)
3. **Aspect** (Sun exposure)
4. **Elevation** (Temperature lapse)
5. **Soil Moisture** (From flow accumulation)
6. **Soil Depth** (Accumulated sediment)

**Algorithm**:
```
For each vegetation type:
1. Calculate climate suitability
   temp_suit = GaussCurve(temperature, optimal_temp, temp_range)
   precip_suit = GaussCurve(precipitation, optimal_precip, precip_range)

2. Calculate terrain suitability
   slope_suit = Curve(slope, max_slope)
   aspect_suit = AspectCurve(aspect, preferred_direction)

3. Calculate resource suitability
   moisture_suit = Curve(soilMoisture, min_moisture, max_moisture)
   depth_suit = Curve(soilDepth, min_depth)

4. Combine factors (multiplicative or weighted)
   suitability = temp_suit * precip_suit * slope_suit * moisture_suit * depth_suit

5. Competition (optional)
   If multiple types suitable, dominant type wins
```

### 7. Snowfall and Melt

**Optimal Implementation**: Energy Balance Model
Used by: World Machine

**Factors**:
1. **Accumulation**:
   - Elevation (temperature lapse rate)
   - Precipitation type (rain vs snow threshold)
   - Wind redistribution

2. **Melt**:
   - Solar radiation (aspect, slope, time of day)
   - Air temperature
   - Longwave radiation

**Algorithm**:
```
// Accumulation
temperature = baseTemp + elevation * lapseRate;
if (temperature < snowThreshold) {
    snowfall = precipitation;
} else {
    snowfall = 0;
}

// Wind redistribution (simplified)
if (exposedToWind && slope > threshold) {
    snowDepth *= (1 - windScour);
}

// Melt
solarEnergy = insolation * (1 - albedo) * aspect_factor;
meltRate = (solarEnergy + temperature) * meltCoeff;
snowDepth = max(0, snowDepth - meltRate);
```

### 8. Roughness Calculation

**Current Implementation**: Good (StdDev, Range, Area Ratio)
**Additional Methods**:

#### Terrain Ruggedness Index (TRI)
Used by: GIS applications
```
TRI = sqrt(sum((z_center - z_neighbor)^2)) / 8
```

#### Vector Ruggedness Measure (VRM)
```
For each cell:
1. Calculate normal vectors for each neighbor
2. Calculate resultant vector
3. VRM = 1 - (length(resultant) / 9)
```

More robust than TRI, captures 3D terrain complexity.

### 9. Slope and Aspect

**Current Implementation**: To be added
**Optimal Implementation**: Horn's Method

#### Slope (Horn 1981)
```
// 3x3 neighborhood weights
dz_dx = ((z3 + 2*z6 + z9) - (z1 + 2*z4 + z7)) / (8 * cellSize)
dz_dy = ((z7 + 2*z8 + z9) - (z1 + 2*z2 + z3)) / (8 * cellSize)

slope_radians = atan(sqrt(dz_dx^2 + dz_dy^2))
slope_degrees = slope_radians * (180 / PI)
```

#### Aspect
```
aspect_radians = atan2(dz_dy, -dz_dx)
aspect_degrees = aspect_radians * (180 / PI)

if (aspect_degrees < 0):
    aspect_degrees += 360

// Special case for flat areas
if (dz_dx == 0 && dz_dy == 0):
    aspect = -1 // or noData value
```

## Performance Optimization Strategies

### From Gaea (GPU-First Design)
1. **Tile-Based Processing**: Process terrain in tiles for large datasets
2. **LOD Pyramid**: Multiple resolutions for preview
3. **Async Compute**: Overlap CPU and GPU work
4. **Persistent Buffers**: Reuse allocated memory
5. **Compute Queue**: Batch multiple operations

### From World Machine (Quality-First)
1. **Multi-Pass Refinement**: Coarse → Fine erosion
2. **Adaptive Sampling**: More detail where needed
3. **Convergence Detection**: Stop when stable
4. **Blending**: Smooth transitions between scales

### Implementation Strategy

```cpp
// Multi-resolution approach
struct TerrainLOD {
    int32 Resolution;
    UTexture2D* Heightmap;
    float CellSize;
};

TArray<TerrainLOD> LODs; // 4096 → 2048 → 1024 → 512

// Process from coarse to fine
for (int32 i = LODs.Num() - 1; i >= 0; i--) {
    ProcessErosion(LODs[i]);
    if (i > 0) {
        Upsample(LODs[i], LODs[i-1]); // Transfer to finer level
    }
}
```

## Recommended Implementation Priority

### Phase 1: Core Accuracy
1. ✅ Curvature (already good)
2. 🔄 Flow Accumulation (D8 algorithm)
3. 🔄 Hydraulic Erosion (Pipe model)
4. 🔄 Thermal Erosion (Talus angle)

### Phase 2: Realism
5. 🔄 Sediment Transport (suspension + bed load)
6. 🔄 Climate-Based Vegetation
7. 🔄 Energy Balance Snow

### Phase 3: Advanced
8. 🔄 Multi-scale erosion
9. 🔄 Wind erosion
10. 🔄 Coastal erosion

## References

### Academic Papers
- Mei, X. et al. (2007): "Fast Hydraulic Erosion Simulation and Visualization on GPU"
- O'Callaghan & Mark (1984): "D8 Flow Direction Algorithm"
- Tarboton (1997): "D-Infinity Flow Direction Algorithm"
- Horn, B.K.P. (1981): "Hill Shading and the Reflectance Map"

### Industry Documentation
- Gaea Documentation: Erosion node specifications
- World Machine: Erosion device technical details
- Houdini: Heightfield erosion methods

### GIS Standards
- ESRI Spatial Analyst: Terrain analysis methods
- GRASS GIS: r.watershed, r.slope.aspect algorithms

## Implementation Notes

### Multi-Threading Strategy
- Use atomic operations for accumulation
- Separate read/write buffers (ping-pong)
- Synchronize between passes

### Memory Layout
- Structure of Arrays (SoA) better than Array of Structures (AoS)
- Coalesced memory access patterns
- Shared memory for neighborhood operations

### Numerical Stability
- Use double precision for accumulation
- Clamp values to prevent overflow
- Handle edge cases (flat terrain, pits)

---

**Next Steps**: Implement these optimal algorithms in ShiftyMapper, starting with the most impactful features (flow accumulation, improved erosion).
