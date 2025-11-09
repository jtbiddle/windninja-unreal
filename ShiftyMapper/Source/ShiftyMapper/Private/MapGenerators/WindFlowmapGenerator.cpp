// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#include "MapGenerators/WindFlowmapGenerator.h"
#include "TextureUtils.h"
#include "Async/ParallelFor.h"

FWindFlowmapGenerator::FWindFlowmapGenerator()
{
	Parameters = FWindFlowmapParams();
}

FWindFlowmapGenerator::~FWindFlowmapGenerator()
{
}

EShiftyMapType FWindFlowmapGenerator::GetMapType() const
{
	return EShiftyMapType::WindFlowmap;
}

FString FWindFlowmapGenerator::GetGeneratorName() const
{
	return TEXT("Wind Flowmap Generator");
}

FString FWindFlowmapGenerator::GetGeneratorDescription() const
{
	return TEXT("Generates wind flowmaps from heightmap data using terrain-based wind flow modeling. "
				"Takes into account terrain slope, aspect, and surface roughness to compute realistic wind patterns.");
}

bool FWindFlowmapGenerator::ValidateInput(UTexture2D* HeightmapTexture, FString& OutErrorMessage) const
{
	return FTextureUtils::ValidateHeightmapTexture(HeightmapTexture, OutErrorMessage);
}

void FWindFlowmapGenerator::Generate(UTexture2D* HeightmapTexture, FShiftyMapResult& OutResult)
{
	GenerateWindFlowmap(HeightmapTexture, Parameters, OutResult);
}

void FWindFlowmapGenerator::GenerateWindFlowmap(
	UTexture2D* HeightmapTexture,
	const FWindFlowmapParams& Params,
	FShiftyMapResult& OutResult)
{
	double StartTime = FPlatformTime::Seconds();

	OutResult.bSuccess = false;
	OutResult.ErrorMessage.Empty();

	// Validate input
	if (!ValidateInput(HeightmapTexture, OutResult.ErrorMessage))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Starting wind flowmap generation..."));
	UE_LOG(LogTemp, Log, TEXT("  Wind Speed: %.2f m/s"), Params.WindSpeed);
	UE_LOG(LogTemp, Log, TEXT("  Wind Direction: %.2f degrees"), Params.WindDirection);
	UE_LOG(LogTemp, Log, TEXT("  Cell Size: %.2f meters"), Params.CellSize);

	// Step 1: Convert heightmap to elevation grid
	FShiftyDoubleGrid ElevationGrid;
	if (!ConvertHeightmapToElevationGrid(HeightmapTexture, ElevationGrid, OutResult.ErrorMessage))
	{
		UE_LOG(LogTemp, Error, TEXT("ShiftyMapper: Failed to convert heightmap: %s"), *OutResult.ErrorMessage);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Converted heightmap to elevation grid (%dx%d)"),
		ElevationGrid.NumCols, ElevationGrid.NumRows);

	// Step 2: Run wind simulation
	FShiftyFloatGrid VelocityGrid;
	FShiftyFloatGrid AngleGrid;
	if (!RunWindNinjaSimulation(ElevationGrid, VelocityGrid, AngleGrid, OutResult.ErrorMessage))
	{
		UE_LOG(LogTemp, Error, TEXT("ShiftyMapper: Wind simulation failed: %s"), *OutResult.ErrorMessage);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Wind simulation completed"));

	// Step 3: Convert to flowmap texture
	OutResult.OutputTexture = ConvertToFlowmapTexture(VelocityGrid, AngleGrid, OutResult.ErrorMessage);
	if (!OutResult.OutputTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("ShiftyMapper: Failed to create flowmap texture: %s"), *OutResult.ErrorMessage);
		return;
	}

	// Success!
	OutResult.bSuccess = true;
	OutResult.GenerationTimeSeconds = FPlatformTime::Seconds() - StartTime;

	// Add metadata
	OutResult.Metadata.Add(TEXT("WindSpeed"), FString::Printf(TEXT("%.2f m/s"), Params.WindSpeed));
	OutResult.Metadata.Add(TEXT("WindDirection"), FString::Printf(TEXT("%.2f degrees"), Params.WindDirection));
	OutResult.Metadata.Add(TEXT("CellSize"), FString::Printf(TEXT("%.2f meters"), Params.CellSize));
	OutResult.Metadata.Add(TEXT("GridSize"), FString::Printf(TEXT("%dx%d"), VelocityGrid.NumCols, VelocityGrid.NumRows));
	OutResult.Metadata.Add(TEXT("GenerationTime"), FString::Printf(TEXT("%.3f seconds"), OutResult.GenerationTimeSeconds));

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Flowmap generation completed successfully in %.3f seconds"),
		OutResult.GenerationTimeSeconds);
}

