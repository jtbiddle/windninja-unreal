// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "ShiftyMapperTypes.generated.h"

/**
 * Vegetation type for wind simulation surface properties
 */
UENUM(BlueprintType)
enum class EShiftyVegetationType : uint8
{
	Grass UMETA(DisplayName = "Grass"),
	Brush UMETA(DisplayName = "Brush"),
	Trees UMETA(DisplayName = "Trees")
};

/**
 * Type of map to generate from heightmap
 */
UENUM(BlueprintType)
enum class EShiftyMapType : uint8
{
	WindFlowmap UMETA(DisplayName = "Wind Flowmap"),
	NormalMap UMETA(DisplayName = "Normal Map"),
	CurvatureMap UMETA(DisplayName = "Curvature Map"),
	ErosionMap UMETA(DisplayName = "Erosion Map"),
	FlowAccumulation UMETA(DisplayName = "Flow Accumulation"),
	SlopeMap UMETA(DisplayName = "Slope Map"),
	AspectMap UMETA(DisplayName = "Aspect Map")
};

/**
 * Interpolation method for resampling
 */
UENUM(BlueprintType)
enum class EShiftyInterpMethod : uint8
{
	NearestNeighbor UMETA(DisplayName = "Nearest Neighbor"),
	Bilinear UMETA(DisplayName = "Bilinear"),
	Bicubic UMETA(DisplayName = "Bicubic")
};

/**
 * Parameters for wind flowmap generation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FWindFlowmapParams
{
	GENERATED_BODY()

	/** Input wind speed in meters per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Parameters", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float WindSpeed = 10.0f;

	/** Input wind direction in degrees (0-360, clockwise from North) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Parameters", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float WindDirection = 270.0f;

	/** Height above vegetation where wind is measured in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Parameters", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float InputWindHeight = 6.1f;

	/** Height above vegetation for output wind in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Parameters", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float OutputWindHeight = 6.1f;

	/** Vegetation type affecting surface roughness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Parameters")
	EShiftyVegetationType VegetationType = EShiftyVegetationType::Trees;

	/** Size of each cell in meters (resolution of simulation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Parameters", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float CellSize = 30.0f;

	/** Number of vertical layers in 3D mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Parameters", meta = (ClampMin = "6", ClampMax = "50"))
	int32 NumVerticalLayers = 20;

	/** Mesh resolution choice affects accuracy vs performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Parameters")
	bool bUseFineResolution = false;

	/** Enable multi-threading for faster computation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bUseMultiThreading = true;

	/** Number of threads to use (0 = auto-detect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0", ClampMax = "64"))
	int32 NumThreads = 0;

	FWindFlowmapParams()
	{
	}
};

/**
 * Result data from map generation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FShiftyMapResult
{
	GENERATED_BODY()

	/** Generated output texture */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> OutputTexture = nullptr;

	/** Whether generation was successful */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	/** Error message if generation failed */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	/** Time taken to generate the map in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float GenerationTimeSeconds = 0.0f;

	/** Additional metadata about the generation */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TMap<FString, FString> Metadata;

	FShiftyMapResult()
		: bSuccess(false)
		, GenerationTimeSeconds(0.0f)
	{
	}
};

/**
 * Grid data structure for 2D spatial data
 * This is an Unreal-friendly version of WindNinja's AsciiGrid
 */
template<typename T>
struct FShiftyGrid2D
{
	/** Grid dimensions */
	int32 NumRows;
	int32 NumCols;

	/** Geographic reference */
	double XLLCorner;
	double YLLCorner;
	double CellSize;
	T NoDataValue;

	/** Grid data stored row-major */
	TArray<T> Data;

	FShiftyGrid2D()
		: NumRows(0)
		, NumCols(0)
		, XLLCorner(0.0)
		, YLLCorner(0.0)
		, CellSize(1.0)
		, NoDataValue(static_cast<T>(-9999))
	{
	}

	FShiftyGrid2D(int32 InNumCols, int32 InNumRows, double InXLL, double InYLL, double InCellSize, T InNoDataValue)
		: NumRows(InNumRows)
		, NumCols(InNumCols)
		, XLLCorner(InXLL)
		, YLLCorner(InYLL)
		, CellSize(InCellSize)
		, NoDataValue(InNoDataValue)
	{
		Data.SetNumZeroed(NumRows * NumCols);
	}

	FORCEINLINE T& operator()(int32 Row, int32 Col)
	{
		check(Row >= 0 && Row < NumRows && Col >= 0 && Col < NumCols);
		return Data[Row * NumCols + Col];
	}

	FORCEINLINE const T& operator()(int32 Row, int32 Col) const
	{
		check(Row >= 0 && Row < NumRows && Col >= 0 && Col < NumCols);
		return Data[Row * NumCols + Col];
	}

	FORCEINLINE bool IsValidIndex(int32 Row, int32 Col) const
	{
		return Row >= 0 && Row < NumRows && Col >= 0 && Col < NumCols;
	}

	FORCEINLINE int32 GetArraySize() const
	{
		return NumRows * NumCols;
	}
};

/** Common typedefs for grid types */
typedef FShiftyGrid2D<float> FShiftyFloatGrid;
typedef FShiftyGrid2D<double> FShiftyDoubleGrid;
typedef FShiftyGrid2D<int32> FShiftyIntGrid;
