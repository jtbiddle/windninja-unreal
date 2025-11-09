// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "ShiftyMapperTypes.h"

/**
 * Interface for all map generators
 * This allows for a modular, extensible architecture where different
 * map generation algorithms can be plugged in
 */
class SHIFTYMAPPER_API IMapGenerator
{
public:
	virtual ~IMapGenerator() = default;

	/**
	 * Get the type of map this generator produces
	 */
	virtual EShiftyMapType GetMapType() const = 0;

	/**
	 * Get a human-readable name for this generator
	 */
	virtual FString GetGeneratorName() const = 0;

	/**
	 * Get a description of what this generator does
	 */
	virtual FString GetGeneratorDescription() const = 0;

	/**
	 * Validate that the input heightmap is suitable for this generator
	 * @param HeightmapTexture - The input heightmap texture
	 * @param OutErrorMessage - Error message if validation fails
	 * @return true if the heightmap is valid for this generator
	 */
	virtual bool ValidateInput(UTexture2D* HeightmapTexture, FString& OutErrorMessage) const = 0;

	/**
	 * Generate the output map from the heightmap
	 * This is the main entry point for map generation
	 * @param HeightmapTexture - The input heightmap texture
	 * @param OutResult - The result structure containing the output texture and metadata
	 */
	virtual void Generate(UTexture2D* HeightmapTexture, FShiftyMapResult& OutResult) = 0;
};

/**
 * Factory for creating map generators
 * Allows runtime selection of different generators
 */
class SHIFTYMAPPER_API FMapGeneratorFactory
{
public:
	/**
	 * Create a map generator for the specified type
	 * @param MapType - The type of map to generate
	 * @return Shared pointer to the generator, or nullptr if type not supported
	 */
	static TSharedPtr<IMapGenerator> CreateGenerator(EShiftyMapType MapType);

	/**
	 * Get a list of all supported map types
	 */
	static TArray<EShiftyMapType> GetSupportedMapTypes();

	/**
	 * Check if a map type is currently supported
	 */
	static bool IsMapTypeSupported(EShiftyMapType MapType);
};
