// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "ShiftyMapperTypes.h"

/**
 * Utility functions for converting between UTexture2D and grid data structures
 */
class FTextureUtils
{
public:
	/**
	 * Extract elevation data from a heightmap texture
	 *
	 * @param HeightmapTexture - The input heightmap texture (grayscale, R16 or R8 format preferred)
	 * @param OutGrid - Output elevation grid
	 * @param CellSize - Size of each cell in meters
	 * @param OutErrorMessage - Error message if extraction fails
	 * @return true if extraction succeeded
	 */
	static bool ExtractHeightmapData(
		UTexture2D* HeightmapTexture,
		FShiftyDoubleGrid& OutGrid,
		double CellSize,
		FString& OutErrorMessage
	);

	/**
	 * Create a flowmap texture from velocity and angle grids
	 * Flowmap encoding: RG channels encode normalized flow direction
	 * - R channel: X component of flow direction (remapped to 0-1)
	 * - G channel: Y component of flow direction (remapped to 0-1)
	 * - B channel: Flow magnitude (normalized)
	 * - A channel: 1.0 (fully opaque)
	 *
	 * @param VelocityGrid - Grid of wind velocities
	 * @param AngleGrid - Grid of wind angles (degrees)
	 * @param OutErrorMessage - Error message if creation fails
	 * @return Created flowmap texture, or nullptr on failure
	 */
	static UTexture2D* CreateFlowmapTexture(
		const FShiftyFloatGrid& VelocityGrid,
		const FShiftyFloatGrid& AngleGrid,
		FString& OutErrorMessage
	);

	/**
	 * Create a grayscale texture from a float grid
	 *
	 * @param Grid - Input grid data
	 * @param OutErrorMessage - Error message if creation fails
	 * @return Created texture, or nullptr on failure
	 */
	static UTexture2D* CreateGrayscaleTexture(
		const FShiftyFloatGrid& Grid,
		FString& OutErrorMessage
	);

	/**
	 * Normalize a grid's values to 0-1 range
	 *
	 * @param Grid - Grid to normalize
	 * @param OutMin - Output minimum value found
	 * @param OutMax - Output maximum value found
	 */
	static void NormalizeGrid(
		FShiftyFloatGrid& Grid,
		float& OutMin,
		float& OutMax
	);

	/**
	 * Get pixel data from texture as float array
	 * Handles various texture formats and converts to float
	 *
	 * @param Texture - Input texture
	 * @param OutPixelData - Output pixel data array
	 * @param OutWidth - Output texture width
	 * @param OutHeight - Output texture height
	 * @return true if successful
	 */
	static bool GetTexturePixelData(
		UTexture2D* Texture,
		TArray<float>& OutPixelData,
		int32& OutWidth,
		int32& OutHeight
	);

	/**
	 * Create a new texture from RGBA8 pixel data
	 *
	 * @param Width - Texture width
	 * @param Height - Texture height
	 * @param PixelData - RGBA8 pixel data (4 bytes per pixel)
	 * @param TextureName - Name for the new texture
	 * @return Created texture, or nullptr on failure
	 */
	static UTexture2D* CreateTextureFromRGBA8(
		int32 Width,
		int32 Height,
		const TArray<uint8>& PixelData,
		const FString& TextureName = TEXT("GeneratedTexture")
	);

	/**
	 * Validate that a texture is suitable for use as a heightmap
	 *
	 * @param Texture - Texture to validate
	 * @param OutErrorMessage - Error message if validation fails
	 * @return true if texture is valid
	 */
	static bool ValidateHeightmapTexture(
		UTexture2D* Texture,
		FString& OutErrorMessage
	);

private:
	/**
	 * Convert angle in degrees to normalized X/Y components
	 *
	 * @param AngleDegrees - Angle in degrees (0 = North, clockwise)
	 * @param OutX - Output X component (-1 to 1)
	 * @param OutY - Output Y component (-1 to 1)
	 */
	static void AngleToVector(float AngleDegrees, float& OutX, float& OutY);

	/**
	 * Remap value from [-1, 1] to [0, 1] for texture encoding
	 */
	static FORCEINLINE float RemapToTexture(float Value)
	{
		return (Value + 1.0f) * 0.5f;
	}
};
