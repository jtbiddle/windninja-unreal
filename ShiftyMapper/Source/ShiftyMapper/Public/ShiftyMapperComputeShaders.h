// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameters.h"
#include "RenderGraphUtils.h"

/**
 * Base compute shader for terrain analysis
 */
class SHIFTYMAPPER_API FShiftyMapperComputeShaderBase : public FGlobalShader
{
public:
	FShiftyMapperComputeShaderBase() = default;
	FShiftyMapperComputeShaderBase(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE"), 8);
	}
};

/**
 * Curvature calculation compute shader
 */
class SHIFTYMAPPER_API FCurvatureComputeShader : public FShiftyMapperComputeShaderBase
{
	DECLARE_GLOBAL_SHADER(FCurvatureComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FCurvatureComputeShader, FShiftyMapperComputeShaderBase);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, InputHeightmap)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputCurvature)
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, CellSize)
		SHADER_PARAMETER(float, Scale)
		SHADER_PARAMETER(uint32, bCalculatePlan)
		SHADER_PARAMETER(uint32, bCalculateProfile)
	END_SHADER_PARAMETER_STRUCT()
};

/**
 * Roughness calculation compute shader
 */
class SHIFTYMAPPER_API FRoughnessComputeShader : public FShiftyMapperComputeShaderBase
{
	DECLARE_GLOBAL_SHADER(FRoughnessComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FRoughnessComputeShader, FShiftyMapperComputeShaderBase);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, InputHeightmap)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputRoughness)
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(int32, WindowSize)
		SHADER_PARAMETER(uint32, Method)
	END_SHADER_PARAMETER_STRUCT()
};

/**
 * Erosion simulation compute shader
 */
class SHIFTYMAPPER_API FErosionComputeShader : public FShiftyMapperComputeShaderBase
{
	DECLARE_GLOBAL_SHADER(FErosionComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FErosionComputeShader, FShiftyMapperComputeShaderBase);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, InputHeightmap)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, PrecipitationMap)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputHeightmap)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputErosion)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputDeposition)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputFlow)
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, CellSize)
		SHADER_PARAMETER(float, ErosionRate)
		SHADER_PARAMETER(float, DepositionRate)
		SHADER_PARAMETER(float, EvaporationRate)
		SHADER_PARAMETER(float, MinSlope)
		SHADER_PARAMETER(uint32, RandomSeed)
	END_SHADER_PARAMETER_STRUCT()
};

/**
 * Sunlight/insolation compute shader
 */
class SHIFTYMAPPER_API FSunlightComputeShader : public FShiftyMapperComputeShaderBase
{
	DECLARE_GLOBAL_SHADER(FSunlightComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FSunlightComputeShader, FShiftyMapperComputeShaderBase);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<float>, InputHeightmap)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float>, OutputSunlight)
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, CellSize)
		SHADER_PARAMETER(FVector3f, SunDirection)
		SHADER_PARAMETER(float, MaxShadowDistance)
		SHADER_PARAMETER(uint32, bCalculateShadows)
		SHADER_PARAMETER(uint32, bCalculateDiffuse)
		SHADER_PARAMETER(float, AtmosphericScattering)
	END_SHADER_PARAMETER_STRUCT()
};

/**
 * Helper class for dispatching compute shaders
 */
class SHIFTYMAPPER_API FShiftyMapperComputeHelper
{
public:
	/** Run curvature calculation on GPU */
	static void ComputeCurvature(
		FRHICommandListImmediate& RHICmdList,
		UTexture2D* InputHeightmap,
		UTexture2D* OutputCurvature,
		float CellSize,
		float Scale,
		bool bCalculatePlan,
		bool bCalculateProfile
	);

	/** Run roughness calculation on GPU */
	static void ComputeRoughness(
		FRHICommandListImmediate& RHICmdList,
		UTexture2D* InputHeightmap,
		UTexture2D* OutputRoughness,
		int32 WindowSize,
		int32 Method
	);

	/** Run erosion simulation step on GPU */
	static void ComputeErosionStep(
		FRHICommandListImmediate& RHICmdList,
		UTexture2D* InputHeightmap,
		UTexture2D* PrecipitationMap,
		UTexture2D* OutputHeightmap,
		UTexture2D* OutputErosion,
		UTexture2D* OutputDeposition,
		UTexture2D* OutputFlow,
		float CellSize,
		float ErosionRate,
		float DepositionRate,
		float EvaporationRate,
		float MinSlope,
		uint32 RandomSeed
	);

	/** Run sunlight calculation on GPU */
	static void ComputeSunlight(
		FRHICommandListImmediate& RHICmdList,
		UTexture2D* InputHeightmap,
		UTexture2D* OutputSunlight,
		float CellSize,
		const FVector& SunDirection,
		float MaxShadowDistance,
		bool bCalculateShadows,
		bool bCalculateDiffuse,
		float AtmosphericScattering
	);

private:
	/** Create a transient render target texture */
	static UTexture2D* CreateTransientRT(int32 Width, int32 Height, EPixelFormat Format);
};
