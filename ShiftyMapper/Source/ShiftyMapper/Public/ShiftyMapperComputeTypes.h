// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Engine/Texture2D.h"
#include "ShiftyMapperTypes.h"
#include "ShiftyMapperComputeTypes.generated.h"

/**
 * Parameters for curvature calculation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FCurvatureParams
{
	GENERATED_BODY()

	/** Scale factor for curvature values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curvature")
	float Scale = 1.0f;

	/** Smoothing iterations to reduce noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curvature")
	int32 SmoothingIterations = 0;

	/** Calculate plan curvature (curvature perpendicular to slope) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curvature")
	bool bCalculatePlanCurvature = true;

	/** Calculate profile curvature (curvature parallel to slope) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curvature")
	bool bCalculateProfileCurvature = true;
};

/**
 * Method for calculating terrain roughness
 */
UENUM(BlueprintType)
enum class ERoughnessMethod : uint8
{
	StandardDeviation UMETA(DisplayName = "Standard Deviation"),
	RangeOfValues UMETA(DisplayName = "Range of Values"),
	AreaRatio UMETA(DisplayName = "Area Ratio")
};

/**
 * Parameters for roughness calculation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FRoughnessParams
{
	GENERATED_BODY()

	/** Window size for roughness calculation (in pixels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roughness", meta = (ClampMin = "1", ClampMax = "32"))
	int32 WindowSize = 3;

	/** Method for calculating roughness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Roughness")
	ERoughnessMethod Method = ERoughnessMethod::StandardDeviation;
};

/**
 * Parameters for erosion simulation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FErosionParams
{
	GENERATED_BODY()

	/** Number of simulation iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "10000"))
	int32 Iterations = 1000;

	/** Number of erosion particles per iteration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "100"))
	int32 ParticlesPerIteration = 1;

	/** Precipitation map (optional, can be null for uniform rain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
	TObjectPtr<UTexture2D> PrecipitationMap = nullptr;

	/** Base erosion rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ErosionRate = 0.3f;

	/** Deposition rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DepositionRate = 0.3f;

	/** Evaporation rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EvaporationRate = 0.01f;

	/** Minimum slope for erosion to occur */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MinSlope = 0.01f;

	/** Sediment capacity curve (slope -> capacity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Curves")
	TObjectPtr<UCurveFloat> SedimentCapacityCurve = nullptr;

	/** Erosion strength curve (slope -> erosion multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Curves")
	TObjectPtr<UCurveFloat> ErosionStrengthCurve = nullptr;

	/** Output erosion amount map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Output")
	bool bOutputErosionMap = true;

	/** Output deposition amount map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Output")
	bool bOutputDepositionMap = true;

	/** Output water flow map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion|Output")
	bool bOutputFlowMap = true;
};

/**
 * Parameters for groundwater simulation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FGroundwaterParams
{
	GENERATED_BODY()

	/** Flow accumulation map from erosion simulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groundwater")
	TObjectPtr<UTexture2D> FlowAccumulationMap = nullptr;

	/** Precipitation map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groundwater")
	TObjectPtr<UTexture2D> PrecipitationMap = nullptr;

	/** Soil permeability (0 = impermeable, 1 = fully permeable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groundwater", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoilPermeability = 0.5f;

	/** Water table depth multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groundwater", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float WaterTableDepth = 1.0f;

	/** Permeability curve (height -> permeability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groundwater|Curves")
	TObjectPtr<UCurveFloat> PermeabilityCurve = nullptr;
};

/**
 * Parameters for soil fertility/vegetation mask generation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FSoilFertilityParams
{
	GENERATED_BODY()

	/** Groundwater/moisture map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility")
	TObjectPtr<UTexture2D> GroundwaterMap = nullptr;

	/** Slope map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility")
	TObjectPtr<UTexture2D> SlopeMap = nullptr;

	/** Aspect map */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility")
	TObjectPtr<UTexture2D> AspectMap = nullptr;

	/** Grass suitability curve (slope -> suitability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility|Curves")
	TObjectPtr<UCurveFloat> GrassSlopeCurve = nullptr;

	/** Shrub suitability curve (slope -> suitability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility|Curves")
	TObjectPtr<UCurveFloat> ShrubSlopeCurve = nullptr;

	/** Tree suitability curve (slope -> suitability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility|Curves")
	TObjectPtr<UCurveFloat> TreeSlopeCurve = nullptr;

	/** Moisture requirement for grass */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility|Requirements", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GrassMoistureRequirement = 0.3f;

	/** Moisture requirement for shrubs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility|Requirements", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShrubMoistureRequirement = 0.5f;

	/** Moisture requirement for trees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil Fertility|Requirements", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TreeMoistureRequirement = 0.7f;
};

/**
 * Parameters for sunlight/insolation calculation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FSunlightParams
{
	GENERATED_BODY()

	/** Sun azimuth angle in degrees (0 = North, 90 = East) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float SunAzimuth = 180.0f;

	/** Sun elevation angle in degrees (0 = horizon, 90 = zenith) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SunElevation = 45.0f;

	/** Calculate shadow casting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight")
	bool bCalculateShadows = true;

	/** Maximum shadow ray distance in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight", meta = (ClampMin = "0.0"))
	float MaxShadowDistance = 1000.0f;

	/** Calculate diffuse lighting (based on slope) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight")
	bool bCalculateDiffuse = true;

	/** Atmospheric scattering factor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AtmosphericScattering = 0.2f;

	/** Sun path curve for time-of-day simulation (hour -> sun elevation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight|Curves")
	TObjectPtr<UCurveFloat> SunPathCurve = nullptr;

	/** Time of day for single calculation (if not using sun path curve) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sunlight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDay = 12.0f;
};

/**
 * Parameters for snowfall and melt simulation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FSnowfallParams
{
	GENERATED_BODY()

	/** Temperature map (optional, uses elevation if not provided) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall")
	TObjectPtr<UTexture2D> TemperatureMap = nullptr;

	/** Sunlight/insolation map for melt calculation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall")
	TObjectPtr<UTexture2D> SunlightMap = nullptr;

	/** Base snowfall amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall", meta = (ClampMin = "0.0"))
	float BaseSnowfall = 1.0f;

	/** Temperature lapse rate (degrees C per meter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall")
	float TemperatureLapseRate = -0.0065f;

	/** Base temperature at sea level (degrees C) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall")
	float BaseTemperature = 15.0f;

	/** Melting temperature threshold (degrees C) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall")
	float MeltingTemperature = 0.0f;

	/** Melt rate multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall", meta = (ClampMin = "0.0"))
	float MeltRate = 0.1f;

	/** Snowfall multiplier curve (elevation -> snowfall multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall|Curves")
	TObjectPtr<UCurveFloat> SnowfallElevationCurve = nullptr;

	/** Aspect influence on melt (south-facing = more melt) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snowfall", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AspectMeltInfluence = 0.5f;
};

/**
 * Result from compute-based map generation
 */
USTRUCT(BlueprintType)
struct SHIFTYMAPPER_API FComputeMapResult
{
	GENERATED_BODY()

	/** Primary output texture */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> OutputTexture = nullptr;

	/** Secondary output texture (for multi-channel outputs) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> SecondaryTexture = nullptr;

	/** Tertiary output texture (for tri-channel outputs like erosion) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TObjectPtr<UTexture2D> TertiaryTexture = nullptr;

	/** Success flag */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	/** Error message if failed */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FString ErrorMessage;

	/** Generation time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float GenerationTimeSeconds = 0.0f;

	/** Metadata about the generation */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TMap<FString, FString> Metadata;
};
