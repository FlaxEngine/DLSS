#include "NGXWrapper.h"
#include "Engine/Core/Log.h"
#include "Engine/Engine/Time.h"
#include "Engine/Engine/Globals.h"
#include "Engine/Platform/FileSystem.h"
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Graphics/GPUContext.h"
#include "Engine/Graphics/RenderBuffers.h"
#include "Engine/Graphics/Textures/GPUTexture.h"
#include "FlaxEngine.Gen.h"
#include <nvsdk_ngx.h>
#include <nvsdk_ngx_helpers.h>
#if GRAPHICS_API_VULKAN
#include "ThirdParty/volk/volk.h"
#include <nvsdk_ngx_vk.h>
#include <nvsdk_ngx_helpers_vk.h>
#endif

#if GRAPHICS_API_VULKAN

void GetVulkanResource(NVSDK_NGX_Resource_VK& resource, GPUTexture* texture, NVSDK_NGX_Resource_VK*& ptr)
{
    if (!texture)
    {
        ptr = nullptr;
        return;
    }

    // TODO: find a way to get VkImageInfo and other data from GPUTextureViewVulkan (maybe refactor GetNativePtr for vulkan view to have more data there?)
    CRASH;
    auto& resourceImageView = resource.Resource.ImageViewInfo;
    /*resourceImageView.ImageView = 0;
    resourceImageView.Image = (VkImage)texture->GetNativePtr();
    resourceImageView.SubresourceRange;
    resourceImageView.Format;*/
    resourceImageView.Width = texture->Width();
    resourceImageView.Height = texture->Height();
    resource.Type = NVSDK_NGX_RESOURCE_VK_TYPE_VK_IMAGEVIEW;
    resource.ReadWrite = texture->IsUnorderedAccess();
    ptr = &resource;
}

#endif

NVSDK_NGX_PerfQuality_Value GetQuality(DLSSQuality quality)
{
    switch (quality)
    {
    case DLSSQuality::UltraPerformance:
        return NVSDK_NGX_PerfQuality_Value_UltraPerformance;
    case DLSSQuality::Performance:
        return NVSDK_NGX_PerfQuality_Value_MaxPerf;
    case DLSSQuality::Balanced:
        return NVSDK_NGX_PerfQuality_Value_Balanced;
    case DLSSQuality::Quality:
        return NVSDK_NGX_PerfQuality_Value_MaxQuality;
    case DLSSQuality::UltraQuality:
    default:
        return NVSDK_NGX_PerfQuality_Value_UltraQuality;
    }
}

