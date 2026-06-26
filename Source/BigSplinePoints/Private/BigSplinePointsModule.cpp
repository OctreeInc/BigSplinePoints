// Copyright 2026 Octree, Inc. All Rights Reserved.

#include "BigSplinePointsModule.h"
#include "BigSplinePointsSettings.h"

#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "LandscapeSettings.h"

#include "ToolMenus.h"
#include "ToolMenuSection.h"
#include "ToolMenuEntry.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Brushes/SlateImageBrush.h"
#include "Textures/SlateIcon.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/UnrealType.h"
#include "HAL/IConsoleManager.h"
#include "Containers/Ticker.h"

#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FBigSplinePointsModule"

namespace BigSplinePoints
{
    // Derived-knob scaling from the master Size (extra screen-space pixels on the point handles).
    constexpr float TangentRatio  = 1.0f;    // tangents grow with the points
    constexpr float LineRatio     = 0.12f;   // lines thicken gently
    constexpr float IconGrowth    = 0.10f;   // landscape icon: baseline * (1 + Size * this)

    static TSharedPtr<FSlateStyleSet> GStyleSet;
}

FName FBigSplinePointsModule::GetStyleSetName()
{
    static FName Name(TEXT("BigSplinePointsStyle"));
    return Name;
}

FSlateIcon FBigSplinePointsModule::GetToolbarIcon()
{
    return FSlateIcon(GetStyleSetName(), "BigSplinePoints.ToolbarIcon");
}

void FBigSplinePointsModule::RegisterStyle()
{
    if (BigSplinePoints::GStyleSet.IsValid())
    {
        return;
    }

    BigSplinePoints::GStyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    if (TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BigSplinePoints")))
    {
        BigSplinePoints::GStyleSet->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
    }

    const FVector2D Icon16(16.0f, 16.0f);
    BigSplinePoints::GStyleSet->Set("BigSplinePoints.ToolbarIcon",
        new FSlateVectorImageBrush(
            BigSplinePoints::GStyleSet->RootToContentDir(TEXT("Icons/BigSplinePoints_16"), TEXT(".svg")),
            Icon16));

    FSlateStyleRegistry::RegisterSlateStyle(*BigSplinePoints::GStyleSet);
}

void FBigSplinePointsModule::UnregisterStyle()
{
    if (BigSplinePoints::GStyleSet.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*BigSplinePoints::GStyleSet);
        BigSplinePoints::GStyleSet.Reset();
    }
}

FBigSplinePointsModule& FBigSplinePointsModule::Get()
{
    return FModuleManager::LoadModuleChecked<FBigSplinePointsModule>("BigSplinePoints");
}

void FBigSplinePointsModule::StartupModule()
{
    RegisterStyle();
    CaptureBaselines();

    // Re-apply persisted state (or write clean baselines back when inactive).
    ApplyState();

    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBigSplinePointsModule::RegisterMenus));
}

void FBigSplinePointsModule::ShutdownModule()
{
    if (UObjectInitialized())
    {
        // Restore the user's original settings so the engine never persists our runtime overlay.
        RestoreBaselines();

        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
    }

    UnregisterStyle();
}

bool FBigSplinePointsModule::IsActive() const
{
    const UBigSplinePointsSettings* Settings = GetDefault<UBigSplinePointsSettings>();
    return Settings && Settings->bActive;
}

float FBigSplinePointsModule::GetSize() const
{
    const UBigSplinePointsSettings* Settings = GetDefault<UBigSplinePointsSettings>();
    return Settings ? Settings->Size : 0.0f;
}

void FBigSplinePointsModule::ToggleActive()
{
    UBigSplinePointsSettings* Settings = UBigSplinePointsSettings::Get();
    if (!Settings)
    {
        return;
    }

    Settings->bActive = !Settings->bActive;
    Settings->SaveConfig();
    ApplyState();
}

void FBigSplinePointsModule::SetSize(float InSize)
{
    UBigSplinePointsSettings* Settings = UBigSplinePointsSettings::Get();
    if (!Settings)
    {
        return;
    }

    Settings->Size = FMath::Clamp(InSize, SliderMinSize, SliderMaxSize);
    Settings->bActive = true;
    Settings->SaveConfig();
    ApplyState();
}

