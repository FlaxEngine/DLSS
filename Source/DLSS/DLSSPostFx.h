#pragma once

#include "Engine/Graphics/PostProcessEffect.h"

/// <summary>
/// DLSS effect renderer.
/// </summary>
API_CLASS(Namespace="NVIDIA") class DLSS_API DLSSPostFx : public PostProcessEffect
{
    DECLARE_SCRIPTING_TYPE(DLSSPostFx);
public:
    // [PostProcessEffect]
    bool CanRender(const RenderContext& renderContext) const override;
    void PreRender(GPUContext* context, RenderContext& renderContext) override;
    void Render(GPUContext* context, RenderContext& renderContext, GPUTexture* input, GPUTexture* output) override;
};
