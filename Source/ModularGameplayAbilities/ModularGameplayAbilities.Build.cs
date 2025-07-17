// Author: Tom Werner (MajorT), 2025

using UnrealBuildTool;

public class ModularGameplayAbilities : ModuleRules
{
	public ModularGameplayAbilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new[] 
		{ 
			"Core",
			"GameFeatures",
			"GameplayAbilities",
			"GameplayTasks",
			"GameplayTags",
		});
		
		PrivateDependencyModuleNames.AddRange(new[]
		{ 
			"CoreUObject", 
			"Engine",
			"NetCore",
			"AIModule",
		});
	}
}