void FBigSplinePointsModule::CaptureBaselines()
{
    UBigSplinePointsSettings* Settings = UBigSplinePointsSettings::Get();
    const ULevelEditorViewportSettings* ViewportSettings = GetDefault<ULevelEditorViewportSettings>();
    if (!Settings || !ViewportSettings)
    {
        return;
    }

    const bool bShouldCapture = !Settings->bHasBaselines || !Settings->bActive;
    if (!bShouldCapture)
    {
        return;
    }

    Settings->BaselinePointSize          = ViewportSettings->SelectedSplinePointSizeAdjustment;
    Settings->BaselineTangentSize        = ViewportSettings->SplineTangentHandleSizeAdjustment;
    Settings->BaselineLineThickness      = ViewportSettings->SplineLineThicknessAdjustment;
    Settings->BaselineLandscapeIconScale = GetLandscapeIconScale();
    Settings->bHasBaselines = true;
    Settings->SaveConfig();
}

void FBigSplinePointsModule::ApplyState()
{
    UBigSplinePointsSettings* Settings = UBigSplinePointsSettings::Get();
    ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
    if (!Settings || !ViewportSettings)
    {
        return;
    }

    if (Settings->bActive)
    {
        const float Size = FMath::Clamp(Settings->Size, SliderMinSize, SliderMaxSize);

        // The visualizer reads these values raw (it does not re-clamp), so we are free to exceed the
        // engine's own Editor Preferences slider cap of 20 for genuinely large handles.
        ViewportSettings->SelectedSplinePointSizeAdjustment = Settings->BaselinePointSize + Size;
        ViewportSettings->SplineTangentHandleSizeAdjustment = Settings->BaselineTangentSize + Size * BigSplinePoints::TangentRatio;
        ViewportSettings->SplineLineThicknessAdjustment     = FMath::Max(Settings->BaselineLineThickness + Size * BigSplinePoints::LineRatio, 0.0f);

        SetLandscapeIconScale(FMath::Clamp(
            Settings->BaselineLandscapeIconScale * (1.0f + Size * BigSplinePoints::IconGrowth),
            32.0f, 2048.0f));
    }
    else
    {
        RestoreBaselines();
    }

    // FSplineComponentVisualizer caches its draw batches and only rebuilds them when its cache is
    // dirtied by a spline edit -- NOT when these viewport settings change. So the new handle sizes
    // would not appear until the user next edited the spline. Briefly force the visualizer to rebuild
    // every frame, redraw, then restore the cvar after a moment so a single fresh rebuild picks up the
    // new GrabHandleSize/TangentHandleSize.
    static const TCHAR* RebuildCVarName = TEXT("SplineComponentVisualizer.RebuildPDICacheEveryFrame");
    if (IConsoleVariable* RebuildCVar = IConsoleManager::Get().FindConsoleVariable(RebuildCVarName))
    {
        RebuildCVar->Set(true);
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateLambda([](float) -> bool
            {
                if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(RebuildCVarName))
                {
                    CVar->Set(false);
                }
                return false; // one-shot
            }),
            0.25f);
    }

    if (GEditor)
    {
        GEditor->RedrawLevelEditingViewports(true);
    }
}

void FBigSplinePointsModule::RestoreBaselines()
{
    const UBigSplinePointsSettings* Settings = GetDefault<UBigSplinePointsSettings>();
    ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
    if (!Settings || !ViewportSettings || !Settings->bHasBaselines)
    {
        return;
    }

    ViewportSettings->SelectedSplinePointSizeAdjustment = Settings->BaselinePointSize;
    ViewportSettings->SplineTangentHandleSizeAdjustment = Settings->BaselineTangentSize;
    ViewportSettings->SplineLineThicknessAdjustment     = Settings->BaselineLineThickness;
    SetLandscapeIconScale(Settings->BaselineLandscapeIconScale);
}