bool NGXWrapper::Initialize(uint32 appId, const StringAnsi& projectId, DLSSSupport& support)
{
    // Check DLSS support
    auto gpuDevice = GPUDevice::Instance;
    if (!gpuDevice->Limits.HasCompute || !gpuDevice->GetAdapter()->IsNVIDIA())
    {
        support = DLSSSupport::NotSupportedIncompatibleHardware;
        return true;
    }
#if PLATFORM_WINDOWS
    if (!Platform::IsWindows10())
    {
        support = DLSSSupport::NotSupportedOperatingSystemOutOfDate;
        return true;
    }
#endif

    // Initialize NGX
    void* gpuDeviceNative = gpuDevice->GetNativePtr();
    const char* engineVersion = FLAXENGINE_VERSION_TEXT;
    const String& appDataPath = Globals::TemporaryFolder;
    if (appId == 0)
        appId = 231313132; // Fallback to value from Sample App
    NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;
    _rendererType = gpuDevice->GetRendererType();
    switch (_rendererType)
    {
    case RendererType::DirectX11:
        if (projectId.HasChars())
            result = NVSDK_NGX_D3D11_Init_with_ProjectID(projectId.Get(), NVSDK_NGX_ENGINE_TYPE_CUSTOM, engineVersion, *appDataPath, (ID3D11Device*)gpuDeviceNative);
        else
            result = NVSDK_NGX_D3D11_Init(appId, *appDataPath, (ID3D11Device*)gpuDeviceNative);
        break;
    case RendererType::DirectX12:
        if (projectId.HasChars())
            result = NVSDK_NGX_D3D12_Init_with_ProjectID(projectId.Get(), NVSDK_NGX_ENGINE_TYPE_CUSTOM, engineVersion, *appDataPath, (ID3D12Device*)gpuDeviceNative);
        else
            result = NVSDK_NGX_D3D12_Init(appId, *appDataPath, (ID3D12Device*)gpuDeviceNative);
        break;
#if GRAPHICS_API_VULKAN
    case RendererType::Vulkan:
        {
            VkInstance vkInstance = (VkInstance)((void**)gpuDeviceNative)[0];
            VkDevice vkDevice = (VkDevice)((void**)gpuDeviceNative)[1];
            VkPhysicalDevice vkPhysicalDevice = (VkPhysicalDevice)gpuDevice->GetAdapter()->GetNativePtr();
            if (projectId.HasChars())
                result = NVSDK_NGX_VULKAN_Init_with_ProjectID(projectId.Get(), NVSDK_NGX_ENGINE_TYPE_CUSTOM, engineVersion, *appDataPath, vkInstance, vkPhysicalDevice, vkDevice);
            else
                result = NVSDK_NGX_VULKAN_Init(appId, *appDataPath, vkInstance, vkPhysicalDevice, vkDevice);
            break;
        }
#endif
    default:
        return true;
    }
    if (NVSDK_NGX_FAILED(result))
    {
        if (result == NVSDK_NGX_Result_FAIL_OutOfDate)
            support = DLSSSupport::NotSupportedDriverOutOfDate;
        else if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported)
            support = DLSSSupport::NotSupportedIncompatibleHardware;
        if (result == NVSDK_NGX_Result_FAIL_FeatureNotSupported || result == NVSDK_NGX_Result_FAIL_PlatformError)
            LOG(Warning, "NVIDIA NGX not available on this hardware/platform. Error code: 0x{:x}, {}", result, GetNGXResultAsString(result));
        else
            LOG(Error, "Failed to initialize NGX. Error code: 0x{:x}, {}", result, GetNGXResultAsString(result));
        return true;
    }

    // Get capability parameters
    switch (_rendererType)
    {
    case RendererType::DirectX11:
        result = NVSDK_NGX_D3D11_GetCapabilityParameters(&_capabilityParameters);
        break;
    case RendererType::DirectX12:
        result = NVSDK_NGX_D3D12_GetCapabilityParameters(&_capabilityParameters);
        break;
#if GRAPHICS_API_VULKAN
    case RendererType::Vulkan:
        result = NVSDK_NGX_VULKAN_GetCapabilityParameters(&_capabilityParameters);
        break;
#endif
    }
    if (NVSDK_NGX_FAILED(result) || !_capabilityParameters)
    {
        LOG(Error, "Failed to get NGX capability parameters. Error code: 0x{:x}, {}", result, GetNGXResultAsString(result));
        return true;
    }
    {
        int available = 0;
        result = _capabilityParameters->Get(NVSDK_NGX_EParameter_SuperSampling_Available, &available);
        if (!available)
        {
            LOG(Warning, "DLSS is not available.");
            return true;
        }
    }

    support = DLSSSupport::Supported;
    _initialized = true;
    return false;
}

