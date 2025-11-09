// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "LandscapeInputUtils.generated.h"

class ALandscape;
class ULandscapeComponent;

/**
 * Utilities for extracting heightmap data from Landscape actors
 */
UCLASS()
class SHIFTYMAPPER_API ULandscapeInputUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Extract heightmap texture from a Landscape actor
	 * @param Landscape - The landscape actor to extract from
	 * @param OutHeightmap - The extracted heightmap texture
	 * @param OutErrorMessage - Error message if extraction fails
	 * @return true if extraction succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Landscape")
	static bool ExtractLandscapeHeightmap(
		ALandscape* Landscape,
		UTexture2D*& OutHeightmap,
		FString& OutErrorMessage
	);

	/**
	 * Get all landscape actors in the current world
	 * @param WorldContext - World context object
	 * @return Array of landscape actors found
	 */
	UFUNCTION(BlueprintCallable, Category = "ShiftyMapper|Landscape", meta = (WorldContext = "WorldContext"))
	static TArray<ALandscape*> GetAllLandscapesInWorld(const UObject* WorldContext);

	/**
	 * Extract heightmap from landscape components and combine into single texture
	 * @param Landscape - The landscape actor
	 * @param Resolution - Target resolution (0 = native resolution)
	 * @return Combined heightmap texture
	 */
	static UTexture2D* CreateHeightmapFromLandscape(ALandscape* Landscape, int32 Resolution = 0);

	/**
	 * Get the world-space bounds of a landscape
	 * @param Landscape - The landscape actor
	 * @param OutMin - Minimum world-space coordinates
	 * @param OutMax - Maximum world-space coordinates
	 */
	UFUNCTION(BlueprintPure, Category = "ShiftyMapper|Landscape")
	static void GetLandscapeBounds(
		ALandscape* Landscape,
		FVector& OutMin,
		FVector& OutMax
	);

	/**
	 * Get the cell size (meters per pixel) of a landscape
	 * @param Landscape - The landscape actor
	 * @return Cell size in meters
	 */
	UFUNCTION(BlueprintPure, Category = "ShiftyMapper|Landscape")
	static float GetLandscapeCellSize(ALandscape* Landscape);

private:
	/** Helper to read heightmap data from a component */
	static bool ReadComponentHeightData(
		ULandscapeComponent* Component,
		TArray<uint16>& OutHeightData,
		int32& OutSizeX,
		int32& OutSizeY
	);
};