void FBigSplinePointsModule::SetLandscapeIconScale(float Value)
{
    ULandscapeSettings* LandscapeSettings = GetMutableDefault<ULandscapeSettings>();
    if (!LandscapeSettings)
    {
        return;
    }

    // SplineIconScale is a protected UPROPERTY with only a public getter, so set it via reflection.
    static FFloatProperty* IconScaleProperty =
        FindFProperty<FFloatProperty>(ULandscapeSettings::StaticClass(), TEXT("SplineIconScale"));
    if (IconScaleProperty)
    {
        IconScaleProperty->SetPropertyValue_InContainer(LandscapeSettings, Value);
    }
}

float FBigSplinePointsModule::GetLandscapeIconScale() const
{
    const ULandscapeSettings* LandscapeSettings = GetDefault<ULandscapeSettings>();
    return LandscapeSettings ? LandscapeSettings->GetSplineIconScale() : 125.0f;
}

void FBigSplinePointsModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.ViewportToolbar");
    if (!Toolbar)
    {
        return;
    }

    FToolMenuSection& Section = Toolbar->FindOrAddSection("Left");

    FToolMenuEntry Entry = FToolMenuEntry::InitSubMenu(
        "BigSplinePoints",
        LOCTEXT("BigSplinePointsLabel", "Big Spline Points"),
        LOCTEXT("BigSplinePointsTooltip",
            "Enlarge spline control points and tangent handles so they are easier to grab.\nClick to toggle on/off; use the dropdown for size presets and a custom size slider."),
        FNewToolMenuDelegate::CreateRaw(this, &FBigSplinePointsModule::PopulateDropdown));

    // Primary button click toggles the boost on/off.
    FToolUIAction ToggleAction;
    ToggleAction.ExecuteAction = FToolMenuExecuteAction::CreateLambda(
        [](const FToolMenuContext&)
        {
            FBigSplinePointsModule::Get().ToggleActive();
        });
    ToggleAction.GetActionCheckState = FToolMenuGetActionCheckState::CreateLambda(
        [](const FToolMenuContext&)
        {
            return FBigSplinePointsModule::Get().IsActive() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
        });

    Entry.ToolBarData.ActionOverride = ToggleAction;
    Entry.ToolBarData.LabelOverride = FText();
    // Keep the button pinned in the left cluster (next to the transform/snapping tools) rather than
    // collapsing into the overflow menu when the toolbar is tight.
    Entry.ToolBarData.ResizeParams.ClippingPriority = 1000;
    Entry.SetShowInToolbarTopLevel(true);
    Entry.Icon = GetToolbarIcon();

    Section.AddEntry(Entry);
}

void FBigSplinePointsModule::PopulateDropdown(UToolMenu* Menu)
{
    if (!Menu)
    {
        return;
    }

    // Single size slider (1..100).
    {
        FToolMenuSection& SliderSection = Menu->FindOrAddSection("Size", LOCTEXT("SizeHeader", "Spline Point Size"));

        // clang-format off
        TSharedRef<SWidget> SliderWidget =
            SNew(SBox)
            .Padding(FMargin(16.0f, 2.0f, 8.0f, 2.0f))
            .MinDesiredWidth(220.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .AutoWidth()
                .Padding(0.0f, 0.0f, 6.0f, 0.0f)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("SizeLabel", "Size"))
                    .TextStyle(FAppStyle::Get(), "Menu.Label")
                ]
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(SNumericEntryBox<float>)
                    .AllowSpin(true)
                    .MinValue(FBigSplinePointsModule::SliderMinSize)
                    .MaxValue(FBigSplinePointsModule::SliderMaxSize)
                    .MinSliderValue(FBigSplinePointsModule::SliderMinSize)
                    .MaxSliderValue(FBigSplinePointsModule::SliderMaxSize)
                    .MaxFractionalDigits(0)
                    .Value_Lambda([]() -> TOptional<float> { return FBigSplinePointsModule::Get().GetSize(); })
                    .OnValueChanged_Lambda([](float NewValue) { FBigSplinePointsModule::Get().SetSize(NewValue); })
                ]
            ];
        // clang-format on

        SliderSection.AddEntry(FToolMenuEntry::InitWidget("BigSplineSizeSlider", SliderWidget, FText::GetEmpty()));
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBigSplinePointsModule, BigSplinePoints)
