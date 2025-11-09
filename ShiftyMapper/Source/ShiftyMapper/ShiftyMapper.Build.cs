// Copyright Epic Games, Inc. All Rights Reserved.
// ShiftyMapper - Landscape Heightmap Analysis and Map Generation Plugin

using UnrealBuildTool;
using System.IO;

public class ShiftyMapper : ModuleRules
{
	public ShiftyMapper(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Use Unreal's types instead of STL where possible
		bUseUnity = false; // Disable unity builds for better debugging during development

		// ============================================================================
		// OPTIONAL: Enable full WindNinja solver integration with GDAL
		// The simplified wind model (v1.0) does NOT require GDAL
		// Only enable this when integrating the full mass-consistent solver
		// See WINDNINJA_INTEGRATION.md for details
		// ============================================================================
		bool bEnableFullWindNinjaSolver = false;
		// Uncomment the line below to enable full solver with GDAL:
		// bEnableFullWindNinjaSolver = true;

		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public"),
				Path.Combine(ModuleDirectory, "Public/MapGenerators")
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private"),
				Path.Combine(ModuleDirectory, "Private/MapGenerators"),
				Path.Combine(ModuleDirectory, "Private/WindNinjaCore")
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"RHI"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"ImageWrapper",
				"ImageCore"
			}
		);

		// Compiler settings
		PublicDefinitions.Add("NOMINMAX=1"); // Prevent Windows.h from defining min/max macros

		// ============================================================================
		// GDAL Integration (Optional - only for full WindNinja solver)
		// ============================================================================
		if (bEnableFullWindNinjaSolver)
		{
			PublicDefinitions.Add("WITH_WINDNINJA_FULL_SOLVER=1");

			// Setup GDAL based on platform
			if (Target.Platform == UnrealTargetPlatform.Win64)
			{
				SetupWindowsGDAL(Target);
			}
			else if (Target.Platform == UnrealTargetPlatform.Linux)
			{
				SetupLinuxGDAL(Target);
			}
			else if (Target.Platform == UnrealTargetPlatform.Mac)
			{
				SetupMacGDAL(Target);
			}
		}
		else
		{
			PublicDefinitions.Add("WITH_WINDNINJA_FULL_SOLVER=0");
		}

		// ============================================================================
		// OpenMP for Multi-threading (Optional but recommended)
		// ============================================================================
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// MSVC OpenMP support
			PrivateDefinitions.Add("_OPENMP=1");
			// Note: May need to add /openmp compiler flag
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// GCC/Clang OpenMP support
			PublicAdditionalLibraries.Add("gomp");
			PrivateDefinitions.Add("_OPENMP=1");
		}
	}

	private void SetupWindowsGDAL(ReadOnlyTargetRules Target)
	{
		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));
		string GDALPath = Path.Combine(ThirdPartyPath, "GDAL", "Win64");
		string GDALIncludePath = Path.Combine(GDALPath, "include");
		string GDALLibPath = Path.Combine(GDALPath, "lib");

		PublicIncludePaths.Add(GDALIncludePath);

		// Link against GDAL import library
		PublicAdditionalLibraries.Add(Path.Combine(GDALLibPath, "gdal.lib"));

		// Delay-load the DLL for better load time management
		PublicDelayLoadDLLs.Add("gdal.dll");

		// Ensure the DLL is staged with the build
		RuntimeDependencies.Add(Path.Combine(GDALPath, "bin", "gdal.dll"));
	}

	private void SetupLinuxGDAL(ReadOnlyTargetRules Target)
	{
		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));
		string GDALPath = Path.Combine(ThirdPartyPath, "GDAL", "Linux");

		// Check if we have bundled GDAL, otherwise use system GDAL
		if (Directory.Exists(GDALPath))
		{
			string GDALIncludePath = Path.Combine(GDALPath, "include");
			string GDALLibPath = Path.Combine(GDALPath, "lib");

			PublicIncludePaths.Add(GDALIncludePath);
			PublicAdditionalLibraries.Add(Path.Combine(GDALLibPath, "libgdal.a"));
		}
		else
		{
			// Use system GDAL
			PublicSystemLibraries.Add("gdal");
		}
	}

	private void SetupMacGDAL(ReadOnlyTargetRules Target)
	{
		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty"));
		string GDALPath = Path.Combine(ThirdPartyPath, "GDAL", "Mac");

		// Check if we have bundled GDAL, otherwise use system GDAL
		if (Directory.Exists(GDALPath))
		{
			string GDALIncludePath = Path.Combine(GDALPath, "include");
			string GDALLibPath = Path.Combine(GDALPath, "lib");

			PublicIncludePaths.Add(GDALIncludePath);
			PublicAdditionalLibraries.Add(Path.Combine(GDALLibPath, "libgdal.a"));
		}
		else
		{
			// Use system GDAL (e.g., from Homebrew)
			PublicSystemLibraries.Add("gdal");
		}
	}
}
