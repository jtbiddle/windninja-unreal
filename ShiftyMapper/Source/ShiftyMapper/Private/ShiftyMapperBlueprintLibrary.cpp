// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#include "ShiftyMapperBlueprintLibrary.h"
#include "MapGenerators/IMapGenerator.h"
#include "MapGenerators/WindFlowmapGenerator.h"

UTexture2D* UShiftyMapperBlueprintLibrary::GenerateWindFlowmap(
	UTexture2D* HeightmapTexture,
	float WindSpeed,
	float WindDirection,
	EShiftyVegetationType VegetationType,
	float InputWindHeight,
	float OutputWindHeight,
	float CellSize,
	bool& bSuccess)
{
	FWindFlowmapParams Params;
	Params.WindSpeed = WindSpeed;
	Params.WindDirection = WindDirection;
	Params.VegetationType = VegetationType;
	Params.InputWindHeight = InputWindHeight;
	Params.OutputWindHeight = OutputWindHeight;
	Params.CellSize = CellSize;

	FString ErrorMessage;
	return GenerateWindFlowmapAdvanced(HeightmapTexture, Params, bSuccess, ErrorMessage);
}

UTexture2D* UShiftyMapperBlueprintLibrary::GenerateWindFlowmapAdvanced(
	UTexture2D* HeightmapTexture,
	const FWindFlowmapParams& Parameters,
	bool& bSuccess,
	FString& ErrorMessage)
{
	if (!HeightmapTexture)
	{
		bSuccess = false;
		ErrorMessage = TEXT("Heightmap texture is null");
		return nullptr;
	}

	// Create the wind flowmap generator
	TSharedPtr<FWindFlowmapGenerator> Generator = MakeShared<FWindFlowmapGenerator>();
	Generator->SetParameters(Parameters);

	// Generate the flowmap
	FShiftyMapResult Result;
	Generator->GenerateWindFlowmap(HeightmapTexture, Parameters, Result);

	bSuccess = Result.bSuccess;
	ErrorMessage = Result.ErrorMessage;

	return Result.OutputTexture;
}

FShiftyMapResult UShiftyMapperBlueprintLibrary::GenerateMapFromHeightmap(
	UTexture2D* HeightmapTexture,
	EShiftyMapType MapType)
{
	FShiftyMapResult Result;

	if (!HeightmapTexture)
	{
		Result.bSuccess = false;
		Result.ErrorMessage = TEXT("Heightmap texture is null");
		return Result;
	}

	// Create the appropriate generator
	TSharedPtr<IMapGenerator> Generator = FMapGeneratorFactory::CreateGenerator(MapType);

	if (!Generator.IsValid())
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("Map type %d is not yet supported"), static_cast<int32>(MapType));
		return Result;
	}

	// Generate the map
	Generator->Generate(HeightmapTexture, Result);

	return Result;
}

bool UShiftyMapperBlueprintLibrary::IsMapTypeSupported(EShiftyMapType MapType)
{
	return FMapGeneratorFactory::IsMapTypeSupported(MapType);
}

TArray<EShiftyMapType> UShiftyMapperBlueprintLibrary::GetSupportedMapTypes()
{
	return FMapGeneratorFactory::GetSupportedMapTypes();
}

FWindFlowmapParams UShiftyMapperBlueprintLibrary::MakeDefaultWindParameters()
{
	return FWindFlowmapParams();
}

FWindFlowmapParams UShiftyMapperBlueprintLibrary::MakeWindParameters(
	float WindSpeed,
	float WindDirection,
	EShiftyVegetationType VegetationType,
	float CellSize)
{
	FWindFlowmapParams Params;
	Params.WindSpeed = WindSpeed;
	Params.WindDirection = WindDirection;
	Params.VegetationType = VegetationType;
	Params.CellSize = CellSize;
	return Params;
}
