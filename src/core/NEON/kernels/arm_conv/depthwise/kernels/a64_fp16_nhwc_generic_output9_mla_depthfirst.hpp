/*
 * Copyright (c) 2021-2023 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "utils.hpp"

#include <cstdint>

#pragma once

#if defined(__aarch64__) && defined(__ARM_FP16_ARGS) && defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)

namespace arm_conv {
namespace depthwise {

void a64_fp16_nhwc_generic_output9_mla_depthfirst_impl(const __fp16 *const *const, __fp16 *const *const, const void *, const void *, const unsigned int, const unsigned int, const __fp16, const __fp16);

class a64_fp16_nhwc_generic_output9_mla_depthfirst : public GenericDepthfirstKernelStrategy<__fp16, __fp16, __fp16, __fp16>
{
  KernelType kernel = a64_fp16_nhwc_generic_output9_mla_depthfirst_impl;

  public:
  a64_fp16_nhwc_generic_output9_mla_depthfirst(const CPUInfo *) : GenericDepthfirstKernelStrategy<__fp16, __fp16, __fp16, __fp16>(9, arm_gemm::VLType::None) {}

  KernelType get_kernel() const override { return kernel; }
};

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(__aarch64__) && defined(__ARM_FP16_ARGS) && defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
