/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2025, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream> // for std::cout, std::cerr
#include <format>   // for std::format

#include "../Resource.hpp"
#include "ImplSycl.hpp"

#include <sycl/sycl.hpp>

namespace syclexp = sycl::ext::oneapi::experimental;

namespace sgl {
extern bool openMessageBoxOnComputeApiError;
extern sycl::queue* g_syclQueue;
}

// clang-format off
// Some commonly used channel types
constexpr sycl::image_channel_type sycl_uint8   = sycl::image_channel_type::unsigned_int8;
constexpr sycl::image_channel_type sycl_uint16  = sycl::image_channel_type::unsigned_int16;
constexpr sycl::image_channel_type sycl_uint32  = sycl::image_channel_type::unsigned_int32;
constexpr sycl::image_channel_type sycl_sint8   = sycl::image_channel_type::signed_int8;
constexpr sycl::image_channel_type sycl_sint16  = sycl::image_channel_type::signed_int16;
constexpr sycl::image_channel_type sycl_sint32  = sycl::image_channel_type::signed_int32;
constexpr sycl::image_channel_type sycl_unorm8  = sycl::image_channel_type::unorm_int8;
constexpr sycl::image_channel_type sycl_unorm16 = sycl::image_channel_type::unorm_int16;
constexpr sycl::image_channel_type sycl_snorm8  = sycl::image_channel_type::snorm_int8;
constexpr sycl::image_channel_type sycl_snorm16 = sycl::image_channel_type::snorm_int16;
constexpr sycl::image_channel_type sycl_half    = sycl::image_channel_type::fp16;
constexpr sycl::image_channel_type sycl_float   = sycl::image_channel_type::fp32;

constexpr sycl::image_channel_order sycl_r    = sycl::image_channel_order::r;
constexpr sycl::image_channel_order sycl_rg   = sycl::image_channel_order::rg;
constexpr sycl::image_channel_order sycl_rgb  = sycl::image_channel_order::rgb;
constexpr sycl::image_channel_order sycl_rgba = sycl::image_channel_order::rgba;

template <typename T> 
inline constexpr uint32_t to_u32(T val) { return static_cast<uint32_t>(val); }
// clang-format on

