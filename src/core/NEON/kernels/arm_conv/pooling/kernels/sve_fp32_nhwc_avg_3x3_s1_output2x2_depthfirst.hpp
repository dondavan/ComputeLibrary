/*
 * Copyright (c) 2021-2022 Arm Limited.
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

#pragma once

#if defined(ARM_COMPUTE_ENABLE_SVE)

namespace arm_conv {
namespace pooling {

void sve_fp32_nhwc_avg_3x3_s1_output2x2_depthfirst_impl(unsigned int, const float *const *const, float *const *const, bool, unsigned int, unsigned int, unsigned int, unsigned int);

struct sve_fp32_nhwc_avg_3x3_s1_output2x2_depthfirst : public DepthfirstStrategy<float, float>
{
  using Parent = DepthfirstStrategy<float, float>;

  const static auto pooling_type = PoolingType::AVERAGE;
  const static auto pool_rows = 3u, pool_cols = 3u;
  const static auto stride_rows = 1u, stride_cols = 1u;

  sve_fp32_nhwc_avg_3x3_s1_output2x2_depthfirst(const CPUInfo *)
  : Parent(pool_rows, pool_cols, stride_rows, stride_cols, 2, 2) {}

  Parent::KernelType get_kernel(void) const { return sve_fp32_nhwc_avg_3x3_s1_output2x2_depthfirst_impl; }
};

}  // namespace pooling
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)