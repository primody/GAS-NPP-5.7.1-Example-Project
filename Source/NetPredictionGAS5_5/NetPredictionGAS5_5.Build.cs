// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetPredictionGAS5_5 : ModuleRules
{
	public NetPredictionGAS5_5(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "NetworkPrediction" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
