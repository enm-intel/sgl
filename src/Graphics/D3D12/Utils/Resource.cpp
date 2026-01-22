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

#include <iostream> // for std::cout, cerr
#include <format>   // for std::format

#include "../Render/CommandList.hpp"
#include "Device.hpp"
#include "Resource.hpp"

#include "Math/Math.hpp"

namespace sgl { namespace d3d12 {

//----------------------------------------------------------------------------//
uint32_t getDimensions(D3D12_RESOURCE_DIMENSION dim) {
    switch (dim) {
        case D3D12_RESOURCE_DIMENSION_UNKNOWN  : return 0;
        case D3D12_RESOURCE_DIMENSION_BUFFER   : return 1;
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D: return 1;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D: return 2;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D: return 3;
    }
    return 0;
}
//----------------------------------------------------------------------------//
std::string getTextureLayout(D3D12_TEXTURE_LAYOUT layout) {
    switch (layout) {
        case D3D12_TEXTURE_LAYOUT_UNKNOWN               : return std::string("Unknown");
        case D3D12_TEXTURE_LAYOUT_ROW_MAJOR             : return std::string("Row Major");
        case D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE: return std::string("64KB Tiled (Undefined Swizzle)");
        case D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE : return std::string("64KB Tiled (Standard Swizzle)");
    }
    return std::string("Unspecified");
}
//----------------------------------------------------------------------------//
std::string getResourceFlags(D3D12_RESOURCE_FLAGS flags) {
    std::string flagStr = std::format("(0x{:X})", static_cast<uint32_t>(flags));

    if (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
        flagStr += " ALLOW_RENDER_TARGET";
    }
    if (flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
        flagStr += " ALLOW_DEPTH_STENCIL";
    }
    if (flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
        flagStr += " ALLOW_UNORDERED_ACCESS";
    }
    if (flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) {
        flagStr += " DENY_SHADER_RESOURCE";
    }
    if (flags & D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER) {
        flagStr += " ALLOW_CROSS_ADAPTER";
    }
    if (flags & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS) {
        flagStr += " ALLOW_SIMULTANEOUS_ACCESS";
    }
    if (flags & D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY) {
        flagStr += " VIDEO_DECODE_REFERENCE_ONLY";
    }
    if (flags & D3D12_RESOURCE_FLAG_VIDEO_ENCODE_REFERENCE_ONLY) {
        flagStr += " VIDEO_ENCODE_REFERENCE_ONLY";
    }
    if (flags & D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE) {
        flagStr += " RAYTRACING_ACCELERATION_STRUCTURE";
    }
    return flagStr;
}
//----------------------------------------------------------------------------//
std::string getHeapFlags(D3D12_HEAP_FLAGS flags) {
    std::string flagStr = std::format("(0x{:X})", static_cast<uint32_t>(flags));

    if (flags & D3D12_HEAP_FLAG_SHARED) {
        flagStr += " SHARED";
    }
    if (flags & D3D12_HEAP_FLAG_DENY_BUFFERS) {
        flagStr += " DENY_BUFFERS";
    }
    if (flags & D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES) {
        flagStr += " DENY_RT_DS_TEXTURES";
    }
    if (flags & D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES) {
        flagStr += " DENY_NON_RT_DS_TEXTURES";
    }
    if (flags & D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS) {
        flagStr += " ALLOW_ONLY_BUFFERS";
    }
    if (flags & D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES) {
        flagStr += " ALLOW_ONLY_NON_RT_DS_TEXTURES";
    }
    if (flags & D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES) {
        flagStr += " ALLOW_ONLY_RT_DS_TEXTURES";
    }
    return flagStr;
}
//----------------------------------------------------------------------------//
std::string getResourceStates(D3D12_RESOURCE_STATES states) {
    std::string statesStr = std::format("(0x{:X})", static_cast<uint32_t>(states));
    if (states & D3D12_RESOURCE_STATE_COMMON) {
        statesStr += " COMMON";
    }
    if (states & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) {
        statesStr += " VERTEX_AND_CONSTANT_BUFFER";
    }
    if (states & D3D12_RESOURCE_STATE_INDEX_BUFFER) {
        statesStr += " INDEX_BUFFER";
    }
    if (states & D3D12_RESOURCE_STATE_RENDER_TARGET) {
        statesStr += " RENDER_TARGET";
    }
    if (states & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        statesStr += " UNORDERED_ACCESS";
    }
    if (states & D3D12_RESOURCE_STATE_DEPTH_WRITE) {
        statesStr += " DEPTH_WRITE";
    }
    if (states & D3D12_RESOURCE_STATE_DEPTH_READ) {
        statesStr += " DEPTH_READ";
    }
    if (states & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
        statesStr += " NON_PIXEL_SHADER_RESOURCE";
    }
    if (states & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        statesStr += " PIXEL_SHADER_RESOURCE";
    }
    if (states & D3D12_RESOURCE_STATE_STREAM_OUT) {
        statesStr += " STREAM_OUT";
    }
    if (states & D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT) {
        statesStr += " INDIRECT_ARGUMENT";
    }
    if (states & D3D12_RESOURCE_STATE_COPY_DEST) {
        statesStr += " COPY_DEST";
    }
    if (states & D3D12_RESOURCE_STATE_COPY_SOURCE) {
        statesStr += " COPY_SOURCE";
    }
    if (states & D3D12_RESOURCE_STATE_RESOLVE_DEST) {
        statesStr += " RESOLVE_DEST";
    }
    if (states & D3D12_RESOURCE_STATE_RESOLVE_SOURCE) {
        statesStr += " RESOLVE_SOURCE";
    }
    return statesStr;
}
//----------------------------------------------------------------------------//
size_t getDXGIFormatNumChannels(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_D32_FLOAT:
            return 1;
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return 2;
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return 3;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return 4;
        default:
            return 0;
    }
}
//----------------------------------------------------------------------------//
size_t getDXGIFormatSizeInBytes(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SINT:
            return 1;
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SINT:
            return 2;
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return 4;
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return 8;
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return 12;
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return 16;
        default:
            return 0;
    }
}
//----------------------------------------------------------------------------//