void NGXWrapper::Shutdown()
{
    if (!_initialized)
        return;
    _initialized = false;

    _capabilityParameters = nullptr;
    NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;
    auto gpuDevice = GPUDevice::Instance;
    void* gpuDeviceNative = gpuDevice->GetNativePtr();
    switch (_rendererType)
    {
    case RendererType::DirectX11:
        if (_parametersObject)
            NVSDK_NGX_D3D11_DestroyParameters(_parametersObject);
        result = NVSDK_NGX_D3D11_Shutdown1((ID3D11Device*)gpuDeviceNative);
        break;
    case RendererType::DirectX12:
        if (_parametersObject)
            NVSDK_NGX_D3D12_DestroyParameters(_parametersObject);
        result = NVSDK_NGX_D3D12_Shutdown1((ID3D12Device*)gpuDeviceNative);
        break;
#if GRAPHICS_API_VULKAN
    case RendererType::Vulkan:
        if (_parametersObject)
            NVSDK_NGX_VULKAN_DestroyParameters(_parametersObject);
        result = NVSDK_NGX_VULKAN_Shutdown1((VkDevice)((void**)gpuDeviceNative)[1]);
        break;
#endif
    }
    _parametersObject = nullptr;
    _paramsHandle = nullptr;
    _params = NGXParams();
    if (NVSDK_NGX_FAILED(result))
    {
        LOG(Error, "Failed to shutdown NGX. Error code: 0x{:x}, {}", result, GetNGXResultAsString(result));
    }
}

void NGXWrapper::QueryRecommendedSettings(const Int2& displaySize, DLSSRecommendedSettings& output, DLSSQuality quality) const
{
    if (_initialized)
    {
        NVSDK_NGX_PerfQuality_Value dlssQuality = GetQuality(quality);
        uint32 renderOptimalX, renderOptimalY, renderMinX, renderMinY, renderMaxX, renderMaxY;
        const NVSDK_NGX_Result result = NGX_DLSS_GET_OPTIMAL_SETTINGS(_capabilityParameters, displaySize.X, displaySize.Y, dlssQuality, &renderOptimalX, &renderOptimalY, &renderMaxX, &renderMaxY, &renderMinX, &renderMinY, &output.Sharpness);
        if (NVSDK_NGX_SUCCEED(result))
        {
            output.ResolutionOptimal.X = (int32)renderOptimalX;
            output.ResolutionOptimal.Y = (int32)renderOptimalY;
            output.ResolutionMin.X = (int32)renderMinX;
            output.ResolutionMin.Y = (int32)renderMinY;
            output.ResolutionMax.X = (int32)renderMaxX;
            output.ResolutionMax.Y = (int32)renderMaxY;
            return;
        }
    }
    output.ResolutionOptimal = displaySize;
    output.ResolutionMin = displaySize;
    output.ResolutionMax = displaySize;
    output.Sharpness = 0.0f;
}

