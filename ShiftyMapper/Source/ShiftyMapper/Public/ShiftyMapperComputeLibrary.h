// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShiftyMapperComputeTypes.h"
#include "Engine/Texture2D.h"
#include "ShiftyMapperComputeLibrary.generated.h"

class ALandscape;

/**
 * Blueprint function library for GPU-accelerated terrain analysis
 * All functions run on the GPU for high performance
 */
UCLASS()
class SHIFTYMAPPER_API UShiftyMapperComputeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// Input Utilities
	// ========================================================================

	/**
	 * Get heightmap from landscape actor
	 * @param Landscape - Landscape to extract from
	 * @param bSuccess - Whether extraction succeeded
	 * @return Heightmap texture
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Input")
	static UTexture2D* GetHeightmapFromLandscape(
		ALandscape* Landscape,
		bool& bSuccess
	);

	/**
	 * Get heightmap from texture or landscape
	 * @param Heightmap - Optional heightmap texture
	 * @param Landscape - Optional landscape actor
	 * @param bSuccess - Whether we got a valid heightmap
	 * @return Heightmap texture
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Input")
	static UTexture2D* GetHeightmapFromSource(
		UTexture2D* Heightmap,
		ALandscape* Landscape,
		bool& bSuccess
	);

	// ========================================================================
	// Curvature Analysis
	// ========================================================================

	/**
	 * Calculate terrain curvature (plan, profile, mean, gaussian)
	 * Output channels: R=Plan, G=Profile, B=Mean, A=Gaussian
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param Params - Curvature calculation parameters
	 * @return Result with curvature texture in RGBA channels
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curvature", meta = (
		DisplayName = "Calculate Curvature",
		Keywords = "curvature terrain analysis"))
	static FComputeMapResult CalculateCurvature(
		UTexture2D* Heightmap,
		const FCurvatureParams& Params
	);

	/**
	 * Calculate terrain curvature from landscape
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curvature")
	static FComputeMapResult CalculateCurvatureFromLandscape(
		ALandscape* Landscape,
		const FCurvatureParams& Params
	);

	// ========================================================================
	// Roughness Analysis
	// ========================================================================

	/**
	 * Calculate terrain roughness
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param Params - Roughness calculation parameters
	 * @return Result with roughness in grayscale
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Roughness", meta = (
		DisplayName = "Calculate Roughness",
		Keywords = "roughness terrain analysis"))
	static FComputeMapResult CalculateRoughness(
		UTexture2D* Heightmap,
		const FRoughnessParams& Params
	);

	/**
	 * Calculate terrain roughness from landscape
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Roughness")
	static FComputeMapResult CalculateRoughnessFromLandscape(
		ALandscape* Landscape,
		const FRoughnessParams& Params
	);

	// ========================================================================
	// Erosion Simulation
	// ========================================================================

	/**
	 * Simulate hydraulic erosion
	 * Outputs: Primary=Modified Heightmap, Secondary=Erosion, Tertiary=Deposition
	 *
	 * @param Heightmap - Input heightmap texture (will be modified)
	 * @param Params - Erosion simulation parameters
	 * @return Result with eroded heightmap and erosion maps
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Erosion", meta = (
		DisplayName = "Simulate Erosion",
		Keywords = "erosion simulation water flow"))
	static FComputeMapResult SimulateErosion(
		UTexture2D* Heightmap,
		const FErosionParams& Params
	);

	/**
	 * Simulate erosion on landscape
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Erosion")
	static FComputeMapResult SimulateErosionOnLandscape(
		ALandscape* Landscape,
		const FErosionParams& Params
	);

	// ========================================================================
	// Groundwater Analysis
	// ========================================================================

	/**
	 * Calculate groundwater/moisture map
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param Params - Groundwater calculation parameters
	 * @return Result with groundwater distribution
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Groundwater", meta = (
		DisplayName = "Calculate Groundwater",
		Keywords = "groundwater moisture water table"))
	static FComputeMapResult CalculateGroundwater(
		UTexture2D* Heightmap,
		const FGroundwaterParams& Params
	);

	// ========================================================================
	// Soil Fertility / Vegetation Masks
	// ========================================================================

	/**
	 * Generate vegetation suitability masks
	 * Output channels: R=Grass, G=Shrub, B=Tree
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param Params - Soil fertility calculation parameters
	 * @return Result with vegetation masks in RGB channels
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Vegetation", meta = (
		DisplayName = "Calculate Vegetation Masks",
		Keywords = "vegetation fertility soil grass shrub tree"))
	static FComputeMapResult CalculateVegetationMasks(
		UTexture2D* Heightmap,
		const FSoilFertilityParams& Params
	);

	// ========================================================================
	// Sunlight / Insolation
	// ========================================================================

	/**
	 * Calculate sunlight/insolation map
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param Params - Sunlight calculation parameters
	 * @return Result with sunlight intensity (includes shadows and diffuse)
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Sunlight", meta = (
		DisplayName = "Calculate Sunlight",
		Keywords = "sunlight insolation shadow lighting"))
	static FComputeMapResult CalculateSunlight(
		UTexture2D* Heightmap,
		const FSunlightParams& Params
	);

	/**
	 * Calculate sunlight from landscape
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Sunlight")
	static FComputeMapResult CalculateSunlightFromLandscape(
		ALandscape* Landscape,
		const FSunlightParams& Params
	);

	// ========================================================================
	// Snowfall / Melt
	// ========================================================================

	/**
	 * Calculate snowfall and melt distribution
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param Params - Snowfall calculation parameters
	 * @return Result with snow accumulation
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Snow", meta = (
		DisplayName = "Calculate Snowfall",
		Keywords = "snow snowfall melt temperature"))
	static FComputeMapResult CalculateSnowfall(
		UTexture2D* Heightmap,
		const FSnowfallParams& Params
	);

	// ========================================================================
	// Slope / Aspect (Utility Functions)
	// ========================================================================

	/**
	 * Calculate slope map (in degrees)
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param CellSize - Size of each cell in meters
	 * @return Result with slope in grayscale
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Utilities")
	static FComputeMapResult CalculateSlope(
		UTexture2D* Heightmap,
		float CellSize = 100.0f
	);

	/**
	 * Calculate aspect map (direction of slope in degrees, 0-360)
	 *
	 * @param Heightmap - Input heightmap texture
	 * @param CellSize - Size of each cell in meters
	 * @return Result with aspect in grayscale (normalized to 0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Utilities")
	static FComputeMapResult CalculateAspect(
		UTexture2D* Heightmap,
		float CellSize = 100.0f
	);

	// ========================================================================
	// Curve Utilities
	// ========================================================================

	/**
	 * Create default curve for erosion sediment capacity
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curves")
	static UCurveFloat* CreateDefaultSedimentCapacityCurve();

	/**
	 * Create default curve for grass slope suitability
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curves")
	static UCurveFloat* CreateDefaultGrassSlopeCurve();

	/**
	 * Create default curve for shrub slope suitability
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curves")
	static UCurveFloat* CreateDefaultShrubSlopeCurve();

	/**
	 * Create default curve for tree slope suitability
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curves")
	static UCurveFloat* CreateDefaultTreeSlopeCurve();

	/**
	 * Create default sun path curve (hour of day -> sun elevation)
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Curves")
	static UCurveFloat* CreateDefaultSunPathCurve();

private:
	/** Helper to get cell size from heightmap or landscape */
	static float GetCellSizeFromSource(UTexture2D* Heightmap, ALandscape* Landscape);

	/** Helper to create result structure */
	static FComputeMapResult MakeResult(bool bSuccess, const FString& ErrorMessage = FString());
};