//-==========================================================================-//
Resource::Resource(Device* device, const ResourceSettings& settings)
        : device(device), settings(settings) {

    std::cerr << "ENTER: Resource::Resource(Device*, const ResourceSettings&)\n";
    auto* d3d12Device = device->getD3D12Device2();
    D3D12_CLEAR_VALUE clearValue{};
    const D3D12_CLEAR_VALUE* optimizedClearValue = nullptr;
    if (settings.optimizedClearValue.has_value()) {
        memcpy(&clearValue, &settings.optimizedClearValue.value(), sizeof(D3D12_CLEAR_VALUE));
        if (clearValue.Format == DXGI_FORMAT_UNKNOWN) {
            clearValue.Format = settings.resourceDesc.Format;
        }
        optimizedClearValue = &clearValue;
    }
    ThrowIfFailed(d3d12Device->CreateCommittedResource(
            &settings.heapProperties,
            settings.heapFlags,
            &settings.resourceDesc,
            settings.resourceStates,
            optimizedClearValue,
            IID_PPV_ARGS(&resource)));

    uint32_t arraySize;
    if (settings.resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE3D) {
        arraySize = settings.resourceDesc.DepthOrArraySize;
    } else {
        arraySize = 1;
    }
    auto formatPlaneCount = uint32_t(D3D12GetFormatPlaneCount(d3d12Device, settings.resourceDesc.Format));
    numSubresources = uint32_t(settings.resourceDesc.MipLevels) * arraySize * formatPlaneCount;
    std::cerr << "LEAVE: Resource::Resource(Device*, const ResourceSettings&)\n";
}
//----------------------------------------------------------------------------//
Resource::~Resource() = default;
//----------------------------------------------------------------------------//
void* Resource::map() {
    return map(0, getCopiableSizeInBytes());
}
//----------------------------------------------------------------------------//
void* Resource::map(size_t readRangeBegin, size_t readRangeEnd) {
    D3D12_RANGE readRange = { readRangeBegin, readRangeEnd };
    void* dataPtr = nullptr;
    if (FAILED(resource->Map(0, &readRange, &dataPtr))) {
        sgl::Logfile::get()->throwError("Error in Resource::map: CPU mapping of resource failed.");
    }
    return dataPtr;
}
//----------------------------------------------------------------------------//
void Resource::unmap() {
    D3D12_RANGE writtenRange = { 0, 0 };
    resource->Unmap(0, &writtenRange);
}
//----------------------------------------------------------------------------//
void Resource::unmap(size_t writtenRangeBegin, size_t writtenRangeEnd) {
    D3D12_RANGE writtenRange = { writtenRangeBegin, writtenRangeEnd };
    resource->Unmap(0, &writtenRange);
}
//----------------------------------------------------------------------------//
void Resource::uploadDataLinear(size_t sizeInBytesData, const void* dataPtr) {
    std::cerr << "ENTER: Resource::uploadDataLinear(size_t, const void*)\n";
    size_t intermediateSizeInBytes;
    if (settings.resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        std::cerr << "  Resource is a buffer.\n";
        intermediateSizeInBytes = sizeInBytesData;
        if (sizeInBytesData > getCopiableSizeInBytes()) {
            sgl::Logfile::get()->throwError(
                    "Error in Resource::uploadDataLinear: "
                    "The copy source is larger than the destination buffer.");
        }
    } else {
        std::cerr << "  Resource is a texture.\n";
        intermediateSizeInBytes = getCopiableSizeInBytes();
        std::cerr << "  Copiable size in bytes: " << intermediateSizeInBytes << "\n";
        if (sizeInBytesData > getRowSizeInBytes() * settings.resourceDesc.Height * settings.resourceDesc.DepthOrArraySize) {
            sgl::Logfile::get()->throwError(
                    "Error in Resource::readBackDataInternal: "
                    "The copy source is larger than the destination texture.");
        }
    }

    queryCopiableFootprints();
    auto* d3d12Device = device->getD3D12Device2();
    CD3DX12_HEAP_PROPERTIES heapPropertiesUpload(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDescUpload = CD3DX12_RESOURCE_DESC::Buffer(intermediateSizeInBytes);
    ComPtr<ID3D12Resource> intermediateResource{};
    // https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
    // "Upload heaps must start out in the state D3D12_RESOURCE_STATE_GENERIC_READ"
    // "Readback heaps must start out in the D3D12_RESOURCE_STATE_COPY_DEST state"
    ThrowIfFailed(d3d12Device->CreateCommittedResource(
            &heapPropertiesUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferDescUpload,
            D3D12_RESOURCE_STATE_COPY_SOURCE,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));

    device->runOnce([&](CommandList* cmdList){
        this->transition(D3D12_RESOURCE_STATE_COPY_DEST, cmdList);
        uploadDataLinearInternal(sizeInBytesData, dataPtr, intermediateResource.Get(), cmdList);
    });
    std::cerr << "LEAVE: Resource::uploadDataLinear(size_t, const void*)\n";
}
//----------------------------------------------------------------------------//
void Resource::uploadDataLinear(
        size_t sizeInBytesData, const void* dataPtr,
        const ResourcePtr& intermediateResource, const CommandListPtr& cmdList) {
    uploadDataLinearInternal(sizeInBytesData, dataPtr, intermediateResource->getD3D12ResourcePtr(), cmdList.get());
}
//----------------------------------------------------------------------------//
void Resource::uploadDataLinearInternal(
        size_t sizeInBytesData, const void* dataPtr,
        ID3D12Resource* intermediateResource, CommandList* cmdList) {
    std::cerr << "ENTER: Resource::uploadDataLinearInternal(size_t, const void*, ID3D12Resource*, CommandList*)\n";
    auto* d3d12CommandList = cmdList->getD3D12GraphicsCommandListPtr();
    D3D12_SUBRESOURCE_DATA subData = {};
    subData.pData = dataPtr;
    if (settings.resourceDesc.Height <= 1 && settings.resourceDesc.DepthOrArraySize <= 1) {
        // 1D data (no pitches necessary).
        subData.RowPitch = LONG_PTR(sizeInBytesData);
        subData.SlicePitch = subData.RowPitch;
    } else if (settings.resourceDesc.DepthOrArraySize <= 1) {
        // 2D data (no slice pitch necessary).
        subData.RowPitch = LONG_PTR(getRowSizeInBytes());
        subData.SlicePitch = subData.RowPitch * LONG_PTR(settings.resourceDesc.Width);
    } else {
        // 3D Data.
        subData.RowPitch = LONG_PTR(getRowSizeInBytes());
        subData.SlicePitch = subData.RowPitch * LONG_PTR(settings.resourceDesc.Width * settings.resourceDesc.Height);
    }

    queryCopiableFootprints();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT& layout =_subLayout.at(0);
    UINT64 rowSize   =_subRowSize.at(0);
    UINT   numRows   =_subNumRows.at(0);
    UINT64 totalSize =_subTotalSize.at(0);
    UpdateSubresources(
            d3d12CommandList, getD3D12ResourcePtr(), intermediateResource, 0, 1,
            totalSize, &layout, &numRows, &rowSize, &subData);
    std::cerr << "LEAVE: Resource::uploadDataLinearInternal(size_t, const void*, ID3D12Resource*, CommandList*)\n";
}
//----------------------------------------------------------------------------//
void Resource::readBackDataLinear(size_t sizeInBytesData, void* dataPtr) {
    std::cerr << "ENTER: Resource::readBackDataLinear(size_t, void*)\n";
    if (numSubresources > 1) {
        sgl::Logfile::get()->throwError(
                "Error in Resource::readBackDataInternal: "
                "The function only supports for resources with one single subresource.");
    }
    if (settings.resourceDesc.SampleDesc.Count > 1) {
        sgl::Logfile::get()->throwError(
                "Error in Resource::readBackDataInternal: "
                "The function does not support multi-sampled resources.");
    }

    size_t rowSize = 0;
    size_t srcRowPitch;
    size_t intermediateSizeInBytes;
    if (settings.resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        srcRowPitch = sizeInBytesData;
        intermediateSizeInBytes = srcRowPitch;
        if (sizeInBytesData > getCopiableSizeInBytes()) {
            sgl::Logfile::get()->throwError(
                    "Error in Resource::readBackDataInternal: "
                    "The copy destination is larger than the source buffer.");
        }
    } else {
        rowSize = getRowSizeInBytes();
        srcRowPitch = getRowPitchInBytes();
        intermediateSizeInBytes = srcRowPitch;
        if (settings.resourceDesc.Height > 1) {
            intermediateSizeInBytes *= settings.resourceDesc.Height;
        }
        if (settings.resourceDesc.DepthOrArraySize > 1) {
            intermediateSizeInBytes *= settings.resourceDesc.DepthOrArraySize;
        }
        if (sizeInBytesData > rowSize * settings.resourceDesc.Height * settings.resourceDesc.DepthOrArraySize) {
            sgl::Logfile::get()->throwError(
                    "Error in Resource::readBackDataInternal: "
                    "The copy destination is larger than the source texture.");
        }
    }

    auto* d3d12Device = device->getD3D12Device2();
    CD3DX12_HEAP_PROPERTIES heapPropertiesReadBack(D3D12_HEAP_TYPE_READBACK);
    auto bufferDescReadBack = CD3DX12_RESOURCE_DESC::Buffer(intermediateSizeInBytes);
    ComPtr<ID3D12Resource> intermediateResource{};
    ThrowIfFailed(d3d12Device->CreateCommittedResource(
            &heapPropertiesReadBack,
            D3D12_HEAP_FLAG_NONE,
            &bufferDescReadBack,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));

    device->runOnce([&](CommandList* cmdList){
        auto* d3d12CommandList = cmdList->getD3D12GraphicsCommandListPtr();
        if (settings.resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
            d3d12CommandList->CopyBufferRegion(
                    intermediateResource.Get(), 0, resource.Get(), 0, sizeInBytesData);
        } else {
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};
            bufferFootprint.Footprint.Width = static_cast<UINT>(settings.resourceDesc.Width);
            bufferFootprint.Footprint.Height = settings.resourceDesc.Height;
            bufferFootprint.Footprint.Depth = settings.resourceDesc.DepthOrArraySize;
            bufferFootprint.Footprint.RowPitch = static_cast<UINT>(srcRowPitch);
            bufferFootprint.Footprint.Format = settings.resourceDesc.Format;

            const CD3DX12_TEXTURE_COPY_LOCATION Dst(intermediateResource.Get(), bufferFootprint);
            const CD3DX12_TEXTURE_COPY_LOCATION Src(resource.Get(), 0);
            this->transition(D3D12_RESOURCE_STATE_COPY_SOURCE, cmdList);
            d3d12CommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
        }
    });

    uint8_t* intermediateDataPtr;
    D3D12_RANGE readRange = { 0, sizeInBytesData };
    D3D12_RANGE writtenRange = { 0, 0 };
    if (FAILED(intermediateResource->Map(0, &readRange, reinterpret_cast<void**>(&intermediateDataPtr)))) {
        sgl::Logfile::get()->throwError(
                "Error: Resource::readBackDataInternal: ID3D12Resource::Map failed.");
    }
    D3D12_MEMCPY_DEST memcpyDest{};
    memcpyDest.pData = dataPtr;
    D3D12_SUBRESOURCE_DATA subresourceSrc{};
    subresourceSrc.pData = intermediateDataPtr;
    if (settings.resourceDesc.Height <= 1 && settings.resourceDesc.DepthOrArraySize <= 1) {
        // 1D data (no pitches necessary).
        memcpyDest.RowPitch = SIZE_T(sizeInBytesData);
        memcpyDest.SlicePitch = memcpyDest.RowPitch;
        subresourceSrc.RowPitch = LONG_PTR(sizeInBytesData);
        subresourceSrc.SlicePitch = subresourceSrc.RowPitch;
    } else if (settings.resourceDesc.DepthOrArraySize <= 1) {
        // 2D data (no slice pitch necessary).
        memcpyDest.RowPitch = SIZE_T(rowSize);
        memcpyDest.SlicePitch = memcpyDest.RowPitch * SIZE_T(settings.resourceDesc.Width);
        subresourceSrc.RowPitch = LONG_PTR(srcRowPitch);
        subresourceSrc.SlicePitch = subresourceSrc.RowPitch * LONG_PTR(settings.resourceDesc.Width);
    } else {
        // 3D Data.
        memcpyDest.RowPitch = SIZE_T(rowSize);
        memcpyDest.SlicePitch = memcpyDest.RowPitch * SIZE_T(settings.resourceDesc.Width * settings.resourceDesc.Height);
        subresourceSrc.RowPitch = LONG_PTR(srcRowPitch);
        memcpyDest.SlicePitch = subresourceSrc.RowPitch * LONG_PTR(settings.resourceDesc.Width * settings.resourceDesc.Height);
    }
    MemcpySubresource(
            &memcpyDest, &subresourceSrc, memcpyDest.RowPitch, settings.resourceDesc.Height,
            settings.resourceDesc.DepthOrArraySize);
    intermediateResource->Unmap(0, &writtenRange);
    std::cerr << "LEAVE: Resource::readBackDataLinear(size_t, void*)\n";
}
//----------------------------------------------------------------------------//
void Resource::transition(
        D3D12_RESOURCE_STATES stateAfter, const CommandListPtr& cmdList) {
    transition(settings.resourceStates, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, cmdList);
}
//----------------------------------------------------------------------------//
void Resource::transition(
        D3D12_RESOURCE_STATES stateAfter, CommandList* cmdList) {
    transition(settings.resourceStates, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, cmdList);
}
//----------------------------------------------------------------------------//
void Resource::transition(
        D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, const CommandListPtr& cmdList) {
    transition(stateBefore, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, cmdList);
}
//----------------------------------------------------------------------------//
void Resource::transition(
        D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, CommandList* cmdList) {
    transition(stateBefore, stateAfter, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, cmdList);
}
//----------------------------------------------------------------------------//
void Resource::transition(
        D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresourcce,
        const CommandListPtr& cmdList) {
    transition(stateBefore, stateAfter, subresourcce, cmdList.get());
}
//----------------------------------------------------------------------------//
void Resource::transition(
        D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresourcce,
        CommandList* cmdList) {
    ID3D12GraphicsCommandList* d3d12GraphicsCommandList = cmdList->getD3D12GraphicsCommandListPtr();
    D3D12_RESOURCE_BARRIER resourceBarrier{};
    resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resourceBarrier.Transition.pResource = resource.Get();
    resourceBarrier.Transition.Subresource = subresourcce;
    resourceBarrier.Transition.StateBefore = stateBefore;
    resourceBarrier.Transition.StateAfter = stateAfter;
    d3d12GraphicsCommandList->ResourceBarrier(1, &resourceBarrier);
    settings.resourceStates = stateAfter;
}
//----------------------------------------------------------------------------//
void Resource::barrierUAV(const CommandListPtr& cmdList) {
    barrierUAV(cmdList.get());
}
//----------------------------------------------------------------------------//
void Resource::barrierUAV(CommandList* cmdList) {
    ID3D12GraphicsCommandList* d3d12GraphicsCommandList = cmdList->getD3D12GraphicsCommandListPtr();
    D3D12_RESOURCE_BARRIER resourceBarrier{};
    resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    resourceBarrier.UAV.pResource = resource.Get();
    d3d12GraphicsCommandList->ResourceBarrier(1, &resourceBarrier);
}
//----------------------------------------------------------------------------//
size_t Resource::getAllocationSizeInBytes() {
    auto* d3d12Device = device->getD3D12Device2();
#if defined(_MSC_VER) || !defined(_WIN32)
    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = d3d12Device->GetResourceAllocationInfo(
            0, 1, &settings.resourceDesc);
#else
    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo{};
    d3d12Device->GetResourceAllocationInfo(
            &allocationInfo, 0, 1, &settings.resourceDesc);
#endif
    return allocationInfo.SizeInBytes;
}
//----------------------------------------------------------------------------//
void Resource::queryCopiableFootprints() {
    if (!_subLayout.empty()) return;
    std::cerr << "ENTER: Resource::queryCopiableFootprints()\n";
    auto* d3d12Device = device->getD3D12Device2();
    uint32_t numEntries = std::max(numSubresources, uint32_t(1));
    std::cerr << "  Number of subresource entries: " << numEntries << "\n";
    _subLayout.resize(numEntries);
    _subNumRows.resize(numEntries);
    _subRowSize.resize(numEntries);
    _subTotalSize.resize(numEntries);
    d3d12Device->GetCopyableFootprints(
           &settings.resourceDesc, 0, numEntries, 0,
           _subLayout.data(),_subNumRows.data(),
           _subRowSize.data(),_subTotalSize.data());
    for (uint32_t i = 0; i < numEntries; ++i) {
        auto& layout = _subLayout.at(i);
        std::cout << std::format("    {}: Offset={}, Footprint=(Format={}, "
                                 "Width={}, Height={}, Depth={}, RowPitch={})"
                                 "  # Rows={}  Row Size={}  Total Bytes={}\n",
                                 i, layout.Offset,
                                 static_cast<uint32_t>(layout.Footprint.Format),
                                 layout.Footprint.Width,
                                 layout.Footprint.Height,
                                 layout.Footprint.Depth,
                                 layout.Footprint.RowPitch,
                                 _subNumRows.at(i),
                                 _subRowSize.at(i),
                                 _subTotalSize.at(i));
    }
    
    std::cerr << "LEAVE: Resource::queryCopiableFootprints()\n";
}
//----------------------------------------------------------------------------//
size_t Resource::getCopiableSizeInBytes() {
    queryCopiableFootprints();
    return size_t(_subTotalSize.at(0));
}
//----------------------------------------------------------------------------//
size_t Resource::getNumRows() {
    queryCopiableFootprints();
    return size_t(_subNumRows.at(0));
}
//----------------------------------------------------------------------------//
size_t Resource::getRowSizeInBytes() {
    queryCopiableFootprints();
    return size_t(_subRowSize.at(0));
}
//----------------------------------------------------------------------------//
size_t Resource::getRowPitchInBytes() {
    queryCopiableFootprints();
    size_t rowSize = getRowSizeInBytes();
    if (rowSize % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT == 0) {
        return rowSize;
    }
    return sgl::sizeceil(rowSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}
//----------------------------------------------------------------------------//
D3D12_GPU_VIRTUAL_ADDRESS Resource::getGPUVirtualAddress() {
    return resource->GetGPUVirtualAddress();
}
//----------------------------------------------------------------------------//
HANDLE Resource::getSharedHandle(const std::wstring& handleName) {
    auto* d3d12Device = device->getD3D12Device2();
    HANDLE fenceHandle{};
    ThrowIfFailed(d3d12Device->CreateSharedHandle(
            resource.Get(), nullptr, GENERIC_ALL, handleName.data(), &fenceHandle));
    return fenceHandle;
}
//----------------------------------------------------------------------------//
HANDLE Resource::getSharedHandle() {
    uint64_t resourceIdx = 0;
    // TODO: Apparently, the handle name may be "nullptr".
    std::wstring handleName = std::wstring(L"Local\\D3D12ResourceHandle") + std::to_wstring(resourceIdx);
    return getSharedHandle(handleName);
}
//----------------------------------------------------------------------------//
void Resource::print() {
    std::cout << "Resource:\n";
    if (device) {
        std::cout << "  Device  :\n";
        std::cout << std::format("    Adapter name   : {}\n", device->getAdapterName());
        std::cout << std::format("    Vendor ID      : {}\n", deviceVendor(device->getVendor()));
        std::cout << std::format("    Feature level  : {}\n", featureLevel(device->getFeatureLevel()));
        //std::cout << std::format("    DX12 device    : {}\n", device->getD3D12Device2());
    }
    else std::cout << "  Device  : NOT SET\n";

    uint32_t dim = getDimensions(settings.resourceDesc.Dimension);
    std::string resourceFlags  = getResourceFlags(settings.resourceFlags);
    std::string heapFlags      = getHeapFlags(settings.heapFlags);
    std::string resourceStates = getResourceStates(settings.resourceStates);
    std::string layout         = getTextureLayout(settings.resourceDesc.Layout);

    DXGI_SAMPLE_DESC SampleDesc;
    D3D12_TEXTURE_LAYOUT Layout;
    D3D12_RESOURCE_FLAGS Flags;
    std::cout << "  Settings:\n";
    std::cout << std::format("    Resourse flags : {}\n", resourceFlags);
    std::cout << std::format("    Heap     flags : {}\n", heapFlags);
    std::cout << std::format("    Resourse states: {}\n", resourceStates);
    std::cout << std::format("    Dimensions     : {}\n", dim);
    std::cout << std::format("    Width          : {}\n", settings.resourceDesc.Width);
    std::cout << std::format("    Height         : {}\n", settings.resourceDesc.Height);
    std::cout << std::format("    Depth/Arraysize: {}\n", settings.resourceDesc.DepthOrArraySize);
    std::cout << std::format("    Alignment      : {}\n", settings.resourceDesc.Alignment);
    std::cout << std::format("    Mip Levels     : {}\n", settings.resourceDesc.MipLevels);
    std::cout << std::format("    Format         : {}\n", static_cast<uint32_t>(settings.resourceDesc.Format));
    std::cout << std::format("    Sample desc.   : {} multisamples\n", settings.resourceDesc.SampleDesc.Count);
    std::cout << std::format("                     {} quality\n", settings.resourceDesc.SampleDesc.Quality);
    std::cout << std::format("    Layout         : {}\n", layout);
    std::cout << std::format("    Desc. flags    : {}\n", getResourceFlags(settings.resourceDesc.Flags));
    std::cout << std::format("    Subresources   : {}\n", numSubresources);
    for (uint32_t i = 0; i < numSubresources; ++i) {
        auto& layout = _subLayout.at(i);
        std::cout << std::format("    {}: Offset={}, Footprint=(Format={}, "
                                 "Width={}, Height={}, Depth={}, RowPitch={})"
                                 "  # Rows={}  Row Size={}  Total Bytes={}\n",
                                 i, layout.Offset,
                                 static_cast<uint32_t>(layout.Footprint.Format),
                                 layout.Footprint.Width,
                                 layout.Footprint.Height,
                                 layout.Footprint.Depth,
                                 layout.Footprint.RowPitch,
                                 _subNumRows.at(i),
                                 _subRowSize.at(i),
                                 _subTotalSize.at(i));
    }
}
//-==========================================================================-//

}}