namespace sgl { namespace d3d12 {

struct SyclExternalSemaphoreWrapper {
    syclexp::external_semaphore syclExternalSemaphore;
};
struct SyclExternalMemWrapper {
    sycl::ext::oneapi::experimental::external_mem syclExternalMem;
};
struct SyclImageMemHandleWrapper {
    syclexp::image_descriptor imgDesc;
    syclexp::image_mem_handle memHandle;
};

//----------------------------------------------------------------------------//
sycl::image_channel_type getSyclChannelType(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8B8A8_UINT:
            return sycl_uint8;
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16B16A16_UINT:
            return sycl_uint16;
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
            return sycl_uint32;
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return sycl_sint8;
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
            return sycl_sint16;
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return sycl_sint32;
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return sycl_unorm8;
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            return sycl_unorm16;
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            return sycl_snorm8;
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            return sycl_snorm16;
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return sycl_half;
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_D32_FLOAT:
            return sycl_float;
        default:
            sgl::Logfile::get()->throwError("Error in getSyclChannelType:"
                                            " Unsupported channel type.");
            return sycl_float;
    }
    return sycl_float;
}
//----------------------------------------------------------------------------//
static sycl::addressing_mode getSyclSampAddrMode(D3D12_TEXTURE_ADDRESS_MODE mode) {
    switch (mode) {
        case D3D12_TEXTURE_ADDRESS_MODE_WRAP:
            return sycl::addressing_mode::repeat;
        case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:
        case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE:
            return sycl::addressing_mode::mirrored_repeat;
        case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:
            return sycl::addressing_mode::clamp_to_edge;
        case D3D12_TEXTURE_ADDRESS_MODE_BORDER:
            return sycl::addressing_mode::clamp;
        default:
            return sycl::addressing_mode::none;
    }
}
//----------------------------------------------------------------------------//

//-==========================================================================-//
void FenceD3D12SyclInterop::importExternalFenceWin32Handle() {
    // https://github.com/intel/llvm/blob/sycl/sycl/doc/extensions/experimental/sycl_ext_oneapi_bindless_images.asciidoc
    syclexp::external_semaphore_handle_type semaphoreHandleType =
            syclexp::external_semaphore_handle_type::win32_nt_dx12_fence;
    syclexp::external_semaphore_descriptor<syclexp::resource_win32_handle>
            syclExternalSemaphoreDescriptor{handle, semaphoreHandleType};
    auto* wrapper = new SyclExternalSemaphoreWrapper;
    wrapper->syclExternalSemaphore = syclexp::import_external_semaphore(
            syclExternalSemaphoreDescriptor, *g_syclQueue);
    externalSemaphore = reinterpret_cast<void*>(wrapper);
}
//----------------------------------------------------------------------------//
void FenceD3D12SyclInterop::free() {
    freeHandle();
    if (externalSemaphore) {
        auto* wrapper = reinterpret_cast<SyclExternalSemaphoreWrapper*>(externalSemaphore);
        syclexp::release_external_semaphore(wrapper->syclExternalSemaphore, *g_syclQueue);
        delete wrapper;
        externalSemaphore = {};
    }
}
//----------------------------------------------------------------------------//
FenceD3D12SyclInterop::~FenceD3D12SyclInterop() {
    FenceD3D12SyclInterop::free();
}
//----------------------------------------------------------------------------//
void FenceD3D12SyclInterop::signalFenceComputeApi(
            StreamWrapper stream, unsigned long long timelineValue, void* eventIn, void* eventOut) {
    auto* wrapper = reinterpret_cast<SyclExternalSemaphoreWrapper*>(externalSemaphore);
    sycl::event syclEvent;
    if (eventIn) {
        syclEvent = stream.syclQueuePtr->ext_oneapi_signal_external_semaphore(
                wrapper->syclExternalSemaphore, uint64_t(timelineValue), *static_cast<sycl::event*>(eventIn));
    } else {
        syclEvent = stream.syclQueuePtr->ext_oneapi_signal_external_semaphore(
                wrapper->syclExternalSemaphore, uint64_t(timelineValue));
    }
    if (eventOut) {
        *static_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//----------------------------------------------------------------------------//
void FenceD3D12SyclInterop::waitFenceComputeApi(
        StreamWrapper stream, unsigned long long timelineValue, void* eventIn, void* eventOut) {
    auto* wrapper = reinterpret_cast<SyclExternalSemaphoreWrapper*>(externalSemaphore);
    sycl::event syclEvent;
    if (eventIn) {
        syclEvent = stream.syclQueuePtr->ext_oneapi_wait_external_semaphore(
                wrapper->syclExternalSemaphore, uint64_t(timelineValue), *static_cast<sycl::event*>(eventIn));
    } else {
        syclEvent = stream.syclQueuePtr->ext_oneapi_wait_external_semaphore(
                wrapper->syclExternalSemaphore, uint64_t(timelineValue));
    }
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//-==========================================================================-//

//-==========================================================================-//
void BufferD3D12SyclInterop::importExternalMemoryWin32Handle() {
    size_t numBytes = resource->getCopiableSizeInBytes();

    // https://github.com/intel/llvm/blob/sycl/sycl/doc/extensions/experimental/sycl_ext_oneapi_bindless_images.asciidoc
    auto memoryHandleType = syclexp::external_mem_handle_type::win32_nt_dx12_resource;
    syclexp::external_mem_descriptor<syclexp::resource_win32_handle>
            syclExternalMemDescriptor{(void*)handle, memoryHandleType, numBytes};
    auto* wrapper = new SyclExternalMemWrapper;
    wrapper->syclExternalMem = syclexp::import_external_memory(
            syclExternalMemDescriptor, *g_syclQueue);
    externalMemory = reinterpret_cast<void*>(wrapper);

    devicePtr = syclexp::map_external_linear_memory(
            wrapper->syclExternalMem, 0, numBytes, *g_syclQueue);
}
//----------------------------------------------------------------------------//
void BufferD3D12SyclInterop::free() {
    freeHandle();
    if (externalMemory) {
        auto* wrapper = reinterpret_cast<SyclExternalMemWrapper*>(externalMemory);
        syclexp::unmap_external_linear_memory(devicePtr, *g_syclQueue);
        syclexp::release_external_memory(wrapper->syclExternalMem, *g_syclQueue);
        delete wrapper;
        externalMemory = {};
    }
}
//----------------------------------------------------------------------------//
BufferD3D12SyclInterop::~BufferD3D12SyclInterop() {
    BufferD3D12SyclInterop::free();
}
//----------------------------------------------------------------------------//
void BufferD3D12SyclInterop::copyFromDevicePtrAsync(
        void* devicePtrSrc, StreamWrapper stream, void* eventOut) {
    auto syclEvent = stream.syclQueuePtr->memcpy(devicePtr, devicePtrSrc, resource->getCopiableSizeInBytes());
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//----------------------------------------------------------------------------//
void BufferD3D12SyclInterop::copyToDevicePtrAsync(
        void* devicePtrDst, StreamWrapper stream, void* eventOut) {
    auto syclEvent = stream.syclQueuePtr->memcpy(devicePtrDst, devicePtr, resource->getCopiableSizeInBytes());
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//----------------------------------------------------------------------------//
void BufferD3D12SyclInterop::copyFromHostPtrAsync(void* hostPtrSrc, StreamWrapper stream, void* eventOut) {
    auto syclEvent = stream.syclQueuePtr->memcpy(devicePtr, hostPtrSrc, resource->getCopiableSizeInBytes());
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//----------------------------------------------------------------------------//
void BufferD3D12SyclInterop::copyToHostPtrAsync(void* hostPtrDst, StreamWrapper stream, void* eventOut) {
    auto syclEvent = stream.syclQueuePtr->memcpy(hostPtrDst, devicePtr, resource->getCopiableSizeInBytes());
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//-==========================================================================-//

//-==========================================================================-//
void ImageD3D12SyclInterop::importExternalMemoryWin32Handle() {
    std::cerr << "ENTER: ImageD3D12SyclInterop::importExternalMemoryWin32Handle())\n";
    size_t numBytes = resource->getCopiableSizeInBytes();

    // https://github.com/intel/llvm/blob/sycl/sycl/doc/extensions/experimental/sycl_ext_oneapi_bindless_images.asciidoc
    auto memoryHandleType = syclexp::external_mem_handle_type::win32_nt_dx12_resource;
    syclexp::external_mem_descriptor<syclexp::resource_win32_handle>
            syclExternalMemDescriptor{(void*)handle, memoryHandleType, numBytes};
    auto* wrapper = new SyclExternalMemWrapper;
    wrapper->syclExternalMem = syclexp::import_external_memory(
            syclExternalMemDescriptor, *g_syclQueue);
    externalMemory = reinterpret_cast<void*>(wrapper);

    const auto& resourceDesc = resource->getD3D12ResourceDesc();
    auto* wrapperImg = new SyclImageMemHandleWrapper;
    syclexp::image_descriptor& imgDesc = wrapperImg->imgDesc;
    imgDesc.width = resourceDesc.Width;
    if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE1D &&
        resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D &&
        resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
        sgl::Logfile::get()->throwError(
                "Error in ImageD3D12SyclInterop::importExternalMemoryWin32Handle: Invalid D3D12 resource dimension.");
    }
    if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
        resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
        imgDesc.height = resourceDesc.Height;
    }
    if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
        // TODO: When imgDesc.array_size?
        imgDesc.depth = resourceDesc.DepthOrArraySize;
    }
    imgDesc.num_levels = resourceDesc.MipLevels;

    imgDesc.num_channels = unsigned(getDXGIFormatNumChannels(resourceDesc.Format));
    if (imgDesc.num_levels > 1) {
        imgDesc.type = syclexp::image_type::mipmap;
    } else {
        imgDesc.type = syclexp::image_type::standard;
    }

    imgDesc.channel_type = getSyclChannelType(resourceDesc.Format);

    imgDesc.verify();

    auto* wrapperMem = reinterpret_cast<SyclExternalMemWrapper*>(externalMemory);
    bool supportsHandle = false;
    std::vector<syclexp::image_memory_handle_type> handleTypes =
            syclexp::get_image_memory_support(imgDesc, *g_syclQueue);
    for (auto supportedHandleType : handleTypes) {
        if (supportedHandleType == syclexp::image_memory_handle_type::opaque_handle) {
            supportsHandle = true;
            break;
        }
    }
    if (!supportsHandle) {
        syclexp::release_external_memory(wrapperMem->syclExternalMem, *g_syclQueue);
        delete wrapperMem;
        externalMemory = nullptr;
        if (openMessageBoxOnComputeApiError) {
            sgl::Logfile::get()->writeError(
                    "Error in ImageD3D12SyclInterop::_initialize: "
                    "Unsupported SYCL image memory type.");
        } else {
            sgl::Logfile::get()->write(
                    "Error in ImageD3D12SyclInterop::_initialize: "
                    "Unsupported SYCL image memory type.", sgl::RED);
        }
        throw UnsupportedComputeApiFeatureException("Unsupported SYCL image memory type");
    }

    wrapperImg->memHandle = syclexp::map_external_image_memory(
            wrapperMem->syclExternalMem, imgDesc, *g_syclQueue);
    _mipmap = reinterpret_cast<void*>(wrapperImg);
    std::cerr << "LEAVE: ImageD3D12SyclInterop::importExternalMemoryWin32Handle())\n";
}
//----------------------------------------------------------------------------//
void ImageD3D12SyclInterop::free() {
    freeHandle();
    if (_mipmap) {
        auto* wrapperImg = reinterpret_cast<SyclImageMemHandleWrapper*>(_mipmap);
        syclexp::free_image_mem(
                wrapperImg->memHandle, wrapperImg->imgDesc.type, *g_syclQueue);
        delete wrapperImg;
        _mipmap = {};
    }
    if (externalMemory) {
        auto* wrapper = reinterpret_cast<SyclExternalMemWrapper*>(externalMemory);
        syclexp::release_external_memory(wrapper->syclExternalMem, *g_syclQueue);
        delete wrapper;
        externalMemory = {};
    }
}
//----------------------------------------------------------------------------//
ImageD3D12SyclInterop::~ImageD3D12SyclInterop() {
    ImageD3D12SyclInterop::free();
}
//----------------------------------------------------------------------------//
void ImageD3D12SyclInterop::copyFromDevicePtrAsync(
        void* devicePtrSrc, StreamWrapper stream, void* eventOut) {
    auto* wrapperImg = reinterpret_cast<SyclImageMemHandleWrapper*>(_mipmap);
    auto syclEvent = stream.syclQueuePtr->ext_oneapi_copy(
            devicePtrSrc, wrapperImg->memHandle, wrapperImg->imgDesc);
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
}
//----------------------------------------------------------------------------//
void ImageD3D12SyclInterop::copyToDevicePtrAsync(
        void* devicePtrDst, StreamWrapper stream, void* eventOut) {
    std::cerr << "ENTER: ImageD3D12SyclInterop::copyToDevicePtrAsync(void*, StreamWrapper, void*)\n";
    auto* wrapperImg = reinterpret_cast<SyclImageMemHandleWrapper*>(_mipmap);
    auto syclEvent = stream.syclQueuePtr->ext_oneapi_copy(
            wrapperImg->memHandle, devicePtrDst, wrapperImg->imgDesc);
    if (eventOut) {
        *reinterpret_cast<sycl::event*>(eventOut) = std::move(syclEvent);
    }
    std::cerr << "LEAVE: ImageD3D12SyclInterop::copyToDevicePtrAsync(void*, StreamWrapper, void*)\n";
}
//----------------------------------------------------------------------------//
void ImageD3D12SyclInterop::print() {
    std::cerr << "ENTER: ImageD3D12SyclInterop::print()\n";
    if (resource) resource->print();
    std::cout << std::format("  Sample image     : {}\n", imageComputeApiInfo.useSampledImage);
    std::cout << std::format("  Use mipmap       : {}\n", imageComputeApiInfo.textureExternalMemorySettings.useMipmappedArray);
    std::cout << std::format("  Normalize coords : {}\n", imageComputeApiInfo.textureExternalMemorySettings.useNormalizedCoordinates);
    std::cout << std::format("  Linear interp.   : {}\n", imageComputeApiInfo.textureExternalMemorySettings.useTrilinearOptimization);
    std::cout << std::format("  Read as int      : {}\n", imageComputeApiInfo.textureExternalMemorySettings.readAsInteger);
    std::cout << std::format("  Mipmap array     : {}\n", _mipmap);
    std::cout << std::format("  External memory  : {}\n", externalMemory);
    std::cout << std::format("  Handle           : {}\n", handle);
    std::cerr << "LEAVE: ImageD3D12SyclInterop::print()\n";
}
//-==========================================================================-//

//-==========================================================================-//
void UnsampledImageD3D12SyclInterop::initialize(const ImageD3D12ComputeApiExternalMemoryPtr& _image) {
    static_assert(sizeof(syclexp::unsampled_image_handle) == sizeof(rawImageHandle));
    this->image = _image;
    auto imageVkSycl = std::static_pointer_cast<ImageD3D12SyclInterop>(image);
    auto* wrapperImg = reinterpret_cast<SyclImageMemHandleWrapper*>(imageVkSycl->_mipmap);

    if (!syclexp::is_image_handle_supported<syclexp::unsampled_image_handle>(
            wrapperImg->imgDesc, syclexp::image_memory_handle_type::opaque_handle,
            *g_syclQueue)) {
        if (openMessageBoxOnComputeApiError) {
            sgl::Logfile::get()->writeError(
                    "Error in UnsampledImageD3D12SyclInterop::_initialize: "
                    "Unsupported SYCL image handle type.");
        } else {
            sgl::Logfile::get()->write(
                    "Error in UnsampledImageD3D12SyclInterop::_initialize: "
                    "Unsupported SYCL image handle type.", sgl::RED);
        }
        throw UnsupportedComputeApiFeatureException("Unsupported SYCL image handle type");
    }

    auto handle = syclexp::create_image(
            wrapperImg->memHandle, wrapperImg->imgDesc, *g_syclQueue);
    rawImageHandle = handle.raw_handle;
}
//----------------------------------------------------------------------------//
UnsampledImageD3D12SyclInterop::~UnsampledImageD3D12SyclInterop() {
    if (rawImageHandle) {
        syclexp::unsampled_image_handle handle{rawImageHandle};
        syclexp::destroy_image_handle(handle, *g_syclQueue);
        rawImageHandle = {};
    }
}
//----------------------------------------------------------------------------//
uint64_t UnsampledImageD3D12SyclInterop::getRawHandle() {
    return rawImageHandle;
}
//-==========================================================================-//

//-==========================================================================-//
void SampledImageD3D12SyclInterop::initialize(
            const ImageD3D12ComputeApiExternalMemoryPtr& _image,
            const TextureExternalMemorySettings& textureExternalMemorySettings) {
    static_assert(sizeof(syclexp::sampled_image_handle) == sizeof(rawImageHandle));
    this->image = _image;
    const auto& imageComputeApiInfo = image->getImageComputeApiInfo();
    const auto& samplerDesc = imageComputeApiInfo.samplerDesc;

    auto imageVkSycl = std::static_pointer_cast<ImageD3D12SyclInterop>(image);
    auto* wrapperImg = reinterpret_cast<SyclImageMemHandleWrapper*>(imageVkSycl->_mipmap);

    if (!syclexp::is_image_handle_supported<syclexp::sampled_image_handle>(
            wrapperImg->imgDesc, syclexp::image_memory_handle_type::opaque_handle,
            *g_syclQueue)) {
        if (openMessageBoxOnComputeApiError) {
            sgl::Logfile::get()->writeError(
                    "Error in SampledImageD3D12SyclInterop::_initialize: "
                    "Unsupported SYCL image handle type.");
        } else {
            sgl::Logfile::get()->write(
                    "Error in SampledImageD3D12SyclInterop::_initialize: "
                    "Unsupported SYCL image handle type.", sgl::RED);
        }
        throw UnsupportedComputeApiFeatureException("Unsupported SYCL image handle type");
    }

    syclexp::bindless_image_sampler syclSampler{};
    syclSampler.addressing[0] = getSyclSampAddrMode(samplerDesc.AddressU);
    syclSampler.addressing[1] = getSyclSampAddrMode(samplerDesc.AddressV);
    syclSampler.addressing[2] = getSyclSampAddrMode(samplerDesc.AddressW);
    syclSampler.coordinate =
            textureExternalMemorySettings.useNormalizedCoordinates
            ? sycl::coordinate_normalization_mode::normalized
            : sycl::coordinate_normalization_mode::unnormalized;
    syclSampler.filtering =
            samplerDesc.Filter == D3D12_FILTER_MIN_MAG_MIP_POINT
            || samplerDesc.Filter == D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR
            ? sycl::filtering_mode::nearest : sycl::filtering_mode::linear;
    syclSampler.mipmap_filtering =
            samplerDesc.Filter == D3D12_FILTER_MIN_MAG_MIP_POINT
            || samplerDesc.Filter == D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT
            || samplerDesc.Filter == D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT
            || samplerDesc.Filter == D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT
            || samplerDesc.Filter == D3D12_FILTER_MIN_MAG_ANISOTROPIC_MIP_POINT
            ? sycl::filtering_mode::nearest : sycl::filtering_mode::linear;
    syclSampler.cubemap_filtering = syclexp::cubemap_filtering_mode::disjointed;
    syclSampler.min_mipmap_level_clamp = samplerDesc.MinLOD;
    syclSampler.max_mipmap_level_clamp = samplerDesc.MaxLOD;
    syclSampler.max_anisotropy = samplerDesc.MaxAnisotropy;

    auto handle = syclexp::create_image(
            wrapperImg->memHandle, wrapperImg->imgDesc, *g_syclQueue);
    rawImageHandle = handle.raw_handle;
}
//----------------------------------------------------------------------------//
SampledImageD3D12SyclInterop::~SampledImageD3D12SyclInterop() {
    if (rawImageHandle) {
        syclexp::sampled_image_handle handle{rawImageHandle};
        syclexp::destroy_image_handle(handle, *g_syclQueue);
        rawImageHandle = {};
    }
}
//----------------------------------------------------------------------------//
uint64_t SampledImageD3D12SyclInterop::getRawHandle() {
    return rawImageHandle;
}
//-==========================================================================-//

}}
