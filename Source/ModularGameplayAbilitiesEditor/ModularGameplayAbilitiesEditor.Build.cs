using UnrealBuildTool;

public class ModularGameplayAbilitiesEditor : ModuleRules
{
    public ModularGameplayAbilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "GameplayAbilities",
                "ModularGameplayAbilities",
                "UnrealEd",
                "AssetDefinition",
            }
        );
    }
}