// Copyright 2026 Octree, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Textures/SlateIcon.h"

class UToolMenu;

/**
 * Editor module for the Big Spline Points plugin.
 *
 * Adds a toggle button (with a presets/slider dropdown) to the level viewport toolbar that enlarges
 * spline control points, tangent handles, and landscape spline control-point icons. All sizing is
 * driven by runtime engine settings (no engine modification, no custom visualizer): the boost is a
 * pure runtime overlay applied to the live CDO values and restored on shutdown, so the user's saved
 * preferences are never overwritten.
 */
class FBigSplinePointsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FBigSplinePointsModule& Get();

    /** Size slider range (extra screen-space pixels added to the handles). */
    static constexpr float SliderMinSize = 1.0f;
    static constexpr float SliderMaxSize = 100.0f;

    /** Style set name for the custom toolbar icon. */
    static FName GetStyleSetName();
    static FSlateIcon GetToolbarIcon();

    /** Flip the boost on/off (primary toolbar button click). */
    void ToggleActive();
    bool IsActive() const;

    /** Set the boost size and activate; used by the slider. */
    void SetSize(float InSize);
    float GetSize() const;

private:
    void RegisterMenus();
    void PopulateDropdown(UToolMenu* Menu);
    static void RegisterStyle();
    static void UnregisterStyle();

    /** Capture the user's original spline-size preferences (first run only). */
    void CaptureBaselines();

    /** Apply the overlay when active, or restore baselines when inactive, then redraw viewports. */
    void ApplyState();

    /** Restore the captured baselines to the live engine settings (used on shutdown). */
    void RestoreBaselines();

    /** Set ULandscapeSettings::SplineIconScale via reflection (the member is protected). */
    void SetLandscapeIconScale(float Value);

    /** Read the current ULandscapeSettings::SplineIconScale via its public getter. */
    float GetLandscapeIconScale() const;
};
