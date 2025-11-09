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

		// Platform-specific setup
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

		// Compiler settings for WindNinja core
		PublicDefinitions.Add("NOMINMAX=1"); // Prevent Windows.h from defining min/max macros

		// Enable OpenMP for multi-threading if available
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// MSVC OpenMP support
			PrivateDefinitions.Add("_OPENMP=1");
			// Note: Add /openmp flag in actual compilation
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
