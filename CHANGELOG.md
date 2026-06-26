# Changelog

All notable changes to **Big Spline Points** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-06-26

### Added
- Initial release.
- Level-viewport toolbar toggle (icon-only button with a dropdown) that enlarges spline editing handles.
- **Size slider (1–100)** controlling the extra screen-space size added to control points and tangent handles.
- Support for `USplineComponent` control points and tangent handles, and landscape spline control-point icons (Landscape Mode).
- Non-destructive: the user's original sizes are captured on startup and restored on shutdown; state is stored per-user and never written to shared project config.
- Forces the spline component visualizer to refresh so size changes apply immediately rather than on the next spline edit.
