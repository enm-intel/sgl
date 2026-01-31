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

#define DLL_OBJECT

#include <stdexcept>

#include "SyclDeviceCode.hpp"

using sycl_unsamp_img = syclexp::unsampled_image_handle;

constexpr auto sgl_u32 = sgl::ChannelFormat::UINT32;
constexpr auto sgl_u16 = sgl::ChannelFormat::UINT16;
constexpr auto sgl_f32 = sgl::ChannelFormat::FLOAT32;
constexpr auto sgl_f16 = sgl::ChannelFormat::FLOAT16;  // Half float.

//----------------------------------------------------------------------------//
template<typename T>
sycl::event _writeSyclBufData(sycl::queue &q, size_t num, T *ptr) {
    auto event = q.submit([&](sycl::handler &cgh) {
        cgh.parallel_for<class LinearWriteKernel>(sycl::range<1>{num}, [=](sycl::id<1> it) {
            const auto idx = it[0];
            ptr[idx] = static_cast<T>(idx);
        });
    });
    return event;
}
//----------------------------------------------------------------------------//
sycl::event writeSyclBufferData(sycl::queue &queue, size_t numEntries,
                                float *devPtr) {
    return _writeSyclBufData(queue, numEntries, devPtr);
}
//----------------------------------------------------------------------------//
template<typename T, uint32_t C=1>
sycl::event _copyImgToBuf(sycl::queue &q, sycl_unsamp_img img,
                          size_t w, size_t h, T *ptr,
                          const sycl::event &wait) {
    auto event = q.submit([&](sycl::handler &cgh) {
        cgh.depends_on(wait);
        cgh.parallel_for(sycl::range<2>{w, h}, [=](sycl::id<2> it) {
            const auto x = it[0];
            const auto y = it[1];
            const size_t idx = (x + y * w) * C;
            const auto data = syclexp::fetch_image<sycl::vec<T, C>>(img, sycl::int2{x, y});
            for (uint32_t c = 0; c < C; c++) ptr[idx + c] = data[c];
        });
    });
    return event;
}
//----------------------------------------------------------------------------//
template<typename T>
sycl::event _copyImgToBuf(sycl::queue &q, sycl_unsamp_img img,
                          size_t channels, size_t w, size_t h,
                          void *devPtr, const sycl::event &wait) {
    T *ptr = static_cast<T *>(devPtr);
    switch (channels) {
        case 1: return _copyImgToBuf<T, 1>(q, img, w, h, ptr, wait);
        case 2: return _copyImgToBuf<T, 2>(q, img, w, h, ptr, wait);
        case 4: return _copyImgToBuf<T, 4>(q, img, w, h, ptr, wait);
    }
    throw std::runtime_error("Error in copySyclBindlessImgToBuf: "
                             "Unsupported number of channels.");
}
//----------------------------------------------------------------------------//
sycl::event copySyclBindlessImgToBuf(sycl::queue &queue, sycl_unsamp_img img,
                                     const sgl::FormatInfo &format,
                                     size_t wdth, size_t hght, void *devPtr,
                                     const sycl::event &wait) {
    const size_t channels = format.numChannels;
    const auto   frmt     = format.channelFormat;
    if (frmt == sgl_f32) return _copyImgToBuf<float     >(queue, img, channels, wdth, hght, devPtr, wait);
    if (frmt == sgl_u32) return _copyImgToBuf<uint32_t  >(queue, img, channels, wdth, hght, devPtr, wait);
    if (frmt == sgl_u16) return _copyImgToBuf<uint16_t  >(queue, img, channels, wdth, hght, devPtr, wait);
    if (frmt == sgl_f16) return _copyImgToBuf<sycl::half>(queue, img, channels, wdth, hght, devPtr, wait);
    throw std::runtime_error("Error in copySyclBindlessImgToBuf: "
                             "Unsupported channel format.");
    return sycl::event();
}
//----------------------------------------------------------------------------//
// Writes a SYCL test image with increasing indices per channel.
template<typename T, uint32_t C=1>
sycl::event _writeTestImg(sycl::queue &q, sycl_unsamp_img img,
                          size_t w, size_t h) {
    auto event = q.submit([&](sycl::handler &cgh) {
        cgh.parallel_for(sycl::range<2>{w, h}, [=](sycl::id<2> it) {
            const auto x = it[0];
            const auto y = it[1];
            const auto index = (x + y * width) * static_cast<size_t>(C);
            if constexpr (std::is_same_v<T, sycl::half>) {
                sycl::vec<uint16_t, C> data;
                for (int c = 0; c < C; c++) {
                    data[c] = sycl::bit_cast<uint16_t>(sycl::half(float(index + c)));
                    //data[c] = sycl::detail::float2Half(index + c); // may not use __SYCL_DEVICE_ONLY__ code path
                    //data[c] = sycl::half(index + c); // sycl::half constructor doesn't work, converts to float
                }
                syclexp::write_image<sycl::vec<uint16_t, C>>(img, sycl::int2{x, y}, data);
            } else {
                sycl::vec<T, C> data;
                for (int c = 0; c < C; c++) {
                    data[c] = T(index + c);
                }
                syclexp::write_image<sycl::vec<T, C>>(img, sycl::int2{x, y}, data);
            }
        });
    });
    return event;
}
//----------------------------------------------------------------------------//
template<typename T>
sycl::event _writeTestImg(sycl::queue &q, sycl_unsamp_img img,
                          size_t channels, size_t w, size_t h) {
    switch (channels) {
        case 1: return _writeTestImg<T, 1>(q, img, w, h);
        case 2: return _writeTestImg<T, 2>(q, img, w, h);
        case 4: return _writeTestImg<T, 4>(q, img, w, h);
    }
    throw std::runtime_error("Error in writeSyclBindlessTestImg: "
                             "Unsupported number of channels.");
}
//----------------------------------------------------------------------------//
// Writes a SYCL test image with increasing indices per channel.
sycl::event writeSyclBindlessTestImg(sycl::queue &queue, sycl_unsamp_img img,
                                     const sgl::FormatInfo &format,
                                     size_t wdth, size_t hght) {
    const size_t channels = format.numChannels;
    const auto   frmt     = format.channelFormat;
    if (frmt == sgl_f32) return _writeTestImg<float     >(queue, img, channels, wdth, hght);
    if (frmt == sgl_u32) return _writeTestImg<uint32_t  >(queue, img, channels, wdth, hght);
    if (frmt == sgl_u16) return _writeTestImg<uint16_t  >(queue, img, channels, wdth, hght);
    if (frmt == sgl_f16) return _writeTestImg<sycl::half>(queue, img, channels, wdth, hght);
    throw std::runtime_error("Error in writeSyclBindlessTestImg: "
                             "Unsupported channel format.");
    return sycl::event();
}
//----------------------------------------------------------------------------//
