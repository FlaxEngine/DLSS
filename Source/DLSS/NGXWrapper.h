#pragma once

#include "Types.h"
#include "Engine/Core/Types/BaseTypes.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Graphics/RenderTask.h"

struct NVSDK_NGX_Parameter;
struct NVSDK_NGX_Handle;
class GPUTexture;
class GPUContext;

struct NGXParams
{
    Int2 SrcSize = Int2::Zero;
    Int2 DstSize = Int2::Zero;
    DLSSQuality Quality = DLSSQuality::Balanced;
    bool UseSharpness = false;

    friend bool operator==(const NGXParams& lhs, const NGXParams& rhs)
    {
        return lhs.SrcSize == rhs.SrcSize
            && lhs.DstSize == rhs.DstSize
            && lhs.Quality == rhs.Quality
            && lhs.UseSharpness == rhs.UseSharpness;
    }

    friend bool operator!=(const NGXParams& lhs, const NGXParams& rhs)
    {
        return !(lhs == rhs);
    }
};

class NGXWrapper
{
private:
    bool _initialized = false;
    RendererType _rendererType;
    NVSDK_NGX_Parameter* _capabilityParameters = nullptr;
    NGXParams _params;
    NVSDK_NGX_Handle* _paramsHandle = nullptr;
    NVSDK_NGX_Parameter* _parametersObject = nullptr;

public:
    bool Initialize(uint32 appId, const StringAnsi& projectId, DLSSSupport& support);
    void Shutdown();
    void QueryRecommendedSettings(const Int2& displaySize, DLSSRecommendedSettings& output, DLSSQuality quality) const;
    void TemporalResolve(GPUContext* context, RenderContext& renderContext, GPUTexture* input, GPUTexture* output, DLSSQuality quality, const Float2& pixelOffset, float sharpness);
};
