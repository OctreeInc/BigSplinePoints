using UnrealBuildTool;

public class BigSplinePoints : ModuleRules
{
    public BigSplinePoints(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "DeveloperSettings"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate",
                "SlateCore",
                "InputCore",
                "Projects",       // IPluginManager (resolve Resources dir for the custom icon)
                "UnrealEd",       // ULevelEditorViewportSettings, GEditor
                "ToolMenus",      // UToolMenus viewport toolbar extension
                "Landscape"       // ULandscapeSettings (SplineIconScale)
            }
        );
    }
}
