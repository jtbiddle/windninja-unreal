// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * ShiftyMapper Module
 *
 * General-purpose landscape heightmap analysis and map generation plugin.
 * Supports generating various terrain analysis maps including:
 * - Wind flowmaps (using WindNinja mass-consistent solver)
 * - Normal maps
 * - Curvature maps
 * - Erosion maps
 * - Flow accumulation maps
 * - Slope and aspect maps
 */
class FShiftyMapperModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FShiftyMapperModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FShiftyMapperModule>("ShiftyMapper");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ShiftyMapper");
	}

private:
#if WITH_WINDNINJA_FULL_SOLVER
	/** Handle to the GDAL DLL (if dynamically loaded) */
	void* GDALDllHandle = nullptr;

	/** Load GDAL library */
	bool LoadGDAL();

	/** Unload GDAL library */
	void UnloadGDAL();
#endif // WITH_WINDNINJA_FULL_SOLVER
};
