// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkCompulsory : ModuleRules
{
	public NetworkCompulsory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"NetworkCompulsory",
			"NetworkCompulsory/Variant_Platforming",
			"NetworkCompulsory/Variant_Platforming/Animation",
			"NetworkCompulsory/Variant_Combat",
			"NetworkCompulsory/Variant_Combat/AI",
			"NetworkCompulsory/Variant_Combat/Animation",
			"NetworkCompulsory/Variant_Combat/Gameplay",
			"NetworkCompulsory/Variant_Combat/Interfaces",
			"NetworkCompulsory/Variant_Combat/UI",
			"NetworkCompulsory/Variant_SideScrolling",
			"NetworkCompulsory/Variant_SideScrolling/AI",
			"NetworkCompulsory/Variant_SideScrolling/Gameplay",
			"NetworkCompulsory/Variant_SideScrolling/Interfaces",
			"NetworkCompulsory/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
