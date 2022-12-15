#include "DLSSPostFx.h"
#include "DLSS.h"
#include "Engine/Profiler/Profiler.h"
#include "Engine/Scripting/Plugins/PluginManager.h"
#include "Engine/Graphics/GPUContext.h"
#include "Engine/Graphics/RenderTargetPool.h"
#include "Engine/Graphics/Textures/GPUTexture.h"
#include "Engine/Renderer/RenderList.h"

DLSSPostFx::DLSSPostFx(const SpawnParams& params)
    : PostProcessEffect(params)
{
    Location = PostProcessEffectLocation::CustomUpscale;
}

bool DLSSPostFx::CanRender() const
{
    auto dlss = PluginManager::GetPlugin<DLSS>();
    return PostProcessEffect::CanRender() && dlss && dlss->Support == DLSSSupport::Supported;
}

void DLSSPostFx::PreRender(GPUContext* context, RenderContext& renderContext)
{
    if (renderContext.Task->RenderingPercentage >= 1.0f)
        return;

    // Override upscaling location to happen before PostFx
    renderContext.List->Setup.UpscaleLocation = RenderingUpscaleLocation::BeforePostProcessingPass;

    // Enable temporal jitter
    renderContext.List->Setup.UseTemporalAAJitter = true;

    // Disable anti-aliasing
    renderContext.List->Settings.AntiAliasing.Mode = AntialiasingMode::None;
}

void DLSSPostFx::Render(GPUContext* context, RenderContext& renderContext, GPUTexture* input, GPUTexture* output)
{
    PROFILE_GPU_CPU("DLSS");

    // DLSS requries output texture to have UAV
    // TODO: when running in RenderingUpscaleLocation::BeforePostProcessingPass add UAV to tempBuffer in Renderer (maybe add api to PostProcessEffect to declare output info?)
    GPUTexture* dlssOutput = output;
    if (!dlssOutput->IsUnorderedAccess())
    {
        GPUTextureDescription desc = output->GetDescription();
        desc.Flags &= ~GPUTextureFlags::BackBuffer;
        desc.Flags |= GPUTextureFlags::UnorderedAccess;
        dlssOutput = RenderTargetPool::Get(desc);
    }

    // Run DLSS
    auto dlss = PluginManager::GetPlugin<DLSS>();
    const float sharpness = Math::Clamp(dlss->Sharpness, -1.0f, 1.0f);
    const Float2 pixelOffset(renderContext.View.TemporalAAJitter.X * renderContext.View.ScreenSize.X / 2.0f, renderContext.View.TemporalAAJitter.X * renderContext.View.ScreenSize.Y / 2.0f);
    dlss->_ngx.TemporalResolve(context, renderContext, input, dlssOutput, dlss->Quality, pixelOffset, sharpness);

    // Copy back results
    if (dlssOutput != output)
    {
        PROFILE_GPU("Copy");
        context->CopyResource(output, dlssOutput);
        RenderTargetPool::Release(dlssOutput);
    }
}
