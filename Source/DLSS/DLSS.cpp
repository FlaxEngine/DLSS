#include "DLSS.h"
#include "DLSSPostFx.h"
#include "DLSSSettings.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Config/GameSettings.h"
#include "Engine/Content/Content.h"
#include "Engine/Content/JsonAsset.h"
#include "Engine/Graphics/RenderTask.h"
#include "Engine/Profiler/ProfilerCPU.h"

IMPLEMENT_GAME_SETTINGS_GETTER(DLSSSettings, "DLSS");

DLSS::DLSS(const SpawnParams& params)
    : GamePlugin(params)
{
    _description.Name = TEXT("DLSS");
#if USE_EDITOR
    _description.Category = TEXT("Rendering");
    _description.Description = TEXT("DLSS is a revolutionary breakthrough in AI-powered graphics upscaling technology that massively boosts performance.");
    _description.Author = TEXT("NVIDIA");
    _description.RepositoryUrl = TEXT("https://github.com/FlaxEngine/DLSS");
#endif
    _description.Version = Version(3, 1, 0);
}

DLSSSupport DLSS::GetSupport() const
{
    if (_delayInit)
        const_cast<DLSS*>(this)->DelayInit();
    return _support;
}

void DLSS::ApplyRecommendedSettings(DLSSQuality quality)
{
    auto task = MainRenderTask::Instance;
    if (!task)
        return;
    DLSSRecommendedSettings settings;
    const Float2 outputSize = task->GetOutputViewport().Size;
    QueryRecommendedSettings(outputSize, settings, quality);
    task->RenderingPercentage = Math::Min((float)settings.ResolutionOptimal.X / outputSize.X, (float)settings.ResolutionOptimal.Y / outputSize.Y);
    Sharpness = settings.Sharpness;
}

void DLSS::QueryRecommendedSettings(const Int2& displaySize, DLSSRecommendedSettings& result, DLSSQuality quality)
{
    if (quality == DLSSQuality::MAX)
        quality = Quality;
    _ngx.QueryRecommendedSettings(displaySize, result, quality);
}

void DLSS::DelayInit()
{
    PROFILE_CPU();
    _delayInit = false;
    const auto settings = DLSSSettings::Get();
    LOG(Info, "Initializing DLSS with AppId={}, ProjectId={}", settings->AppId, String(settings->ProjectId));
    if (_ngx.Initialize(settings->AppId, settings->ProjectId, _support))
    {
        LOG(Warning, "DLSS is not supported on this platform.");
        return;
    }
}

void DLSS::Initialize()
{
    GamePlugin::Initialize();

    // TODO: apply global mip bias to texture groups samplers to increase texturing quality
    PostFx = New<DLSSPostFx>();
    SceneRenderTask::AddGlobalCustomPostFx(PostFx);

    const auto settings = DLSSSettings::Get();
    _support = DLSSSupport::NotSupported;
    _delayInit = settings->LazyInit;
    if (_delayInit)
        return;
    DelayInit();
}

void DLSS::Deinitialize()
{
    if (PostFx)
    {
        SceneRenderTask::RemoveGlobalCustomPostFx(PostFx);
        PostFx->DeleteObject();
        PostFx = nullptr;
    }
    _ngx.Shutdown();

    GamePlugin::Deinitialize();
}
