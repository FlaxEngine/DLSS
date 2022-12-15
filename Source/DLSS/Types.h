#pragma once

#include "Engine/Core/Types/BaseTypes.h"
#include "Engine/Core/Math/Vector2.h"

/// <summary>
/// DLSS support modes.
/// </summary>
API_ENUM(Namespace="NVIDIA") enum class DLSSSupport
{
    // DLSS is supported.
    Supported,
    // DLSS is not supported.
    NotSupported,
    // DLSS is not supported due to incompatible hardware (eg. non-NVIDIA GPU).
    NotSupportedIncompatibleHardware,
    // DLSS is not supported due to incompatible driver (too old version).
    NotSupportedDriverOutOfDate,
    // DLSS is not supported due to incompatible operating system (too old version).
    NotSupportedOperatingSystemOutOfDate,

    MAX
};

/// <summary>
/// DLSS quality modes.
/// </summary>
API_ENUM(Namespace="NVIDIA") enum class DLSSQuality
{
    // Ultra performance.
    UltraPerformance,
    // Max performance.
    Performance,
    // Balanced quality.
    Balanced,
    // Max quality.
    Quality,
    // Ultra quality.
    UltraQuality,

    MAX
};

/// <summary>
/// DLSS optimal settings descriptor.
/// </summary>
API_STRUCT(Namespace="NVIDIA") struct DLSS_API DLSSRecommendedSettings
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(DLSSRecommendedSettings);

    // Optimal render resolution value.
    API_FIELD() Int2 ResolutionOptimal;
    // Optimal render minimum resolution value (if using dynamic resolution).
    API_FIELD() Int2 ResolutionMin;
    // Optimal render maximum resolution value (if using dynamic resolution).
    API_FIELD() Int2 ResolutionMax;
    // Optimal sharpness parameter value.
    API_FIELD() float Sharpness;
};
