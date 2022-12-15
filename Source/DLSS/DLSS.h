#pragma once

#include "Engine/Scripting/Plugins/GamePlugin.h"
#include "Types.h"
#include "NGXWrapper.h"

class DLSSPostFx;

/// <summary>
/// DLSS plugin.
/// </summary>
API_CLASS(Namespace="NVIDIA") class DLSS_API DLSS : public GamePlugin
{
    friend DLSSPostFx;
    DECLARE_SCRIPTING_TYPE(DLSS);

private:
    NGXWrapper _ngx;

public:
    /// <summary>
    /// DLSS post process effect.
    /// </summary>
    API_FIELD(ReadOnly) DLSSPostFx* PostFx = nullptr;

    /// <summary>
    /// DLSS support information.
    /// </summary>
    API_FIELD() DLSSSupport Support = DLSSSupport::NotSupported;

    /// <summary>
    /// DLSS upscaling quality.
    /// </summary>
    API_FIELD() DLSSQuality Quality = DLSSQuality::Balanced;

    /// <summary>
    /// Softening or sharpening factor to apply during the DLSS pass. Negative values soften the image, positive values sharpen. In range [-1; 1].
    /// </summary>
    API_FIELD() float Sharpness = 0.0f;

    /// <summary>
    /// Calculates the optimal settings for the rendering into the certain display resolution at given quality.
    /// </summary>
    /// <param name="quality">DLSS quality, MAX to use current setting.</param>
    API_FUNCTION() void ApplyRecommendedSettings(DLSSQuality quality = DLSSQuality::MAX);

    /// <summary>
    /// Calculates the optimal settings for the rendering into the certain display resolution at given quality.
    /// </summary>
    /// <param name="displaySize">Display (output) resolution (in pixels).</param>
    /// <param name="result">Output settings.</param>
    /// <param name="quality">DLSS quality, MAX to use current setting.</param>
    API_FUNCTION() void QueryRecommendedSettings(API_PARAM(ref) const Int2& displaySize, API_PARAM(Out) DLSSRecommendedSettings& result, DLSSQuality quality = DLSSQuality::MAX);

public:
    // [GamePlugin]
    void Initialize() override;
    void Deinitialize() override;
};
