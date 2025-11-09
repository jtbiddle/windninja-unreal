// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#include "TextureUtils.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "RenderUtils.h"
#include "ImageCore.h"

bool FTextureUtils::ExtractHeightmapData(
	UTexture2D* HeightmapTexture,
	FShiftyDoubleGrid& OutGrid,
	double CellSize,
	FString& OutErrorMessage)
{
	if (!ValidateHeightmapTexture(HeightmapTexture, OutErrorMessage))
	{
		return false;
	}

	TArray<float> PixelData;
	int32 Width, Height;

	if (!GetTexturePixelData(HeightmapTexture, PixelData, Width, Height))
	{
		OutErrorMessage = TEXT("Failed to extract pixel data from heightmap texture");
		return false;
	}

	// Initialize the grid
	OutGrid = FShiftyDoubleGrid(Width, Height, 0.0, 0.0, CellSize, -9999.0);

	// Copy pixel data to grid (flip Y axis as textures are stored top-down)
	for (int32 Row = 0; Row < Height; ++Row)
	{
		for (int32 Col = 0; Col < Width; ++Col)
		{
			int32 TextureIndex = (Height - 1 - Row) * Width + Col; // Flip Y
			OutGrid(Row, Col) = static_cast<double>(PixelData[TextureIndex]);
		}
	}

	return true;
}

UTexture2D* FTextureUtils::CreateFlowmapTexture(
	const FShiftyFloatGrid& VelocityGrid,
	const FShiftyFloatGrid& AngleGrid,
	FString& OutErrorMessage)
{
	if (VelocityGrid.NumRows != AngleGrid.NumRows || VelocityGrid.NumCols != AngleGrid.NumCols)
	{
		OutErrorMessage = TEXT("Velocity and angle grids have mismatched dimensions");
		return nullptr;
	}

	const int32 Width = VelocityGrid.NumCols;
	const int32 Height = VelocityGrid.NumRows;

	// Normalize velocity for magnitude encoding
	FShiftyFloatGrid NormalizedVelocity = VelocityGrid;
	float MinVel, MaxVel;
	NormalizeGrid(NormalizedVelocity, MinVel, MaxVel);

	// Create RGBA8 pixel data
	TArray<uint8> PixelData;
	PixelData.SetNumUninitialized(Width * Height * 4);

	for (int32 Row = 0; Row < Height; ++Row)
	{
		for (int32 Col = 0; Col < Width; ++Col)
		{
			float Angle = AngleGrid(Row, Col);
			float Velocity = NormalizedVelocity(Row, Col);

			// Convert angle to normalized direction vector
			float DirX, DirY;
			AngleToVector(Angle, DirX, DirY);

			// Remap from [-1, 1] to [0, 1] for texture encoding
			float TexR = RemapToTexture(DirX);
			float TexG = RemapToTexture(DirY);
			float TexB = Velocity; // Magnitude (already normalized)
			float TexA = 1.0f;

			// Write to pixel data (flip Y axis)
			int32 PixelIndex = ((Height - 1 - Row) * Width + Col) * 4;
			PixelData[PixelIndex + 0] = static_cast<uint8>(FMath::Clamp(TexR * 255.0f, 0.0f, 255.0f));
			PixelData[PixelIndex + 1] = static_cast<uint8>(FMath::Clamp(TexG * 255.0f, 0.0f, 255.0f));
			PixelData[PixelIndex + 2] = static_cast<uint8>(FMath::Clamp(TexB * 255.0f, 0.0f, 255.0f));
			PixelData[PixelIndex + 3] = static_cast<uint8>(FMath::Clamp(TexA * 255.0f, 0.0f, 255.0f));
		}
	}

	return CreateTextureFromRGBA8(Width, Height, PixelData, TEXT("WindFlowmap"));
}

