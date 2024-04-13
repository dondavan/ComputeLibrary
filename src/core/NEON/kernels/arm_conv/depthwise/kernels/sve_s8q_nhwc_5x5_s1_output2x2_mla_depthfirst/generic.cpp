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

#include "arm_gemm.hpp"

#include <cstddef>
#include <cstdint>

#if defined(ARM_COMPUTE_ENABLE_SVE)

namespace arm_conv {
namespace depthwise {

void sve_s8q_nhwc_5x5_s1_output2x2_mla_depthfirst_impl(
  const unsigned int n_channels,
  const int8_t *const *const inptrs,
  const int8_t *const weights,
  const int32_t *const bias,
  const arm_gemm::Requantize32 &qp,
  const int32_t *const requant_muls,
  const int32_t *const requant_shifts,
  int8_t *const *const outptrs
)
{
  struct Params
  {
    long unsigned int n_channels;
    const void *weights;
    const int32_t *bias;
    const arm_gemm::Requantize32 *requant;
    const int32_t *const requant_muls;
    const int32_t *const requant_shifts;
    int8_t *const *const outptrs;
    const int8_t *inptrs[36];

    Params(
      long unsigned int n_channels,
      const int8_t *const *inptrs_raw,
      const void *const weights,
      const int32_t *const bias,
      const arm_gemm::Requantize32 &qp,
      const int32_t *const requant_muls,
      const int32_t *const requant_shifts,
      int8_t *const *outptrs
    ) : n_channels(n_channels), weights(weights), bias(bias),
        requant(&qp), requant_muls(requant_muls),
        requant_shifts(requant_shifts), outptrs(outptrs)
    {
      inptrs[0] = inptrs_raw[0];
      inptrs[1] = inptrs_raw[1];
      inptrs[2] = inptrs_raw[6];
      inptrs[3] = inptrs_raw[7];
      inptrs[4] = inptrs_raw[2];
      inptrs[5] = inptrs_raw[8];
      inptrs[6] = inptrs_raw[3];
      inptrs[7] = inptrs_raw[4];
      inptrs[8] = inptrs_raw[11];
      inptrs[9] = inptrs_raw[12];
      inptrs[10] = inptrs_raw[9];
      inptrs[11] = inptrs_raw[10];
      inptrs[12] = inptrs_raw[5];
      inptrs[13] = inptrs_raw[13];
      inptrs[14] = inptrs_raw[14];
      inptrs[15] = inptrs_raw[15];
      inptrs[16] = inptrs_raw[16];
      inptrs[17] = inptrs_raw[17];
      inptrs[18] = inptrs_raw[18];
      inptrs[19] = inptrs_raw[19];
      inptrs[20] = inptrs_raw[20];
      inptrs[21] = inptrs_raw[21];
      inptrs[22] = inptrs_raw[22];
      inptrs[23] = inptrs_raw[23];
      inptrs[24] = inptrs_raw[24];
      inptrs[25] = inptrs_raw[25];
      inptrs[26] = inptrs_raw[26];
      inptrs[27] = inptrs_raw[27];
      inptrs[28] = inptrs_raw[28];
      inptrs[29] = inptrs_raw[29];
      inptrs[30] = inptrs_raw[30];
      inptrs[31] = inptrs_raw[31];
      inptrs[32] = inptrs_raw[32];
      inptrs[33] = inptrs_raw[33];
      inptrs[34] = inptrs_raw[34];
      inptrs[35] = inptrs_raw[35];

    }
  };

  const Params params(n_channels, inptrs, weights, bias, qp,
                      requant_muls, requant_shifts, outptrs);

  __asm__ __volatile__(
    "mov x2, #0x0\n"
    "mov x24, x2\n"
    "ldr x23, [%x[params], %[offsetof_Params_requant]]\n"
    "ldr x3, [%x[params], %[offsetof_Params_n_channels]]\n"
    "ptrue p4.b\n"
    "ldr x22, [%x[params], %[offsetof_Params_outptrs]]\n"
    "incw x24\n"
    "ldr x4, [%x[params], %[offsetof_Params_weights]]\n"
    "add x21, x23, %[offsetof_Requantize32_a_offset]\n"
    "add x20, x23, %[offsetof_Requantize32_b_offset]\n"
    "ld1rb { z30.b }, p4/Z, [x21]\n"
    "ld1rb { z10.b }, p4/Z, [x20]\n"
    "add x21, x23, %[offsetof_Requantize32_c_offset]\n"
    "add x20, x23, %[offsetof_Requantize32_minval]\n"
    "ld1rh { z15.h }, p4/Z, [x21]\n"
    "ld1rh { z12.h }, p4/Z, [x20]\n"
    "add x20, x23, %[offsetof_Requantize32_maxval]\n"
    "ld1rh { z13.h }, p4/Z, [x20]\n"
    "ldp x5, x6, [x22, #0x0]\n"
    "whilelt p3.h, x2, x3\n"
    "ldp x7, x8, [x22, #0x10]\n"
    "whilelt p2.s, x2, x3\n"
    "whilelt p1.s, x24, x3\n"
    "ldr x10, [%x[params], %[offsetof_Params_bias]]\n"
    "add x17, %x[params], %[offsetof_Params_inptrs]\n"
    "ld1w { z17.s }, p2/Z, [x10]\n"
    "ld1w { z16.s }, p1/Z, [x10, #1, MUL VL]\n"
    "uzp1 z14.s, z17.s, z16.s\n"
    "ld1sb { z26.h }, p4/Z, [x4]\n"
    "ld1sb { z8.h }, p4/Z, [x4, #1, MUL VL]\n"
    "uzp2 z23.s, z17.s, z16.s\n"
    "addvl x10, x10, #2\n"
    "ld1sb { z16.h }, p4/Z, [x4, #2, MUL VL]\n"
    "ld1sb { z21.h }, p4/Z, [x4, #3, MUL VL]\n"
    "mov x16, #0x0\n"
    "mov z6.d, z14.d\n"
    "ld1sb { z17.h }, p4/Z, [x4, #4, MUL VL]\n"
    "ldp x9, x28, [x17, #0x0]\n"
    "mov z18.d, z23.d\n"
    "mov z9.d, z14.d\n"
    "ldp x27, x26, [x17, #0x10]\n"
    "ldp x25, x24, [x17, #0x20]\n"
    "mov z20.d, z23.d\n"
    "mov z7.d, z14.d\n"
    "ldp x23, x22, [x17, #0x30]\n"
    "ldp x21, x20, [x17, #0x40]\n"
    "mov z1.d, z23.d\n"
    ".inst 0x454a135a  // ssublb z26.h, z26.b, z10.b\n"
    "ld1sb { z22.h }, p3/Z, [x9, x2]\n"
    "ld1sb { z2.h }, p3/Z, [x28, x2]\n"
    ".inst 0x454a1108  // ssublb z8.h, z8.b, z10.b\n"
    ".inst 0x454a1210  // ssublb z16.h, z16.b, z10.b\n"
    "ld1sb { z11.h }, p3/Z, [x27, x2]\n"
    "ld1sb { z3.h }, p3/Z, [x26, x2]\n"
    ".inst 0x454a12b5  // ssublb z21.h, z21.b, z10.b\n"
    ".inst 0x454a1231  // ssublb z17.h, z17.b, z10.b\n"
    "ld1sb { z29.h }, p3/Z, [x25, x2]\n"
    "ld1sb { z4.h }, p3/Z, [x24, x2]\n"
    ".inst 0x455e12d6  // ssublb z22.h, z22.b, z30.b\n"
    ".inst 0x455e1042  // ssublb z2.h, z2.b, z30.b\n"
    "ld1sb { z31.h }, p3/Z, [x23, x2]\n"
    "ld1sb { z0.h }, p3/Z, [x22, x2]\n"
    ".inst 0x455e116b  // ssublb z11.h, z11.b, z30.b\n"
    ".inst 0x455e1063  // ssublb z3.h, z3.b, z30.b\n"
    "ld1sb { z19.h }, p3/Z, [x21, x2]\n"
    "ld1sb { z28.h }, p3/Z, [x20, x2]\n"
    ".inst 0x455e13bd  // ssublb z29.h, z29.b, z30.b\n"
    ".inst 0x455e1084  // ssublb z4.h, z4.b, z30.b\n"
    "ldr x15, [%x[params], %[offsetof_Params_requant_muls]]\n"
    "ldr x14, [%x[params], %[offsetof_Params_requant_shifts]]\n"
    "str x10, [%x[params], %[offsetof_Params_bias]]\n"
    ".inst 0x455e13ff  // ssublb z31.h, z31.b, z30.b\n"
    ".inst 0x455e1000  // ssublb z0.h, z0.b, z30.b\n"
    ".inst 0x455e1273  // ssublb z19.h, z19.b, z30.b\n"
    ".inst 0x455e139c  // ssublb z28.h, z28.b, z30.b\n"
    "1:"  // Loop
    ".inst 0x449a42ce  // smlalb z14.s, p4/M, z22.h, z26.h\n"
    ".inst 0x449a46d7  // smlalt z23.s, p4/M, z22.h, z26.h\n"
    "ldr x20, [x17, #0x50]\n"
    "ld1sb { z27.h }, p3/Z, [x20, x2]\n"
    ".inst 0x4488404e  // smlalb z14.s, p4/M, z2.h, z8.h\n"
    ".inst 0x449a4046  // smlalb z6.s, p4/M, z2.h, z26.h\n"
    "ldr x20, [x17, #0x58]\n"
    ".inst 0x455e137b  // ssublb z27.h, z27.b, z30.b\n"
    ".inst 0x449a4169  // smlalb z9.s, p4/M, z11.h, z26.h\n"
    ".inst 0x449a4067  // smlalb z7.s, p4/M, z3.h, z26.h\n"
    "ld1sb { z5.h }, p3/Z, [x20, x2]\n"
    "ldr x20, [x17, #0x60]\n"
    ".inst 0x44884457  // smlalt z23.s, p4/M, z2.h, z8.h\n"
    ".inst 0x449043ae  // smlalb z14.s, p4/M, z29.h, z16.h\n"
    "ld1sb { z25.h }, p4/Z, [x4, #5, MUL VL]\n"
    ".inst 0x455e10a5  // ssublb z5.h, z5.b, z30.b\n"
    ".inst 0x449a4452  // smlalt z18.s, p4/M, z2.h, z26.h\n"
    ".inst 0x449a4574  // smlalt z20.s, p4/M, z11.h, z26.h\n"
    "ld1sb { z22.h }, p3/Z, [x20, x2]\n"
    ".inst 0x454a1339  // ssublb z25.h, z25.b, z10.b\n"
    ".inst 0x449a4461  // smlalt z1.s, p4/M, z3.h, z26.h\n"
    ".inst 0x448843a6  // smlalb z6.s, p4/M, z29.h, z8.h\n"
    "ldr x20, [x17, #0x68]\n"
    "ld1sb { z2.h }, p4/Z, [x4, #6, MUL VL]\n"
    ".inst 0x44884069  // smlalb z9.s, p4/M, z3.h, z8.h\n"
    ".inst 0x44884087  // smlalb z7.s, p4/M, z4.h, z8.h\n"
    ".inst 0x455e12d6  // ssublb z22.h, z22.b, z30.b\n"
    "ld1sb { z26.h }, p3/Z, [x20, x2]\n"
    ".inst 0x449047b7  // smlalt z23.s, p4/M, z29.h, z16.h\n"
    ".inst 0x449543ee  // smlalb z14.s, p4/M, z31.h, z21.h\n"
    ".inst 0x454a1042  // ssublb z2.h, z2.b, z10.b\n"
    "ldr x20, [x17, #0x70]\n"
    ".inst 0x448847b2  // smlalt z18.s, p4/M, z29.h, z8.h\n"
    ".inst 0x44884474  // smlalt z20.s, p4/M, z3.h, z8.h\n"
    "ld1sb { z29.h }, p4/Z, [x4, #7, MUL VL]\n"
    ".inst 0x455e135a  // ssublb z26.h, z26.b, z30.b\n"
    ".inst 0x44884481  // smlalt z1.s, p4/M, z4.h, z8.h\n"
    ".inst 0x449043e6  // smlalb z6.s, p4/M, z31.h, z16.h\n"
    "inch x4, ALL, MUL #8\n"
    "ld1sb { z8.h }, p3/Z, [x20, x2]\n"
    ".inst 0x44904089  // smlalb z9.s, p4/M, z4.h, z16.h\n"
    ".inst 0x44904367  // smlalb z7.s, p4/M, z27.h, z16.h\n"
    ".inst 0x454a13bd  // ssublb z29.h, z29.b, z10.b\n"
    "ldr x20, [x17, #0x78]\n"
    ".inst 0x449547f7  // smlalt z23.s, p4/M, z31.h, z21.h\n"
    ".inst 0x4491400e  // smlalb z14.s, p4/M, z0.h, z17.h\n"
    "ld1sb { z24.h }, p4/Z, [x4]\n"
    ".inst 0x455e1108  // ssublb z8.h, z8.b, z30.b\n"
    ".inst 0x449047f2  // smlalt z18.s, p4/M, z31.h, z16.h\n"
    ".inst 0x44904494  // smlalt z20.s, p4/M, z4.h, z16.h\n"
    "ld1sb { z31.h }, p3/Z, [x20, x2]\n"
    ".inst 0x454a1318  // ssublb z24.h, z24.b, z10.b\n"
    ".inst 0x44904761  // smlalt z1.s, p4/M, z27.h, z16.h\n"
    ".inst 0x44954006  // smlalb z6.s, p4/M, z0.h, z21.h\n"
    "ldr x22, [x17, #0x80]\n"
    "ld1sb { z16.h }, p4/Z, [x4, #1, MUL VL]\n"
    ".inst 0x44954369  // smlalb z9.s, p4/M, z27.h, z21.h\n"
    ".inst 0x449540a7  // smlalb z7.s, p4/M, z5.h, z21.h\n"
    ".inst 0x455e13ff  // ssublb z31.h, z31.b, z30.b\n"
    "ldr x21, [x17, #0x88]\n"
    ".inst 0x44914417  // smlalt z23.s, p4/M, z0.h, z17.h\n"
    ".inst 0x4499416e  // smlalb z14.s, p4/M, z11.h, z25.h\n"
    ".inst 0x454a1210  // ssublb z16.h, z16.b, z10.b\n"
    "ldr x20, [x17, #0x90]\n"
    ".inst 0x44954412  // smlalt z18.s, p4/M, z0.h, z21.h\n"
    ".inst 0x44954774  // smlalt z20.s, p4/M, z27.h, z21.h\n"
    "ld1sb { z0.h }, p3/Z, [x22, x2]\n"
    ".inst 0x455e1000  // ssublb z0.h, z0.b, z30.b\n"
    ".inst 0x449544a1  // smlalt z1.s, p4/M, z5.h, z21.h\n"
    ".inst 0x449142c6  // smlalb z6.s, p4/M, z22.h, z17.h\n"
    "ld1sb { z21.h }, p4/Z, [x4, #2, MUL VL]\n"
    ".inst 0x454a12b5  // ssublb z21.h, z21.b, z10.b\n"
    ".inst 0x449140a9  // smlalb z9.s, p4/M, z5.h, z17.h\n"
    ".inst 0x44914267  // smlalb z7.s, p4/M, z19.h, z17.h\n"
    "ldr x23, [x17, #0x98]\n"
    "ldr x22, [x17, #0xa0]\n"
    ".inst 0x44994577  // smlalt z23.s, p4/M, z11.h, z25.h\n"
    ".inst 0x4482406e  // smlalb z14.s, p4/M, z3.h, z2.h\n"
    "ld1sb { z11.h }, p3/Z, [x21, x2]\n"
    ".inst 0x455e116b  // ssublb z11.h, z11.b, z30.b\n"
    ".inst 0x449146d2  // smlalt z18.s, p4/M, z22.h, z17.h\n"
    ".inst 0x449144b4  // smlalt z20.s, p4/M, z5.h, z17.h\n"
    "ld1sb { z22.h }, p4/Z, [x4, #3, MUL VL]\n"
    ".inst 0x454a12d6  // ssublb z22.h, z22.b, z10.b\n"
    ".inst 0x44914661  // smlalt z1.s, p4/M, z19.h, z17.h\n"
    ".inst 0x44994066  // smlalb z6.s, p4/M, z3.h, z25.h\n"
    "ld1sb { z17.h }, p3/Z, [x20, x2]\n"
    ".inst 0x455e1231  // ssublb z17.h, z17.b, z30.b\n"
    ".inst 0x44994389  // smlalb z9.s, p4/M, z28.h, z25.h\n"
    ".inst 0x44994347  // smlalb z7.s, p4/M, z26.h, z25.h\n"
    "ldr x20, [x17, #0xa8]\n"
    "ldr x21, [x17, #0xb0]\n"
    ".inst 0x44824477  // smlalt z23.s, p4/M, z3.h, z2.h\n"
    ".inst 0x449d408e  // smlalb z14.s, p4/M, z4.h, z29.h\n"
    "ldr x13, [x17, #0xb8]\n"
    "ldr x12, [x17, #0xc0]\n"
    ".inst 0x44994472  // smlalt z18.s, p4/M, z3.h, z25.h\n"
    ".inst 0x44994794  // smlalt z20.s, p4/M, z28.h, z25.h\n"
    "ld1sb { z3.h }, p3/Z, [x23, x2]\n"
    ".inst 0x455e1063  // ssublb z3.h, z3.b, z30.b\n"
    ".inst 0x44994741  // smlalt z1.s, p4/M, z26.h, z25.h\n"
    ".inst 0x44824086  // smlalb z6.s, p4/M, z4.h, z2.h\n"
    "ld1sb { z25.h }, p4/Z, [x4, #4, MUL VL]\n"
    ".inst 0x454a1339  // ssublb z25.h, z25.b, z10.b\n"
    ".inst 0x44824349  // smlalb z9.s, p4/M, z26.h, z2.h\n"
    ".inst 0x44824107  // smlalb z7.s, p4/M, z8.h, z2.h\n"
    "ldr x11, [x17, #0xc8]\n"
    "ldr x10, [x17, #0xd0]\n"
    ".inst 0x449d4497  // smlalt z23.s, p4/M, z4.h, z29.h\n"
    ".inst 0x4498436e  // smlalb z14.s, p4/M, z27.h, z24.h\n"
    "ldr x9, [x17, #0xd8]\n"
    "ldr x28, [x17, #0xe0]\n"
    ".inst 0x44824492  // smlalt z18.s, p4/M, z4.h, z2.h\n"
    ".inst 0x44824754  // smlalt z20.s, p4/M, z26.h, z2.h\n"
    "ld1sb { z4.h }, p3/Z, [x22, x2]\n"
    ".inst 0x455e1084  // ssublb z4.h, z4.b, z30.b\n"
    ".inst 0x44824501  // smlalt z1.s, p4/M, z8.h, z2.h\n"
    ".inst 0x449d4366  // smlalb z6.s, p4/M, z27.h, z29.h\n"
    "ld1sb { z2.h }, p4/Z, [x4, #5, MUL VL]\n"
    ".inst 0x454a1042  // ssublb z2.h, z2.b, z10.b\n"
    ".inst 0x449d4109  // smlalb z9.s, p4/M, z8.h, z29.h\n"
    ".inst 0x449d43e7  // smlalb z7.s, p4/M, z31.h, z29.h\n"
    "ldr x27, [x17, #0xe8]\n"
    "ldr x26, [x17, #0xf0]\n"
    ".inst 0x44984777  // smlalt z23.s, p4/M, z27.h, z24.h\n"
    ".inst 0x449040ae  // smlalb z14.s, p4/M, z5.h, z16.h\n"
    "ldr x25, [x17, #0xf8]\n"
    "ldr x24, [x17, #0x100]\n"
    ".inst 0x449d4772  // smlalt z18.s, p4/M, z27.h, z29.h\n"
    ".inst 0x449d4514  // smlalt z20.s, p4/M, z8.h, z29.h\n"
    "ld1sb { z27.h }, p3/Z, [x20, x2]\n"
    ".inst 0x455e137b  // ssublb z27.h, z27.b, z30.b\n"
    ".inst 0x449d47e1  // smlalt z1.s, p4/M, z31.h, z29.h\n"
    ".inst 0x449840a6  // smlalb z6.s, p4/M, z5.h, z24.h\n"
    "ld1sb { z29.h }, p4/Z, [x4, #6, MUL VL]\n"
    ".inst 0x454a13bd  // ssublb z29.h, z29.b, z10.b\n"
    ".inst 0x449843e9  // smlalb z9.s, p4/M, z31.h, z24.h\n"
    ".inst 0x44984007  // smlalb z7.s, p4/M, z0.h, z24.h\n"
    "ldr x23, [x17, #0x108]\n"
    "ldr x22, [x17, #0x110]\n"
    ".inst 0x449044b7  // smlalt z23.s, p4/M, z5.h, z16.h\n"
    ".inst 0x4495438e  // smlalb z14.s, p4/M, z28.h, z21.h\n"
    "ldr x20, [x17, #0x118]\n"
    "whilelt p0.h, x16, x3\n"
    ".inst 0x449844b2  // smlalt z18.s, p4/M, z5.h, z24.h\n"
    ".inst 0x449847f4  // smlalt z20.s, p4/M, z31.h, z24.h\n"
    "ld1sb { z5.h }, p3/Z, [x21, x2]\n"
    ".inst 0x455e10a5  // ssublb z5.h, z5.b, z30.b\n"
    ".inst 0x44984401  // smlalt z1.s, p4/M, z0.h, z24.h\n"
    ".inst 0x44904266  // smlalb z6.s, p4/M, z19.h, z16.h\n"
    "ld1sb { z24.h }, p4/Z, [x4, #7, MUL VL]\n"
    "inch x4, ALL, MUL #8\n"
    ".inst 0x44904009  // smlalb z9.s, p4/M, z0.h, z16.h\n"
    ".inst 0x44904167  // smlalb z7.s, p4/M, z11.h, z16.h\n"
    ".inst 0x454a1318  // ssublb z24.h, z24.b, z10.b\n"
    "ldr x21, [%x[params], %[offsetof_Params_bias]]\n"
    ".inst 0x44954797  // smlalt z23.s, p4/M, z28.h, z21.h\n"
    ".inst 0x4496434e  // smlalb z14.s, p4/M, z26.h, z22.h\n"
    "ld1sb { z28.h }, p3/Z, [x13, x2]\n"
    ".inst 0x455e139c  // ssublb z28.h, z28.b, z30.b\n"
    ".inst 0x44904672  // smlalt z18.s, p4/M, z19.h, z16.h\n"
    ".inst 0x44904414  // smlalt z20.s, p4/M, z0.h, z16.h\n"
    "ld1sb { z19.h }, p4/Z, [x4]\n"
    ".inst 0x454a1273  // ssublb z19.h, z19.b, z10.b\n"
    ".inst 0x44904561  // smlalt z1.s, p4/M, z11.h, z16.h\n"
    ".inst 0x44954346  // smlalb z6.s, p4/M, z26.h, z21.h\n"
    "ld1sb { z16.h }, p3/Z, [x12, x2]\n"
    ".inst 0x455e1210  // ssublb z16.h, z16.b, z30.b\n"
    ".inst 0x44954229  // smlalb z9.s, p4/M, z17.h, z21.h\n"
    ".inst 0x44954067  // smlalb z7.s, p4/M, z3.h, z21.h\n"
    ".inst 0x44964757  // smlalt z23.s, p4/M, z26.h, z22.h\n"
    ".inst 0x4499410e  // smlalb z14.s, p4/M, z8.h, z25.h\n"
    ".inst 0x44954752  // smlalt z18.s, p4/M, z26.h, z21.h\n"
    ".inst 0x44954634  // smlalt z20.s, p4/M, z17.h, z21.h\n"
    "ld1sb { z26.h }, p3/Z, [x11, x2]\n"
    ".inst 0x455e135a  // ssublb z26.h, z26.b, z30.b\n"
    ".inst 0x44954461  // smlalt z1.s, p4/M, z3.h, z21.h\n"
    ".inst 0x44964106  // smlalb z6.s, p4/M, z8.h, z22.h\n"
    "ld1sb { z21.h }, p4/Z, [x4, #1, MUL VL]\n"
    ".inst 0x454a12b5  // ssublb z21.h, z21.b, z10.b\n"
    ".inst 0x44964069  // smlalb z9.s, p4/M, z3.h, z22.h\n"
    ".inst 0x44964087  // smlalb z7.s, p4/M, z4.h, z22.h\n"
    ".inst 0x44994517  // smlalt z23.s, p4/M, z8.h, z25.h\n"
    ".inst 0x448243ee  // smlalb z14.s, p4/M, z31.h, z2.h\n"
    ".inst 0x44964512  // smlalt z18.s, p4/M, z8.h, z22.h\n"
    ".inst 0x44964474  // smlalt z20.s, p4/M, z3.h, z22.h\n"
    "ld1sb { z8.h }, p3/Z, [x10, x2]\n"
    ".inst 0x455e1108  // ssublb z8.h, z8.b, z30.b\n"
    ".inst 0x44964481  // smlalt z1.s, p4/M, z4.h, z22.h\n"
    ".inst 0x449943e6  // smlalb z6.s, p4/M, z31.h, z25.h\n"
    "ld1sb { z22.h }, p4/Z, [x4, #2, MUL VL]\n"
    ".inst 0x454a12d6  // ssublb z22.h, z22.b, z10.b\n"
    ".inst 0x44994089  // smlalb z9.s, p4/M, z4.h, z25.h\n"
    ".inst 0x44994367  // smlalb z7.s, p4/M, z27.h, z25.h\n"
    ".inst 0x448247f7  // smlalt z23.s, p4/M, z31.h, z2.h\n"
    ".inst 0x449d400e  // smlalb z14.s, p4/M, z0.h, z29.h\n"
    ".inst 0x449947f2  // smlalt z18.s, p4/M, z31.h, z25.h\n"
    ".inst 0x44994494  // smlalt z20.s, p4/M, z4.h, z25.h\n"
    "ld1sb { z31.h }, p3/Z, [x9, x2]\n"
    ".inst 0x455e13ff  // ssublb z31.h, z31.b, z30.b\n"
    ".inst 0x44994761  // smlalt z1.s, p4/M, z27.h, z25.h\n"
    ".inst 0x44824006  // smlalb z6.s, p4/M, z0.h, z2.h\n"
    "ld1sb { z25.h }, p4/Z, [x4, #3, MUL VL]\n"
    ".inst 0x454a1339  // ssublb z25.h, z25.b, z10.b\n"
    ".inst 0x44824369  // smlalb z9.s, p4/M, z27.h, z2.h\n"
    ".inst 0x448240a7  // smlalb z7.s, p4/M, z5.h, z2.h\n"
    ".inst 0x449d4417  // smlalt z23.s, p4/M, z0.h, z29.h\n"
    ".inst 0x4498422e  // smlalb z14.s, p4/M, z17.h, z24.h\n"
    ".inst 0x44824412  // smlalt z18.s, p4/M, z0.h, z2.h\n"
    ".inst 0x44824774  // smlalt z20.s, p4/M, z27.h, z2.h\n"
    "ld1sb { z0.h }, p3/Z, [x28, x2]\n"
    ".inst 0x455e1000  // ssublb z0.h, z0.b, z30.b\n"
    ".inst 0x448244a1  // smlalt z1.s, p4/M, z5.h, z2.h\n"
    ".inst 0x449d4166  // smlalb z6.s, p4/M, z11.h, z29.h\n"
    "ld1sb { z2.h }, p4/Z, [x4, #4, MUL VL]\n"
    ".inst 0x454a1042  // ssublb z2.h, z2.b, z10.b\n"
    ".inst 0x449d40a9  // smlalb z9.s, p4/M, z5.h, z29.h\n"
    ".inst 0x449d4387  // smlalb z7.s, p4/M, z28.h, z29.h\n"
    ".inst 0x44984637  // smlalt z23.s, p4/M, z17.h, z24.h\n"
    ".inst 0x4493406e  // smlalb z14.s, p4/M, z3.h, z19.h\n"
    "ld1sb { z17.h }, p3/Z, [x27, x2]\n"
    ".inst 0x455e1231  // ssublb z17.h, z17.b, z30.b\n"
    ".inst 0x449d4572  // smlalt z18.s, p4/M, z11.h, z29.h\n"
    ".inst 0x449d44b4  // smlalt z20.s, p4/M, z5.h, z29.h\n"
    "ld1sb { z11.h }, p4/Z, [x4, #5, MUL VL]\n"
    ".inst 0x454a116b  // ssublb z11.h, z11.b, z10.b\n"
    ".inst 0x449d4781  // smlalt z1.s, p4/M, z28.h, z29.h\n"
    ".inst 0x44984066  // smlalb z6.s, p4/M, z3.h, z24.h\n"
    "ld1sb { z29.h }, p3/Z, [x26, x2]\n"
    ".inst 0x455e13bd  // ssublb z29.h, z29.b, z30.b\n"
    ".inst 0x44984209  // smlalb z9.s, p4/M, z16.h, z24.h\n"
    ".inst 0x44984347  // smlalb z7.s, p4/M, z26.h, z24.h\n"
    ".inst 0x44934477  // smlalt z23.s, p4/M, z3.h, z19.h\n"
    ".inst 0x4495408e  // smlalb z14.s, p4/M, z4.h, z21.h\n"
    ".inst 0x44984472  // smlalt z18.s, p4/M, z3.h, z24.h\n"
    ".inst 0x44984614  // smlalt z20.s, p4/M, z16.h, z24.h\n"
    "ld1sb { z3.h }, p3/Z, [x25, x2]\n"
    ".inst 0x455e1063  // ssublb z3.h, z3.b, z30.b\n"
    ".inst 0x44984741  // smlalt z1.s, p4/M, z26.h, z24.h\n"
    ".inst 0x44934086  // smlalb z6.s, p4/M, z4.h, z19.h\n"
    "ld1sb { z24.h }, p4/Z, [x4, #6, MUL VL]\n"
    ".inst 0x454a1318  // ssublb z24.h, z24.b, z10.b\n"
    ".inst 0x44934349  // smlalb z9.s, p4/M, z26.h, z19.h\n"
    ".inst 0x44934107  // smlalb z7.s, p4/M, z8.h, z19.h\n"
    ".inst 0x44954497  // smlalt z23.s, p4/M, z4.h, z21.h\n"
    ".inst 0x4496436e  // smlalb z14.s, p4/M, z27.h, z22.h\n"
    ".inst 0x44934492  // smlalt z18.s, p4/M, z4.h, z19.h\n"
    ".inst 0x44934754  // smlalt z20.s, p4/M, z26.h, z19.h\n"
    "ld1sb { z4.h }, p3/Z, [x24, x2]\n"
    ".inst 0x455e1084  // ssublb z4.h, z4.b, z30.b\n"
    ".inst 0x44934501  // smlalt z1.s, p4/M, z8.h, z19.h\n"
    ".inst 0x44954366  // smlalb z6.s, p4/M, z27.h, z21.h\n"
    "ld1sb { z19.h }, p4/Z, [x4, #7, MUL VL]\n"
    "inch x4, ALL, MUL #8\n"
    ".inst 0x44954109  // smlalb z9.s, p4/M, z8.h, z21.h\n"
    ".inst 0x449543e7  // smlalb z7.s, p4/M, z31.h, z21.h\n"
    ".inst 0x454a1273  // ssublb z19.h, z19.b, z10.b\n"
    ".inst 0x44964777  // smlalt z23.s, p4/M, z27.h, z22.h\n"
    ".inst 0x449940ae  // smlalb z14.s, p4/M, z5.h, z25.h\n"
    ".inst 0x44954772  // smlalt z18.s, p4/M, z27.h, z21.h\n"
    ".inst 0x44954514  // smlalt z20.s, p4/M, z8.h, z21.h\n"
    "ld1sb { z27.h }, p3/Z, [x23, x2]\n"
    ".inst 0x455e137b  // ssublb z27.h, z27.b, z30.b\n"
    ".inst 0x449547e1  // smlalt z1.s, p4/M, z31.h, z21.h\n"
    ".inst 0x449640a6  // smlalb z6.s, p4/M, z5.h, z22.h\n"
    "ld1sb { z21.h }, p4/Z, [x4]\n"
    ".inst 0x454a12b5  // ssublb z21.h, z21.b, z10.b\n"
    ".inst 0x449643e9  // smlalb z9.s, p4/M, z31.h, z22.h\n"
    ".inst 0x44964007  // smlalb z7.s, p4/M, z0.h, z22.h\n"
    "inch x4\n"
    ".inst 0x449944b7  // smlalt z23.s, p4/M, z5.h, z25.h\n"
    ".inst 0x4482420e  // smlalb z14.s, p4/M, z16.h, z2.h\n"
    ".inst 0x449644b2  // smlalt z18.s, p4/M, z5.h, z22.h\n"
    ".inst 0x449647f4  // smlalt z20.s, p4/M, z31.h, z22.h\n"
    "ld1sb { z5.h }, p3/Z, [x22, x2]\n"
    ".inst 0x455e10a5  // ssublb z5.h, z5.b, z30.b\n"
    ".inst 0x44964401  // smlalt z1.s, p4/M, z0.h, z22.h\n"
    ".inst 0x44994386  // smlalb z6.s, p4/M, z28.h, z25.h\n"
    "ld1w { z22.s }, p2/Z, [x15]\n"
    ".inst 0x44994009  // smlalb z9.s, p4/M, z0.h, z25.h\n"
    ".inst 0x44994227  // smlalb z7.s, p4/M, z17.h, z25.h\n"
    ".inst 0x44824617  // smlalt z23.s, p4/M, z16.h, z2.h\n"
    ".inst 0x448b434e  // smlalb z14.s, p4/M, z26.h, z11.h\n"
    "ld1w { z16.s }, p1/Z, [x15, #1, MUL VL]\n"
    "addvl x15, x15, #2\n"
    ".inst 0x44994792  // smlalt z18.s, p4/M, z28.h, z25.h\n"
    ".inst 0x44994414  // smlalt z20.s, p4/M, z0.h, z25.h\n"
    "ld1sb { z28.h }, p3/Z, [x20, x2]\n"
    ".inst 0x455e139c  // ssublb z28.h, z28.b, z30.b\n"
    ".inst 0x44994621  // smlalt z1.s, p4/M, z17.h, z25.h\n"
    ".inst 0x44824346  // smlalb z6.s, p4/M, z26.h, z2.h\n"
    "uzp1 z25.s, z22.s, z16.s\n"
    "inch x2\n"
    ".inst 0x448243a9  // smlalb z9.s, p4/M, z29.h, z2.h\n"
    ".inst 0x44824067  // smlalb z7.s, p4/M, z3.h, z2.h\n"
    "uzp2 z16.s, z22.s, z16.s\n"
    "ld1w { z22.s }, p2/Z, [x14]\n"
    ".inst 0x448b4757  // smlalt z23.s, p4/M, z26.h, z11.h\n"
    ".inst 0x4498410e  // smlalb z14.s, p4/M, z8.h, z24.h\n"
    "mov x20, x2\n"
    "incw x20\n"
    ".inst 0x44824752  // smlalt z18.s, p4/M, z26.h, z2.h\n"
    ".inst 0x448247b4  // smlalt z20.s, p4/M, z29.h, z2.h\n"
    "ld1w { z26.s }, p1/Z, [x14, #1, MUL VL]\n"
    "uzp1 z29.s, z22.s, z26.s\n"
    ".inst 0x44824461  // smlalt z1.s, p4/M, z3.h, z2.h\n"
    ".inst 0x448b4106  // smlalb z6.s, p4/M, z8.h, z11.h\n"
    "uzp2 z22.s, z22.s, z26.s\n"
    "whilelt p2.s, x2, x3\n"
    ".inst 0x448b4069  // smlalb z9.s, p4/M, z3.h, z11.h\n"
    ".inst 0x448b4087  // smlalb z7.s, p4/M, z4.h, z11.h\n"
    "whilelt p1.s, x20, x3\n"
    "whilelt p3.h, x2, x3\n"
    ".inst 0x44984517  // smlalt z23.s, p4/M, z8.h, z24.h\n"
    ".inst 0x449343ee  // smlalb z14.s, p4/M, z31.h, z19.h\n"
    "addvl x14, x14, #2\n"
    ".inst 0x448b4512  // smlalt z18.s, p4/M, z8.h, z11.h\n"
    ".inst 0x448b4474  // smlalt z20.s, p4/M, z3.h, z11.h\n"
    ".inst 0x448b4481  // smlalt z1.s, p4/M, z4.h, z11.h\n"
    ".inst 0x449843e6  // smlalb z6.s, p4/M, z31.h, z24.h\n"
    ".inst 0x44984089  // smlalb z9.s, p4/M, z4.h, z24.h\n"
    ".inst 0x44984367  // smlalb z7.s, p4/M, z27.h, z24.h\n"
    ".inst 0x449347f7  // smlalt z23.s, p4/M, z31.h, z19.h\n"
    ".inst 0x4495400e  // smlalb z14.s, p4/M, z0.h, z21.h\n"
    ".inst 0x04b975ce  // sqrdmulh z14.s, z14.s, z25.s\n"
    ".inst 0x449847f2  // smlalt z18.s, p4/M, z31.h, z24.h\n"
    ".inst 0x44984494  // smlalt z20.s, p4/M, z4.h, z24.h\n"
    "and z3.d, z14.d, z29.d\n"
    ".inst 0x44984761  // smlalt z1.s, p4/M, z27.h, z24.h\n"
    ".inst 0x44934006  // smlalb z6.s, p4/M, z0.h, z19.h\n"
    "asr z3.s, z3.s, #0x1f\n"
    ".inst 0x44934369  // smlalb z9.s, p4/M, z27.h, z19.h\n"
    ".inst 0x449340a7  // smlalb z7.s, p4/M, z5.h, z19.h\n"
    "sqadd z14.s, z14.s, z3.s\n"
    ".inst 0x448293ae  // srshl z14.s, p4/M, z14.s, z29.s\n"
    ".inst 0x44954417  // smlalt z23.s, p4/M, z0.h, z21.h\n"
    ".inst 0x44934412  // smlalt z18.s, p4/M, z0.h, z19.h\n"
    ".inst 0x04b076f7  // sqrdmulh z23.s, z23.s, z16.s\n"
    ".inst 0x44934774  // smlalt z20.s, p4/M, z27.h, z19.h\n"
    ".inst 0x449344a1  // smlalt z1.s, p4/M, z5.h, z19.h\n"
    "and z31.d, z23.d, z22.d\n"
    ".inst 0x44954226  // smlalb z6.s, p4/M, z17.h, z21.h\n"
    ".inst 0x449540a9  // smlalb z9.s, p4/M, z5.h, z21.h\n"
    ".inst 0x04b974c6  // sqrdmulh z6.s, z6.s, z25.s\n"
    ".inst 0x44954387  // smlalb z7.s, p4/M, z28.h, z21.h\n"
    ".inst 0x44954632  // smlalt z18.s, p4/M, z17.h, z21.h\n"
    ".inst 0x04b97529  // sqrdmulh z9.s, z9.s, z25.s\n"
    ".inst 0x449544b4  // smlalt z20.s, p4/M, z5.h, z21.h\n"
    ".inst 0x44954781  // smlalt z1.s, p4/M, z28.h, z21.h\n"
    ".inst 0x04b974e7  // sqrdmulh z7.s, z7.s, z25.s\n"
    "asr z31.s, z31.s, #0x1f\n"
    "and z3.d, z6.d, z29.d\n"
    ".inst 0x04b07652  // sqrdmulh z18.s, z18.s, z16.s\n"
    "and z0.d, z9.d, z29.d\n"
    ".inst 0x04b07694  // sqrdmulh z20.s, z20.s, z16.s\n"
    "and z19.d, z7.d, z29.d\n"
    ".inst 0x04b07421  // sqrdmulh z1.s, z1.s, z16.s\n"
    "sqadd z23.s, z23.s, z31.s\n"
    ".inst 0x448292d7  // srshl z23.s, p4/M, z23.s, z22.s\n"
    "asr z3.s, z3.s, #0x1f\n"
    "and z21.d, z18.d, z22.d\n"
    "asr z0.s, z0.s, #0x1f\n"
    "and z17.d, z20.d, z22.d\n"
    "asr z19.s, z19.s, #0x1f\n"
    "and z16.d, z1.d, z22.d\n"
    "sqadd z6.s, z6.s, z3.s\n"
    "asr z21.s, z21.s, #0x1f\n"
    ".inst 0x448293a6  // srshl z6.s, p4/M, z6.s, z29.s\n"
    "sqadd z9.s, z9.s, z0.s\n"
    "asr z17.s, z17.s, #0x1f\n"
    ".inst 0x448293a9  // srshl z9.s, p4/M, z9.s, z29.s\n"
    "sqadd z7.s, z7.s, z19.s\n"
    "asr z16.s, z16.s, #0x1f\n"
    ".inst 0x448293a7  // srshl z7.s, p4/M, z7.s, z29.s\n"
    "sqadd z18.s, z18.s, z21.s\n"
    "sqadd z20.s, z20.s, z17.s\n"
    ".inst 0x448292d2  // srshl z18.s, p4/M, z18.s, z22.s\n"
    ".inst 0x448292d4  // srshl z20.s, p4/M, z20.s, z22.s\n"
    "sqadd z1.s, z1.s, z16.s\n"
    ".inst 0x453041ce  // sqxtnb z14.h, z14.s\n"
    ".inst 0x448292c1  // srshl z1.s, p4/M, z1.s, z22.s\n"
    ".inst 0x453040c6  // sqxtnb z6.h, z6.s\n"
    ".inst 0x45304129  // sqxtnb z9.h, z9.s\n"
    ".inst 0x453040e7  // sqxtnb z7.h, z7.s\n"
    ".inst 0x453046ee  // sqxtnt z14.h, z23.s\n"
    ".inst 0x45304646  // sqxtnt z6.h, z18.s\n"
    ".inst 0x45304689  // sqxtnt z9.h, z20.s\n"
    ".inst 0x45304427  // sqxtnt z7.h, z1.s\n"
    "sqadd z14.h, z14.h, z15.h\n"
    "smax z14.h, p4/M, z14.h, z12.h\n"
    "smin z14.h, p4/M, z14.h, z13.h\n"
    "sqadd z6.h, z6.h, z15.h\n"
    "sqadd z9.h, z9.h, z15.h\n"
    "smax z6.h, p4/M, z6.h, z12.h\n"
    "smax z9.h, p4/M, z9.h, z12.h\n"
    "sqadd z7.h, z7.h, z15.h\n"
    "smax z7.h, p4/M, z7.h, z12.h\n"
    "smin z6.h, p4/M, z6.h, z13.h\n"
    "st1b { z14.h }, p0, [x5, x16]\n"
    "smin z9.h, p4/M, z9.h, z13.h\n"
    "smin z7.h, p4/M, z7.h, z13.h\n"
    "st1b { z6.h }, p0, [x6, x16]\n"
    "st1b { z9.h }, p0, [x7, x16]\n"
    "st1b { z7.h }, p0, [x8, x16]\n"
    "ld1w { z17.s }, p2/Z, [x21]\n"
    "ld1w { z16.s }, p1/Z, [x21, #1, MUL VL]\n"
    "uzp1 z14.s, z17.s, z16.s\n"
    "ld1sb { z26.h }, p4/Z, [x4]\n"
    "ld1sb { z8.h }, p4/Z, [x4, #1, MUL VL]\n"
    "uzp2 z23.s, z17.s, z16.s\n"
    "addvl x21, x21, #2\n"
    "ld1sb { z16.h }, p4/Z, [x4, #2, MUL VL]\n"
    "ld1sb { z21.h }, p4/Z, [x4, #3, MUL VL]\n"
    "inch x16\n"
    "str x21, [%x[params], %[offsetof_Params_bias]]\n"
    "ld1sb { z17.h }, p4/Z, [x4, #4, MUL VL]\n"
    "ldp x9, x28, [x17, #0x0]\n"
    "mov z6.d, z14.d\n"
    "mov z18.d, z23.d\n"
    "ldp x27, x26, [x17, #0x10]\n"
    "ldp x25, x24, [x17, #0x20]\n"
    "mov z9.d, z14.d\n"
    "mov z20.d, z23.d\n"
    "ldp x23, x22, [x17, #0x30]\n"
    "ldp x21, x20, [x17, #0x40]\n"
    "mov z7.d, z14.d\n"
    "mov z1.d, z23.d\n"
    "ld1sb { z22.h }, p3/Z, [x9, x2]\n"
    "ld1sb { z2.h }, p3/Z, [x28, x2]\n"
    ".inst 0x454a135a  // ssublb z26.h, z26.b, z10.b\n"
    ".inst 0x454a1108  // ssublb z8.h, z8.b, z10.b\n"
    "ld1sb { z11.h }, p3/Z, [x27, x2]\n"
    "ld1sb { z3.h }, p3/Z, [x26, x2]\n"
    ".inst 0x454a1210  // ssublb z16.h, z16.b, z10.b\n"
    ".inst 0x454a12b5  // ssublb z21.h, z21.b, z10.b\n"
    "ld1sb { z29.h }, p3/Z, [x25, x2]\n"
    "ld1sb { z4.h }, p3/Z, [x24, x2]\n"
    ".inst 0x454a1231  // ssublb z17.h, z17.b, z10.b\n"
    ".inst 0x455e12d6  // ssublb z22.h, z22.b, z30.b\n"
    "ld1sb { z31.h }, p3/Z, [x23, x2]\n"
    "ld1sb { z0.h }, p3/Z, [x22, x2]\n"
    ".inst 0x455e1042  // ssublb z2.h, z2.b, z30.b\n"
    ".inst 0x455e116b  // ssublb z11.h, z11.b, z30.b\n"
    "ld1sb { z19.h }, p3/Z, [x21, x2]\n"
    "ld1sb { z28.h }, p3/Z, [x20, x2]\n"
    ".inst 0x455e1063  // ssublb z3.h, z3.b, z30.b\n"
    ".inst 0x455e13bd  // ssublb z29.h, z29.b, z30.b\n"
    ".inst 0x455e1084  // ssublb z4.h, z4.b, z30.b\n"
    ".inst 0x455e13ff  // ssublb z31.h, z31.b, z30.b\n"
    ".inst 0x455e1000  // ssublb z0.h, z0.b, z30.b\n"
    ".inst 0x455e1273  // ssublb z19.h, z19.b, z30.b\n"
    ".inst 0x455e139c  // ssublb z28.h, z28.b, z30.b\n"
    "b.any 1b\n"
    :
    : [offsetof_Params_bias] "I" (offsetof(Params, bias)), [offsetof_Params_inptrs] "I" (offsetof(Params, inptrs)), [offsetof_Params_n_channels] "I" (offsetof(Params, n_channels)), [offsetof_Params_outptrs] "I" (offsetof(Params, outptrs)), [offsetof_Params_requant] "I" (offsetof(Params, requant)), [offsetof_Params_requant_muls] "I" (offsetof(Params, requant_muls)), [offsetof_Params_requant_shifts] "I" (offsetof(Params, requant_shifts)), [offsetof_Params_weights] "I" (offsetof(Params, weights)), [offsetof_Requantize32_a_offset] "I" (offsetof(arm_gemm::Requantize32, a_offset)), [offsetof_Requantize32_b_offset] "I" (offsetof(arm_gemm::Requantize32, b_offset)), [offsetof_Requantize32_c_offset] "I" (offsetof(arm_gemm::Requantize32, c_offset)), [offsetof_Requantize32_maxval] "I" (offsetof(arm_gemm::Requantize32, maxval)), [offsetof_Requantize32_minval] "I" (offsetof(arm_gemm::Requantize32, minval)), [params] "r" (&params)
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)
