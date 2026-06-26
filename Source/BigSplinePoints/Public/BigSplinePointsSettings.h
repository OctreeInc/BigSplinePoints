// Copyright 2026 Octree, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BigSplinePointsSettings.generated.h"

/**
 * Per-user settings for the Big Spline Points plugin.
 *
 * Stored in EditorPerProjectUserSettings (per-user, never the shared project ini). This object is
 * the single source of truth for the plugin's state. The captured baselines record the user's
 * original spline-size preferences so toggling the feature Off restores exactly what they had,
 * even if the engine settings get contaminated by a previous boosted session.
 */
UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "Big Spline Points"))
class BIGSPLINEPOINTS_API UBigSplinePointsSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UBigSplinePointsSettings();

    /** Whether the size boost is currently applied. */
    UPROPERTY(config, EditAnywhere, Category = "Big Spline Points")
    bool bActive = false;

    /** Extra screen-space size added to spline control points / tangent handles (1..100). */
    UPROPERTY(config, EditAnywhere, Category = "Big Spline Points", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float Size = 20.0f;

    /** True once the original baselines have been captured from the engine settings. */
    UPROPERTY(config)
    bool bHasBaselines = false;

    /** Captured user baseline for ULevelEditorViewportSettings::SelectedSplinePointSizeAdjustment. */
    UPROPERTY(config)
    float BaselinePointSize = 0.0f;

    /** Captured user baseline for ULevelEditorViewportSettings::SplineTangentHandleSizeAdjustment. */
    UPROPERTY(config)
    float BaselineTangentSize = 0.0f;

    /** Captured user baseline for ULevelEditorViewportSettings::SplineLineThicknessAdjustment. */
    UPROPERTY(config)
    float BaselineLineThickness = 0.0f;

    /** Captured user baseline for ULandscapeSettings::SplineIconScale. */
    UPROPERTY(config)
    float BaselineLandscapeIconScale = 125.0f;

    // UDeveloperSettings interface
    virtual FName GetCategoryName() const override;
#if WITH_EDITOR
    virtual FText GetSectionText() const override;
    virtual FText GetSectionDescription() const override;
#endif

    /** Convenience accessor for the mutable CDO. */
    static UBigSplinePointsSettings* Get();
};