void FWindFlowmapGenerator::SetParameters(const FWindFlowmapParams& InParams)
{
	Parameters = InParams;
}

bool FWindFlowmapGenerator::ConvertHeightmapToElevationGrid(
	UTexture2D* HeightmapTexture,
	FShiftyDoubleGrid& OutElevationGrid,
	FString& OutErrorMessage)
{
	return FTextureUtils::ExtractHeightmapData(
		HeightmapTexture,
		OutElevationGrid,
		Parameters.CellSize,
		OutErrorMessage
	);
}

bool FWindFlowmapGenerator::RunWindNinjaSimulation(
	const FShiftyDoubleGrid& ElevationGrid,
	FShiftyFloatGrid& OutVelocityGrid,
	FShiftyFloatGrid& OutAngleGrid,
	FString& OutErrorMessage)
{
	// NOTE: This is a SIMPLIFIED wind flow model for the initial release
	// TODO: Integrate full WindNinja mass-consistent solver for production use
	//
	// This simplified version uses terrain-based wind flow approximations:
	// 1. Calculates terrain slope and aspect
	// 2. Applies speed-up/slow-down factors based on slope
	// 3. Deflects wind direction based on terrain aspect
	// 4. Accounts for surface roughness

	const int32 Width = ElevationGrid.NumCols;
	const int32 Height = ElevationGrid.NumRows;
	const double CellSize = ElevationGrid.CellSize;

	// Initialize output grids
	OutVelocityGrid = FShiftyFloatGrid(Width, Height, ElevationGrid.XLLCorner, ElevationGrid.YLLCorner, CellSize, -9999.0f);
	OutAngleGrid = FShiftyFloatGrid(Width, Height, ElevationGrid.XLLCorner, ElevationGrid.YLLCorner, CellSize, -9999.0f);

	// Get wind parameters
	const float InputSpeed = Parameters.WindSpeed;
	const float InputDirection = Parameters.WindDirection;
	const double Roughness = GetRoughnessForVegetation(Parameters.VegetationType);

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Computing terrain-based wind flow (simplified model)"));

	// Calculate slope and aspect for each cell
	auto ComputeWindForCell = [&](int32 Index)
	{
		const int32 Row = Index / Width;
		const int32 Col = Index % Width;

		// Skip edge cells
		if (Row == 0 || Row == Height - 1 || Col == 0 || Col == Width - 1)
		{
			OutVelocityGrid(Row, Col) = InputSpeed;
			OutAngleGrid(Row, Col) = InputDirection;
			return;
		}

		// Calculate slope using central differences (in radians)
		const double Z_Center = ElevationGrid(Row, Col);
		const double Z_East = ElevationGrid(Row, Col + 1);
		const double Z_West = ElevationGrid(Row, Col - 1);
		const double Z_North = ElevationGrid(Row - 1, Col);
		const double Z_South = ElevationGrid(Row + 1, Col);

		const double dZ_dX = (Z_East - Z_West) / (2.0 * CellSize);
		const double dZ_dY = (Z_North - Z_South) / (2.0 * CellSize);

		// Slope magnitude and aspect
		const double SlopeMagnitude = FMath::Sqrt(dZ_dX * dZ_dX + dZ_dY * dZ_dY);
		const double SlopeDegrees = FMath::RadiansToDegrees(FMath::Atan(SlopeMagnitude));

		// Aspect (direction of steepest descent) in degrees from north
		double AspectDegrees = 0.0;
		if (SlopeMagnitude > 0.0001)
		{
			AspectDegrees = FMath::RadiansToDegrees(FMath::Atan2(dZ_dX, dZ_dY));
			if (AspectDegrees < 0.0)
			{
				AspectDegrees += 360.0;
			}
		}

		// Calculate wind speed modification based on slope
		// Upslope: wind slows down
		// Downslope: wind speeds up
		// Cross-slope: wind maintains speed

		double WindDirRadians = FMath::DegreesToRadians(InputDirection);
		double SlopeDirRadians = FMath::DegreesToRadians(AspectDegrees);

		// Dot product to determine if wind is going upslope or downslope
		double CosAngle = FMath::Cos(WindDirRadians - SlopeDirRadians);

		// Speed modification factor
		// Positive CosAngle = upslope (slow down)
		// Negative CosAngle = downslope (speed up)
		double SpeedModifier = 1.0;
		if (SlopeDegrees > 5.0) // Only apply modification for significant slopes
		{
			// Empirical formula: speed changes ±20% max based on slope
			double SlopeFactor = FMath::Clamp(SlopeDegrees / 45.0, 0.0, 1.0); // Normalize to 0-1
			SpeedModifier = 1.0 - (CosAngle * SlopeFactor * 0.4); // ±40% max change
			SpeedModifier = FMath::Clamp(SpeedModifier, 0.5, 1.5); // Clamp to reasonable range
		}

		// Apply roughness factor (higher roughness = slower wind)
		SpeedModifier *= (1.0 - Roughness * 0.3);

		// Calculate final velocity
		float FinalVelocity = InputSpeed * SpeedModifier;

		// Calculate wind direction deflection
		// Wind tends to flow around obstacles and follow terrain
		float DirectionModifier = 0.0f;
		if (SlopeDegrees > 10.0)
		{
			// For significant slopes, deflect wind direction slightly toward downslope direction
			double AngleDiff = AspectDegrees - InputDirection;
			while (AngleDiff > 180.0) AngleDiff -= 360.0;
			while (AngleDiff < -180.0) AngleDiff += 360.0;

			// Deflect up to 30 degrees based on slope steepness
			double DeflectionFactor = FMath::Clamp(SlopeDegrees / 45.0, 0.0, 1.0);
			DirectionModifier = AngleDiff * DeflectionFactor * 0.3; // 30% deflection max
		}

		float FinalDirection = InputDirection + DirectionModifier;
		while (FinalDirection < 0.0f) FinalDirection += 360.0f;
		while (FinalDirection >= 360.0f) FinalDirection -= 360.0f;

		// Store results
		OutVelocityGrid(Row, Col) = FinalVelocity;
		OutAngleGrid(Row, Col) = FinalDirection;
	};

	// Process all cells
	const int32 TotalCells = Width * Height;
	if (Parameters.bUseMultiThreading)
	{
		ParallelFor(TotalCells, ComputeWindForCell);
	}
	else
	{
		for (int32 i = 0; i < TotalCells; ++i)
		{
			ComputeWindForCell(i);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: Terrain-based wind computation complete"));
	UE_LOG(LogTemp, Warning, TEXT("ShiftyMapper: Using simplified wind model. For production, integrate full WindNinja solver."));

	return true;
}

UTexture2D* FWindFlowmapGenerator::ConvertToFlowmapTexture(
	const FShiftyFloatGrid& VelocityGrid,
	const FShiftyFloatGrid& AngleGrid,
	FString& OutErrorMessage)
{
	return FTextureUtils::CreateFlowmapTexture(VelocityGrid, AngleGrid, OutErrorMessage);
}

double FWindFlowmapGenerator::GetRoughnessForVegetation(EShiftyVegetationType VegType) const
{
	// Surface roughness length in meters (affects wind speed at different heights)
	switch (VegType)
	{
	case EShiftyVegetationType::Grass:
		return 0.01; // Short grass

	case EShiftyVegetationType::Brush:
		return 0.10; // Brush/shrubs

	case EShiftyVegetationType::Trees:
		return 0.50; // Forest/trees

	default:
		return 0.10;
	}
}

double FWindFlowmapGenerator::GetRoughnessHeightForVegetation(EShiftyVegetationType VegType) const
{
	// Roughness height in meters
	switch (VegType)
	{
	case EShiftyVegetationType::Grass:
		return 0.10; // 10cm

	case EShiftyVegetationType::Brush:
		return 1.0; // 1m

	case EShiftyVegetationType::Trees:
		return 10.0; // 10m

	default:
		return 1.0;
	}
}
