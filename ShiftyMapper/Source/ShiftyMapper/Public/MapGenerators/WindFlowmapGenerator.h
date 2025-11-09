// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "IMapGenerator.h"
#include "ShiftyMapperTypes.h"

/**
 * Generator for wind flowmaps using WindNinja mass-consistent solver
 *
 * This generator takes a heightmap and wind parameters, then computes
 * a physically-based wind field over the terrain. The output is a flowmap
 * texture with RG channels encoding wind direction/velocity.
 */
class SHIFTYMAPPER_API FWindFlowmapGenerator : public IMapGenerator
{
public:
	FWindFlowmapGenerator();
	virtual ~FWindFlowmapGenerator();

	// IMapGenerator interface
	virtual EShiftyMapType GetMapType() const override;
	virtual FString GetGeneratorName() const override;
	virtual FString GetGeneratorDescription() const override;
	virtual bool ValidateInput(UTexture2D* HeightmapTexture, FString& OutErrorMessage) const override;
	virtual void Generate(UTexture2D* HeightmapTexture, FShiftyMapResult& OutResult) override;

	/**
	 * Generate a wind flowmap with specific parameters
	 * @param HeightmapTexture - Input heightmap texture
	 * @param Params - Wind simulation parameters
	 * @param OutResult - Result structure with output texture
	 */
	void GenerateWindFlowmap(UTexture2D* HeightmapTexture, const FWindFlowmapParams& Params, FShiftyMapResult& OutResult);

	/**
	 * Set the parameters for wind simulation
	 */
	void SetParameters(const FWindFlowmapParams& InParams);

	/**
	 * Get the current parameters
	 */
	const FWindFlowmapParams& GetParameters() const { return Parameters; }

private:
	/** Current wind simulation parameters */
	FWindFlowmapParams Parameters;

	/**
	 * Convert heightmap texture to elevation grid
	 */
	bool ConvertHeightmapToElevationGrid(UTexture2D* HeightmapTexture, FShiftyDoubleGrid& OutElevationGrid, FString& OutErrorMessage);

	/**
	 * Run the core WindNinja simulation
	 */
	bool RunWindNinjaSimulation(const FShiftyDoubleGrid& ElevationGrid, FShiftyFloatGrid& OutVelocityGrid, FShiftyFloatGrid& OutAngleGrid, FString& OutErrorMessage);

	/**
	 * Convert velocity and angle grids to flowmap texture
	 * Flowmap format: RG channels encode normalized flow direction
	 */
	UTexture2D* ConvertToFlowmapTexture(const FShiftyFloatGrid& VelocityGrid, const FShiftyFloatGrid& AngleGrid, FString& OutErrorMessage);

	/**
	 * Get surface roughness value based on vegetation type
	 */
	double GetRoughnessForVegetation(EShiftyVegetationType VegType) const;

	/**
	 * Get roughness height based on vegetation type
	 */
	double GetRoughnessHeightForVegetation(EShiftyVegetationType VegType) const;
};
