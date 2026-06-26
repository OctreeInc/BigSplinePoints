// Copyright 2026 Octree, Inc. All Rights Reserved.

#include "BigSplinePointsSettings.h"

UBigSplinePointsSettings::UBigSplinePointsSettings()
{
    CategoryName = TEXT("Plugins");
}

FName UBigSplinePointsSettings::GetCategoryName() const
{
    return CategoryName;
}

#if WITH_EDITOR
FText UBigSplinePointsSettings::GetSectionText() const
{
    return NSLOCTEXT("BigSplinePoints", "SettingsSection", "Big Spline Points");
}

FText UBigSplinePointsSettings::GetSectionDescription() const
{
    return NSLOCTEXT("BigSplinePoints", "SettingsDescription",
        "Enlarge spline control points and tangent handles in the viewport so they are easier to grab.");
}
#endif

UBigSplinePointsSettings* UBigSplinePointsSettings::Get()
{
    return GetMutableDefault<UBigSplinePointsSettings>();
}