UTexture2D* FTextureUtils::CreateGrayscaleTexture(
	const FShiftyFloatGrid& Grid,
	FString& OutErrorMessage)
{
	const int32 Width = Grid.NumCols;
	const int32 Height = Grid.NumRows;

	// Normalize the grid
	FShiftyFloatGrid NormalizedGrid = Grid;
	float MinVal, MaxVal;
	NormalizeGrid(NormalizedGrid, MinVal, MaxVal);

	// Create RGBA8 pixel data (grayscale in all channels)
	TArray<uint8> PixelData;
	PixelData.SetNumUninitialized(Width * Height * 4);

	for (int32 Row = 0; Row < Height; ++Row)
	{
		for (int32 Col = 0; Col < Width; ++Col)
		{
			float Value = NormalizedGrid(Row, Col);
			uint8 ByteValue = static_cast<uint8>(FMath::Clamp(Value * 255.0f, 0.0f, 255.0f));

			// Write to pixel data (flip Y axis)
			int32 PixelIndex = ((Height - 1 - Row) * Width + Col) * 4;
			PixelData[PixelIndex + 0] = ByteValue; // R
			PixelData[PixelIndex + 1] = ByteValue; // G
			PixelData[PixelIndex + 2] = ByteValue; // B
			PixelData[PixelIndex + 3] = 255;       // A
		}
	}

	return CreateTextureFromRGBA8(Width, Height, PixelData, TEXT("GrayscaleMap"));
}

void FTextureUtils::NormalizeGrid(FShiftyFloatGrid& Grid, float& OutMin, float& OutMax)
{
	if (Grid.GetArraySize() == 0)
	{
		OutMin = OutMax = 0.0f;
		return;
	}

	OutMin = FLT_MAX;
	OutMax = -FLT_MAX;

	// Find min and max, ignoring no-data values
	for (int32 i = 0; i < Grid.GetArraySize(); ++i)
	{
		float Value = Grid.Data[i];
		if (Value != Grid.NoDataValue)
		{
			OutMin = FMath::Min(OutMin, Value);
			OutMax = FMath::Max(OutMax, Value);
		}
	}

	// Normalize to [0, 1]
	float Range = OutMax - OutMin;
	if (Range > SMALL_NUMBER)
	{
		for (int32 i = 0; i < Grid.GetArraySize(); ++i)
		{
			if (Grid.Data[i] != Grid.NoDataValue)
			{
				Grid.Data[i] = (Grid.Data[i] - OutMin) / Range;
			}
			else
			{
				Grid.Data[i] = 0.0f; // Replace no-data with 0
			}
		}
	}
	else
	{
		// All values are the same
		for (int32 i = 0; i < Grid.GetArraySize(); ++i)
		{
			Grid.Data[i] = 0.5f;
		}
	}
}

bool FTextureUtils::GetTexturePixelData(
	UTexture2D* Texture,
	TArray<float>& OutPixelData,
	int32& OutWidth,
	int32& OutHeight)
{
	if (!Texture)
	{
		return false;
	}

	OutWidth = Texture->GetSizeX();
	OutHeight = Texture->GetSizeY();
	OutPixelData.SetNumUninitialized(OutWidth * OutHeight);

	// Get the platform data
	FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() == 0)
	{
		return false;
	}

	// Lock the first mip level for reading
	const FTexture2DMipMap& Mip = PlatformData->Mips[0];
	const void* RawData = Mip.BulkData.LockReadOnly();

	if (!RawData)
	{
		return false;
	}

	// Convert based on pixel format
	EPixelFormat PixelFormat = PlatformData->PixelFormat;

	if (PixelFormat == PF_G8 || PixelFormat == PF_R8)
	{
		// 8-bit grayscale
		const uint8* ByteData = static_cast<const uint8*>(RawData);
		for (int32 i = 0; i < OutWidth * OutHeight; ++i)
		{
			OutPixelData[i] = ByteData[i] / 255.0f;
		}
	}
	else if (PixelFormat == PF_G16 || PixelFormat == PF_R16_UINT || PixelFormat == PF_R16F)
	{
		// 16-bit grayscale
		const uint16* ShortData = static_cast<const uint16*>(RawData);
		for (int32 i = 0; i < OutWidth * OutHeight; ++i)
		{
			OutPixelData[i] = ShortData[i] / 65535.0f;
		}
	}
	else if (PixelFormat == PF_B8G8R8A8 || PixelFormat == PF_R8G8B8A8)
	{
		// 8-bit RGBA - use R channel
		const FColor* ColorData = static_cast<const FColor*>(RawData);
		for (int32 i = 0; i < OutWidth * OutHeight; ++i)
		{
			OutPixelData[i] = ColorData[i].R / 255.0f;
		}
	}
	else if (PixelFormat == PF_FloatRGBA || PixelFormat == PF_A32B32G32R32F)
	{
		// Float RGBA - use R channel
		const FLinearColor* FloatData = static_cast<const FLinearColor*>(RawData);
		for (int32 i = 0; i < OutWidth * OutHeight; ++i)
		{
			OutPixelData[i] = FloatData[i].R;
		}
	}
	else
	{
		Mip.BulkData.Unlock();
		return false;
	}

	Mip.BulkData.Unlock();
	return true;
}