void NGXWrapper::TemporalResolve(GPUContext* context, RenderContext& renderContext, GPUTexture* input, GPUTexture* output, DLSSQuality quality, const Float2& pixelOffset, float sharpness)
{
    ASSERT(_initialized);
    NVSDK_NGX_Result result = NVSDK_NGX_Result_Fail;
    void* contextNative = context->GetNativePtr();

    // Build params for current pass
    NGXParams params;
    params.SrcSize = input->Size();
    params.DstSize = output->Size();
    params.Quality = quality;
    params.UseSharpness = !Math::IsZero(sharpness);
    if (params != _params)
    {
        _params = params;
        NVSDK_NGX_DLSS_Create_Params createParams;
        Platform::MemoryClear(&createParams, sizeof(createParams));
        createParams.Feature.InWidth = params.SrcSize.X;
        createParams.Feature.InHeight = params.SrcSize.Y;
        createParams.Feature.InTargetWidth = params.DstSize.X;
        createParams.Feature.InTargetHeight = params.DstSize.Y;
        createParams.Feature.InPerfQualityValue = GetQuality(quality);
        createParams.InFeatureCreateFlags |= NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
        createParams.InFeatureCreateFlags |= params.UseSharpness ? NVSDK_NGX_DLSS_Feature_Flags_DoSharpening : 0;
        createParams.InFeatureCreateFlags |= NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;
        switch (_rendererType)
        {
        case RendererType::DirectX11:
            if (!_parametersObject)
                NVSDK_NGX_D3D11_AllocateParameters(&_parametersObject);
            result = NGX_D3D11_CREATE_DLSS_EXT((ID3D11DeviceContext*)contextNative, &_paramsHandle, _parametersObject, &createParams);
            break;
        case RendererType::DirectX12:
            if (!_parametersObject)
                NVSDK_NGX_D3D12_AllocateParameters(&_parametersObject);
            result = NGX_D3D12_CREATE_DLSS_EXT((ID3D12GraphicsCommandList*)contextNative, 1, 1, &_paramsHandle, _parametersObject, &createParams);
            break;
#if GRAPHICS_API_VULKAN
        case RendererType::Vulkan:
            if (!_parametersObject)
                NVSDK_NGX_VULKAN_AllocateParameters(&_parametersObject);
            result = NGX_VULKAN_CREATE_DLSS_EXT((VkCommandBuffer)contextNative, 1, 1, &_paramsHandle, _parametersObject, &createParams);
            break;
#endif
        }
        if (NVSDK_NGX_FAILED(result))
        {
            LOG(Error, "Failed to create params. Error code: 0x{:x}, {}", result, GetNGXResultAsString(result));
            return;
        }
    }

    // Put resources into proper state
    switch (_rendererType)
    {
    case RendererType::DirectX12:
        context->SetResourceState(output, 0x8); // D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        context->SetResourceState(input, 0x40 | 0x80); // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        context->SetResourceState(renderContext.Task->Buffers->DepthBuffer, 0x40 | 0x80); // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        if (renderContext.Task->Buffers->MotionVectors)
            context->SetResourceState(renderContext.Task->Buffers->MotionVectors, 0x40 | 0x80); // D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        break;
    }

    // Sync cached state with backend
    context->FlushState();

    // Evaluate DLSS
    switch (_rendererType)
    {
    case RendererType::DirectX11:
        {
            NVSDK_NGX_D3D11_DLSS_Eval_Params evalParams;
            Platform::MemoryClear(&evalParams, sizeof(evalParams));
            evalParams.Feature.pInOutput = (ID3D11Resource*)output->GetNativePtr();
            evalParams.Feature.pInColor = (ID3D11Resource*)input->GetNativePtr();
            evalParams.pInDepth = (ID3D11Resource*)renderContext.Task->Buffers->DepthBuffer->GetNativePtr();
            evalParams.pInMotionVectors = renderContext.Task->Buffers->MotionVectors ? (ID3D11Resource*)renderContext.Task->Buffers->MotionVectors->GetNativePtr() : nullptr;
            evalParams.InRenderSubrectDimensions.Width = params.SrcSize.X;
            evalParams.InRenderSubrectDimensions.Height = params.SrcSize.Y;
            evalParams.pInExposureTexture = nullptr;
            evalParams.InPreExposure = 0.0f;
            evalParams.Feature.InSharpness = sharpness;
            evalParams.InJitterOffsetX = pixelOffset.X;
            evalParams.InJitterOffsetY = pixelOffset.Y;
            evalParams.InMVScaleX = (float)params.SrcSize.X; // scale motion vectors from normalized [-1;1] to pixel-space
            evalParams.InMVScaleY = (float)params.SrcSize.Y;
            //evalParams.InMVScaleX = 1.0f;
            //evalParams.InMVScaleY = 1.0f;
            evalParams.InReset = renderContext.Task->IsCameraCut;
            evalParams.InFrameTimeDeltaInMsec = (float)Time::Draw.UnscaledDeltaTime.GetTotalMilliseconds();
            result = NGX_D3D11_EVALUATE_DLSS_EXT((ID3D11DeviceContext*)contextNative, _paramsHandle, _parametersObject, &evalParams);
            break;
        }
    case RendererType::DirectX12:
        {
            NVSDK_NGX_D3D12_DLSS_Eval_Params evalParams;
            Platform::MemoryClear(&evalParams, sizeof(evalParams));
            evalParams.Feature.pInOutput = (ID3D12Resource*)output->GetNativePtr();
            evalParams.Feature.pInColor = (ID3D12Resource*)input->GetNativePtr();
            evalParams.pInDepth = (ID3D12Resource*)renderContext.Task->Buffers->DepthBuffer->GetNativePtr();
            evalParams.pInMotionVectors = renderContext.Task->Buffers->MotionVectors ? (ID3D12Resource*)renderContext.Task->Buffers->MotionVectors->GetNativePtr() : nullptr;
            evalParams.InRenderSubrectDimensions.Width = params.SrcSize.X;
            evalParams.InRenderSubrectDimensions.Height = params.SrcSize.Y;
            evalParams.pInExposureTexture = nullptr;
            evalParams.InPreExposure = 0.0f;
            evalParams.Feature.InSharpness = sharpness;
            evalParams.InJitterOffsetX = pixelOffset.X;
            evalParams.InJitterOffsetY = pixelOffset.Y;
            evalParams.InMVScaleX = (float)params.SrcSize.X; // scale motion vectors from normalized [-1;1] to pixel-space
            evalParams.InMVScaleY = (float)params.SrcSize.Y;
            //evalParams.InMVScaleX = 1.0f;
            //evalParams.InMVScaleY = 1.0f;
            evalParams.InReset = renderContext.Task->IsCameraCut;
            evalParams.InFrameTimeDeltaInMsec = (float)Time::Draw.UnscaledDeltaTime.GetTotalMilliseconds();
            result = NGX_D3D12_EVALUATE_DLSS_EXT((ID3D12GraphicsCommandList*)contextNative, _paramsHandle, _parametersObject, &evalParams);

            // Ensure that root signature and descriptor heaps are properly set after DLSS modified them
            context->ForceRebindDescriptors();
            break;
        }
#if GRAPHICS_API_VULKAN
    case RendererType::Vulkan:
        {
            NVSDK_NGX_VK_DLSS_Eval_Params evalParams;
            Platform::MemoryClear(&evalParams, sizeof(evalParams));
            NVSDK_NGX_Resource_VK outputVk, inputVk, depthBufferVk, motionVectorsVk;
            GetVulkanResource(outputVk, output, evalParams.Feature.pInOutput);
            GetVulkanResource(inputVk, input, evalParams.Feature.pInColor);
            GetVulkanResource(depthBufferVk, renderContext.Task->Buffers->DepthBuffer, evalParams.pInDepth);
            GetVulkanResource(motionVectorsVk, renderContext.Task->Buffers->MotionVectors, evalParams.pInMotionVectors);
            evalParams.InRenderSubrectDimensions.Width = params.SrcSize.X;
            evalParams.InRenderSubrectDimensions.Height = params.SrcSize.Y;
            evalParams.pInExposureTexture = nullptr;
            evalParams.InPreExposure = 0.0f;
            evalParams.Feature.InSharpness = sharpness;
            evalParams.InJitterOffsetX = pixelOffset.X;
            evalParams.InJitterOffsetY = pixelOffset.Y;
            evalParams.InMVScaleX = (float)params.SrcSize.X; // scale motion vectors from normalized [-1;1] to pixel-space
            evalParams.InMVScaleY = (float)params.SrcSize.Y;
            //evalParams.InMVScaleX = 1.0f;
            //evalParams.InMVScaleY = 1.0f;
            evalParams.InReset = renderContext.Task->IsCameraCut;
            evalParams.InFrameTimeDeltaInMsec = (float)Time::Draw.UnscaledDeltaTime.GetTotalMilliseconds();
            result = NGX_VULKAN_EVALUATE_DLSS_EXT((VkCommandBuffer)contextNative, _paramsHandle, _parametersObject, &evalParams);
            break;
        }
#endif
    }
    if (NVSDK_NGX_FAILED(result))
    {
        LOG(Error, "Failed to evaluate DLSS. Error code: 0x{:x}, {}", result, GetNGXResultAsString(result));
        return;
    }
    context->ClearState();
}
