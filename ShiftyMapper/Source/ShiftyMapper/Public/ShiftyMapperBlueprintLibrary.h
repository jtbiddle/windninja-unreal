// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShiftyMapperTypes.h"
#include "ShiftyMapperBlueprintLibrary.generated.h"

/**
 * Blueprint function library for ShiftyMapper
 * Provides easy-to-use Blueprint nodes for generating maps from heightmaps
 */
UCLASS()
class SHIFTYMAPPER_API UShiftyMapperBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Generate a wind flowmap from a heightmap texture
	 *
	 * @param HeightmapTexture - The input heightmap texture (grayscale, elevation data)
	 * @param WindSpeed - Wind speed in meters per second (default: 10 m/s)
	 * @param WindDirection - Wind direction in degrees clockwise from North (0-360, default: 270 = West wind)
	 * @param InputWindHeight - Height above vegetation where wind is measured in meters (default: 6.1m / 20ft)
	 * @param OutputWindHeight - Height above vegetation for output wind in meters (default: 6.1m / 20ft)
	 * @param VegetationType - Type of vegetation covering the terrain (affects surface roughness)
	 * @param CellSize - Size of each heightmap pixel in meters (default: 30m)
	 * @param bSuccess - Output parameter indicating if generation succeeded
	 * @return The generated wind flowmap texture (RG channels encode wind direction)
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Wind", meta = (
		DisplayName = "Generate Wind Flowmap",
		Keywords = "wind flowmap heightmap terrain generate",
		AdvancedDisplay = "InputWindHeight,OutputWindHeight,CellSize"))
	static UTexture2D* GenerateWindFlowmap(
		UTexture2D* HeightmapTexture,
		float WindSpeed = 10.0f,
		float WindDirection = 270.0f,
		EShiftyVegetationType VegetationType = EShiftyVegetationType::Trees,
		float InputWindHeight = 6.1f,
		float OutputWindHeight = 6.1f,
		float CellSize = 30.0f,
		bool& bSuccess
	);

	/**
	 * Generate a wind flowmap with full parameter control
	 *
	 * @param HeightmapTexture - The input heightmap texture
	 * @param Parameters - Detailed wind simulation parameters
	 * @param bSuccess - Output parameter indicating if generation succeeded
	 * @param ErrorMessage - Output parameter with error details if generation failed
	 * @return The generated wind flowmap texture
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Wind", meta = (
		DisplayName = "Generate Wind Flowmap Advanced",
		Keywords = "wind flowmap heightmap terrain generate advanced"))
	static UTexture2D* GenerateWindFlowmapAdvanced(
		UTexture2D* HeightmapTexture,
		const FWindFlowmapParams& Parameters,
		bool& bSuccess,
		FString& ErrorMessage
	);

	/**
	 * Generate a map from heightmap using the specified generator type
	 * This is the generic entry point for all map types
	 *
	 * @param HeightmapTexture - The input heightmap texture
	 * @param MapType - The type of map to generate
	 * @return Result structure containing the output texture and metadata
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper", meta = (
		DisplayName = "Generate Map From Heightmap",
		Keywords = "heightmap terrain generate map"))
	static FShiftyMapResult GenerateMapFromHeightmap(
		UTexture2D* HeightmapTexture,
		EShiftyMapType MapType
	);

	/**
	 * Check if a map type is currently supported
	 *
	 * @param MapType - The map type to check
	 * @return true if the map type is supported
	 */
	UFUNCTION(BlueprintPure, Category = "ShiftyMapper", meta = (
		DisplayName = "Is Map Type Supported",
		Keywords = "support check map type"))
	static bool IsMapTypeSupported(EShiftyMapType MapType);

	/**
	 * Get a list of all supported map types
	 *
	 * @return Array of supported map types
	 */
	UFUNCTION(BlueprintPure, Category = "ShiftyMapper", meta = (
		DisplayName = "Get Supported Map Types",
		Keywords = "support list map types"))
	static TArray<EShiftyMapType> GetSupportedMapTypes();

	/**
	 * Create default wind flowmap parameters
	 *
	 * @return Default parameters structure
	 */
	UFUNCTION(BlueprintPure, Category = "ShiftyMapper|Wind", meta = (
		DisplayName = "Make Default Wind Parameters",
		Keywords = "default parameters wind"))
	static FWindFlowmapParams MakeDefaultWindParameters();

	/**
	 * Create custom wind flowmap parameters
	 *
	 * @param WindSpeed - Wind speed in meters per second
	 * @param WindDirection - Wind direction in degrees (0-360)
	 * @param VegetationType - Vegetation type
	 * @param CellSize - Cell size in meters
	 * @return Parameters structure
	 */
	UFUNCTION(BlueprintPure, Category = "ShiftyMapper|Wind", meta = (
		DisplayName = "Make Wind Parameters",
		Keywords = "make parameters wind"))
	static FWindFlowmapParams MakeWindParameters(
		float WindSpeed,
		float WindDirection,
		EShiftyVegetationType VegetationType,
		float CellSize
	);
};
