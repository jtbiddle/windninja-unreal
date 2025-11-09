// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#include "LandscapeInputUtils.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TextureResource.h"

bool ULandscapeInputUtils::ExtractLandscapeHeightmap(
	ALandscape* Landscape,
	UTexture2D*& OutHeightmap,
	FString& OutErrorMessage)
{
	if (!Landscape)
	{
		OutErrorMessage = TEXT("Landscape is null");
		return false;
	}

	OutHeightmap = CreateHeightmapFromLandscape(Landscape, 0);

	if (!OutHeightmap)
	{
		OutErrorMessage = TEXT("Failed to create heightmap from landscape");
		return false;
	}

	return true;
}

TArray<ALandscape*> ULandscapeInputUtils::GetAllLandscapesInWorld(const UObject* WorldContext)
{
	TArray<ALandscape*> Landscapes;

	if (!WorldContext)
	{
		return Landscapes;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return Landscapes;
	}

	for (TActorIterator<ALandscape> It(World); It; ++It)
	{
		Landscapes.Add(*It);
	}

	return Landscapes;
}

UTexture2D* ULandscapeInputUtils::CreateHeightmapFromLandscape(ALandscape* Landscape, int32 Resolution)
{
	if (!Landscape)
	{
		return nullptr;
	}

	// Get landscape bounds and component info
	FVector Min, Max;
	GetLandscapeBounds(Landscape, Min, Max);

	// Get all landscape components
	TArray<ULandscapeComponent*> Components;
	Landscape->ForEachComponent([&Components](ULandscapeComponent* Component)
	{
		if (Component)
		{
			Components.Add(Component);
		}
	});

	if (Components.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Landscape has no components"));
		return nullptr;
	}

	// Calculate total heightmap size
	int32 ComponentSizeQuads = Landscape->ComponentSizeQuads;
	int32 SubsectionSizeQuads = Landscape->SubsectionSizeQuads;
	int32 ComponentSizeVerts = ComponentSizeQuads + 1;

	// Find extents in component grid
	int32 MinX = MAX_int32, MinY = MAX_int32;
	int32 MaxX = MIN_int32, MaxY = MIN_int32;

	for (ULandscapeComponent* Component : Components)
	{
		FIntPoint ComponentKey = Component->GetSectionBase() / Component->ComponentSizeQuads;
		MinX = FMath::Min(MinX, ComponentKey.X);
		MinY = FMath::Min(MinY, ComponentKey.Y);
		MaxX = FMath::Max(MaxX, ComponentKey.X);
		MaxY = FMath::Max(MaxY, ComponentKey.Y);
	}

	int32 NumComponentsX = (MaxX - MinX) + 1;
	int32 NumComponentsY = (MaxY - MinY) + 1;

	int32 TotalWidth = NumComponentsX * ComponentSizeVerts;
	int32 TotalHeight = NumComponentsY * ComponentSizeVerts;

	// Create heightmap data array
	TArray<uint16> HeightmapData;
	HeightmapData.SetNumZeroed(TotalWidth * TotalHeight);

	// Read each component's heightmap data
	FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());

	for (int32 Y = 0; Y < TotalHeight; ++Y)
	{
		for (int32 X = 0; X < TotalWidth; ++X)
		{
			// Calculate world position
			int32 ComponentX = X / ComponentSizeVerts;
			int32 ComponentY = Y / ComponentSizeVerts;
			int32 LocalX = X % ComponentSizeVerts;
			int32 LocalY = Y % ComponentSizeVerts;

			// Get height at this position
			uint16 Height = LandscapeEdit.GetHeight(X + MinX * ComponentSizeVerts, Y + MinY * ComponentSizeVerts);
			HeightmapData[Y * TotalWidth + X] = Height;
		}
	}

	// Create texture from heightmap data
	UTexture2D* HeightmapTexture = UTexture2D::CreateTransient(TotalWidth, TotalHeight, PF_G16);
	if (!HeightmapTexture)
	{
		return nullptr;
	}

	// Copy data to texture
	FTexture2DMipMap& Mip = HeightmapTexture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, HeightmapData.GetData(), HeightmapData.Num() * sizeof(uint16));
	Mip.BulkData.Unlock();

	// Update texture
	HeightmapTexture->UpdateResource();

	return HeightmapTexture;
}

void ULandscapeInputUtils::GetLandscapeBounds(ALandscape* Landscape, FVector& OutMin, FVector& OutMax)
{
	if (!Landscape)
	{
		OutMin = OutMax = FVector::ZeroVector;
		return;
	}

	FBox Bounds = Landscape->GetComponentsBoundingBox(true);
	OutMin = Bounds.Min;
	OutMax = Bounds.Max;
}

float ULandscapeInputUtils::GetLandscapeCellSize(ALandscape* Landscape)
{
	if (!Landscape)
	{
		return 100.0f; // Default
	}

	FVector Scale = Landscape->GetActorScale3D();
	return Scale.X; // Landscape scale X/Y should be the same
}

bool ULandscapeInputUtils::ReadComponentHeightData(
	ULandscapeComponent* Component,
	TArray<uint16>& OutHeightData,
	int32& OutSizeX,
	int32& OutSizeY)
{
	if (!Component)
	{
		return false;
	}

	// Get heightmap texture
	UTexture2D* HeightmapTexture = Component->GetHeightmap();
	if (!HeightmapTexture)
	{
		return false;
	}

	// Read texture data
	FTexture2DMipMap& Mip = HeightmapTexture->GetPlatformData()->Mips[0];
	OutSizeX = Mip.SizeX;
	OutSizeY = Mip.SizeY;

	const uint16* Data = static_cast<const uint16*>(Mip.BulkData.LockReadOnly());
	OutHeightData.SetNumUninitialized(OutSizeX * OutSizeY);
	FMemory::Memcpy(OutHeightData.GetData(), Data, OutHeightData.Num() * sizeof(uint16));
	Mip.BulkData.Unlock();

	return true;
}