UTexture2D* FTextureUtils::CreateTextureFromRGBA8(
	int32 Width,
	int32 Height,
	const TArray<uint8>& PixelData,
	const FString& TextureName)
{
	if (Width <= 0 || Height <= 0 || PixelData.Num() != Width * Height * 4)
	{
		return nullptr;
	}

	// Create the texture
	UTexture2D* NewTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
	if (!NewTexture)
	{
		return nullptr;
	}

	// Set texture properties
	NewTexture->MipGenSettings = TMGS_NoMipmaps;
	NewTexture->SRGB = false; // Flowmaps should not be in sRGB space
	NewTexture->CompressionSettings = TC_VectorDisplacementmap; // Good for flowmaps
	NewTexture->Filter = TF_Bilinear;
	NewTexture->AddressX = TA_Clamp;
	NewTexture->AddressY = TA_Clamp;

	// Update the texture data
	FTexture2DMipMap& Mip = NewTexture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, PixelData.GetData(), PixelData.Num());
	Mip.BulkData.Unlock();

	// Update the texture resource
	NewTexture->UpdateResource();

	return NewTexture;
}

bool FTextureUtils::ValidateHeightmapTexture(UTexture2D* Texture, FString& OutErrorMessage)
{
	if (!Texture)
	{
		OutErrorMessage = TEXT("Heightmap texture is null");
		return false;
	}

	int32 Width = Texture->GetSizeX();
	int32 Height = Texture->GetSizeY();

	if (Width <= 0 || Height <= 0)
	{
		OutErrorMessage = FString::Printf(TEXT("Invalid texture dimensions: %dx%d"), Width, Height);
		return false;
	}

	if (Width < 10 || Height < 10)
	{
		OutErrorMessage = FString::Printf(TEXT("Texture too small for wind simulation (minimum 10x10): %dx%d"), Width, Height);
		return false;
	}

	if (Width > 8192 || Height > 8192)
	{
		OutErrorMessage = FString::Printf(TEXT("Texture too large for wind simulation (maximum 8192x8192): %dx%d"), Width, Height);
		return false;
	}

	return true;
}

void FTextureUtils::AngleToVector(float AngleDegrees, float& OutX, float& OutY)
{
	// Convert from meteorological convention (degrees from North, clockwise)
	// to Cartesian coordinates (0 = East, counter-clockwise)

	// Meteorological: 0° = North, 90° = East, 180° = South, 270° = West
	// Cartesian: 0° = East, 90° = North, 180° = West, 270° = South

	float CartesianAngle = 90.0f - AngleDegrees;
	float AngleRadians = FMath::DegreesToRadians(CartesianAngle);

	OutX = FMath::Cos(AngleRadians);
	OutY = FMath::Sin(AngleRadians);
}
