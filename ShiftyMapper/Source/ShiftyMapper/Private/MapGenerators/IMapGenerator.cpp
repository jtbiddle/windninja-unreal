// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#include "MapGenerators/IMapGenerator.h"
#include "MapGenerators/WindFlowmapGenerator.h"

TSharedPtr<IMapGenerator> FMapGeneratorFactory::CreateGenerator(EShiftyMapType MapType)
{
	switch (MapType)
	{
	case EShiftyMapType::WindFlowmap:
		return MakeShared<FWindFlowmapGenerator>();

	// Future generators will be added here as they're implemented
	case EShiftyMapType::NormalMap:
	case EShiftyMapType::CurvatureMap:
	case EShiftyMapType::ErosionMap:
	case EShiftyMapType::FlowAccumulation:
	case EShiftyMapType::SlopeMap:
	case EShiftyMapType::AspectMap:
	default:
		UE_LOG(LogTemp, Warning, TEXT("Map type not yet implemented: %d"), static_cast<int32>(MapType));
		return nullptr;
	}
}

TArray<EShiftyMapType> FMapGeneratorFactory::GetSupportedMapTypes()
{
	TArray<EShiftyMapType> SupportedTypes;

	// Add currently implemented types
	SupportedTypes.Add(EShiftyMapType::WindFlowmap);

	// Future types will be added here as implemented

	return SupportedTypes;
}

bool FMapGeneratorFactory::IsMapTypeSupported(EShiftyMapType MapType)
{
	TArray<EShiftyMapType> SupportedTypes = GetSupportedMapTypes();
	return SupportedTypes.Contains(MapType);
}
