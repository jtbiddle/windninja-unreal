// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#include "ShiftyMapperModule.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "FShiftyMapperModule"

void FShiftyMapperModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper module starting up..."));

	// Load GDAL library
	if (LoadGDAL())
	{
		UE_LOG(LogTemp, Log, TEXT("ShiftyMapper: GDAL library loaded successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShiftyMapper: Failed to load GDAL library - some features may not work"));
	}

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper module started successfully"));
}

void FShiftyMapperModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper module shutting down..."));

	// Unload GDAL
	UnloadGDAL();

	UE_LOG(LogTemp, Log, TEXT("ShiftyMapper module shut down successfully"));
}

bool FShiftyMapperModule::LoadGDAL()
{
#if PLATFORM_WINDOWS
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("ShiftyMapper")->GetBaseDir();

	// Construct path to GDAL DLL
	FString GDALLibPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/GDAL/Win64/bin/gdal.dll"));

	// Load the DLL
	GDALDllHandle = FPlatformProcess::GetDllHandle(*GDALLibPath);

	if (GDALDllHandle == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load GDAL DLL from: %s"), *GDALLibPath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GDAL DLL loaded from: %s"), *GDALLibPath);
	return true;

#elif PLATFORM_LINUX || PLATFORM_MAC
	// On Linux and Mac, we'll link against system GDAL or bundled static library
	// For now, assume GDAL is available through standard linkage
	UE_LOG(LogTemp, Log, TEXT("Using system GDAL library"));
	return true;

#else
	UE_LOG(LogTemp, Warning, TEXT("GDAL not configured for this platform"));
	return false;
#endif
}

void FShiftyMapperModule::UnloadGDAL()
{
#if PLATFORM_WINDOWS
	if (GDALDllHandle != nullptr)
	{
		FPlatformProcess::FreeDllHandle(GDALDllHandle);
		GDALDllHandle = nullptr;
		UE_LOG(LogTemp, Log, TEXT("GDAL DLL unloaded"));
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShiftyMapperModule, ShiftyMapper)
