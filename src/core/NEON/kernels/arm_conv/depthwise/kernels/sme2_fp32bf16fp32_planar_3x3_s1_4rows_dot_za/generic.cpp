/*
 * Copyright (c) 2022-2023 Arm Limited.
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

#if defined(ARM_COMPUTE_ENABLE_SME2)

#include <algorithm>
#include <cstddef>

namespace arm_conv {
namespace depthwise {

void sme2_fp32bf16fp32_planar_3x3_s1_4rows_dot_za_impl(
  const float *inptr,
  size_t ld_in_row,
  size_t ld_in_col,
  size_t ld_in_vl,
  unsigned int pad_top,
  unsigned int valid_input_rows,
  unsigned int pad_left,
  unsigned int valid_input_cols,
  const float *weights,
  const float *bias,
  float **outptrs,
  const size_t *outlds,
  const size_t *outvllds,
  unsigned int output_cols,
  unsigned int start_channel,
  unsigned int valid_channels,
  float act_min,
  float act_max
)
{
  struct Args
  {
    const float *inptr;
    size_t ld_in_vl;
    long unsigned int pad_top, pad_bottom, pad_left;
    const float *weights;
    const float *bias;
    long unsigned int input_cols, output_cols;
    float **outptrs;
    const size_t *ld_out_cols;
    const size_t *ld_out_vls;
    long unsigned int current_channel, n_channels;
    float clamp_min, clamp_max;
  };

  Args args = { inptr, ld_in_vl, pad_top, 6u - std::min(6u, pad_top + valid_input_rows), pad_left, weights, bias, valid_input_cols, output_cols, outptrs, outlds, outvllds, start_channel, valid_channels, act_min, act_max };

  __asm__ __volatile__(
    "ldr x7, [%x[args], %[offsetof_Args_pad_bottom]]\n"
    "mov x20, #0x6\n"
    ".inst 0xd503477f  // SMSTART ZA\n"
    "sub x20, x20, x7\n"
    "ldr x17, [%x[args], %[offsetof_Args_pad_top]]\n"
    "ptrue p2.b\n"
    "ld1rw { z25.s }, p2/Z, [%x[args], %[offsetof_Args_clamp_min]]\n"
    "ldr x16, [%x[args], %[offsetof_Args_n_channels]]\n"
    "whilelt p1.s, XZR, x16\n"
    "whilelt p9.s, XZR, x20\n"
    "ld1rw { z13.s }, p2/Z, [%x[args], %[offsetof_Args_clamp_max]]\n"
    "whilelt p8.s, XZR, x17\n"
    "eor p8.b, p2/Z, p8.b, p9.b\n"
    "ldr x15, [%x[args], %[offsetof_Args_current_channel]]\n"
    "1:"  // Channel loop
    "ldr x20, [%x[args], %[offsetof_Args_bias]]\n"
    "fmov z26.s, #0x0\n"
    "cbz x20, 2f\n"
    "ld1w { z26.s }, p1/Z, [x20, x15, LSL #2]\n"
    "2:"  // Load bias: Done
    "ldr x21, [%x[args], %[offsetof_Args_weights]]\n"
    "mov x20, x21\n"
    "fmov z6.s, #0x0\n"
    "ld1w { z15.s }, p2/Z, [x20]\n"
    "incb x20, ALL, MUL #3\n"
    "incb x21\n"
    "ld1w { z29.s }, p2/Z, [x20]\n"
    ".inst 0x648aa9e6  // bfcvtnt z6.h, p2/M, z15.s\n"
    "incb x20, ALL, MUL #3\n"
    "ld1w { z30.s }, p2/Z, [x20]\n"
    "mov x20, x21\n"
    ".inst 0x658aa9e5  // bfcvt z5.h, p2/M, z15.s\n"
    "ld1w { z14.s }, p2/Z, [x20]\n"
    ".inst 0x658aaba8  // bfcvt z8.h, p2/M, z29.s\n"
    "fmov z11.s, #0x0\n"
    "incb x20, ALL, MUL #3\n"
    ".inst 0x658aa9ca  // bfcvt z10.h, p2/M, z14.s\n"
    ".inst 0x648aaba5  // bfcvtnt z5.h, p2/M, z29.s\n"
    "incb x21\n"
    "ld1w { z24.s }, p2/Z, [x20]\n"
    "incb x20, ALL, MUL #3\n"
    ".inst 0x648aabc8  // bfcvtnt z8.h, p2/M, z30.s\n"
    ".inst 0x658aabcc  // bfcvt z12.h, p2/M, z30.s\n"
    "ld1w { z28.s }, p2/Z, [x20]\n"
    "mov x21, x21\n"
    ".inst 0x648aa9cb  // bfcvtnt z11.h, p2/M, z14.s\n"
    "ld1w { z20.s }, p2/Z, [x21]\n"
    "incb x21, ALL, MUL #3\n"
    ".inst 0x648aab0a  // bfcvtnt z10.h, p2/M, z24.s\n"
    ".inst 0x658aab09  // bfcvt z9.h, p2/M, z24.s\n"
    "ld1w { z15.s }, p2/Z, [x21]\n"
    "ldr x14, [%x[args], %[offsetof_Args_input_cols]]\n"
    "incb x21, ALL, MUL #3\n"
    "fmov z14.s, #0x0\n"
    ".inst 0x658aaa81  // bfcvt z1.h, p2/M, z20.s\n"
    "ldr x13, [%x[args], %[offsetof_Args_inptr]]\n"
    ".inst 0x658aa9e7  // bfcvt z7.h, p2/M, z15.s\n"
    ".inst 0x648aab89  // bfcvtnt z9.h, p2/M, z28.s\n"
    "sub x20, x14, #0x1\n"
    "orr x23, x20, %x[ld_in_col], LSL #18\n"
    ".inst 0x658aab84  // bfcvt z4.h, p2/M, z28.s\n"
    "ld1w { z29.s }, p2/Z, [x21]\n"
    "orr x23, x16, x23, LSL #20\n"
    "mov x22, #0x6\n"
    "add x21, x17, x7\n"
    "lsl x20, %x[ld_in_row], #0x2\n"
    "mov z27.d, z26.d\n"
    ".inst 0x648aaa8e  // bfcvtnt z14.h, p2/M, z20.s\n"
    ".inst 0x648aa9e1  // bfcvtnt z1.h, p2/M, z15.s\n"
    ".inst 0x648aaba7  // bfcvtnt z7.h, p2/M, z29.s\n"
    "mov x8, #0x0\n"
    "ldr x11, [%x[args], %[offsetof_Args_output_cols]]\n"
    ".inst 0x658aaba2  // bfcvt z2.h, p2/M, z29.s\n"
    "lsl x23, x23, #0x2\n"
    "sub x22, x22, x21\n"
    "madd x20, x20, x17, x13\n"
    "3:"  // Issue prefetches
    "subs x22, x22, #0x1\n"
    ".inst 0xf8b74a9c  // rprfm pldstrm, x23, [x20]\n"
    "add x20, x20, %x[ld_in_col], LSL #2\n"
    "bgt 3b\n"
    "ldr x22, [%x[args], %[offsetof_Args_outptrs]]\n"
    "lsl x20, %x[ld_in_row], #0x2\n"
    "msub x13, x17, x20, x13\n"
    ".inst 0xc0040b40  // mova za.d[x8, #0], { z26.d-z27.d }\n"
    "ldr x20, [%x[args], %[offsetof_Args_ld_out_cols]]\n"
    ".inst 0xc0040b41  // mova za.d[x8, #1], { z26.d-z27.d }\n"
    "mov x10, #0x2\n"
    "ldp x9, x28, [x22], #0x10\n"
    ".inst 0xc0040b42  // mova za.d[x8, #2], { z26.d-z27.d }\n"
    "ldp x27, x26, [x20], #0x10\n"
    ".inst 0xc0040b43  // mova za.d[x8, #3], { z26.d-z27.d }\n"
    "ldr x21, [%x[args], %[offsetof_Args_pad_left]]\n"
    ".inst 0xc0040b44  // mova za.d[x8, #4], { z26.d-z27.d }\n"
    "ldp x25, x24, [x22], #0x10\n"
    ".inst 0xc0040b45  // mova za.d[x8, #5], { z26.d-z27.d }\n"
    "ldp x23, x22, [x20], #0x10\n"
    "cbz x21, 5f\n"
    "cmp x21, x10\n"
    "csel x20, x21, x10, LT\n"
    "sub x21, x21, x20\n"
    "sub x10, x10, x20\n"
    "cbz x21, 5f\n"
    ".inst 0xc0060814  // mova { z20.d-z21.d }, za.d[x8, #0]\n"
    "sub x11, x11, x21\n"
    ".inst 0xc0060836  // mova { z22.d-z23.d }, za.d[x8, #1]\n"
    ".inst 0xc1adcb34  // fclamp { z20.s-z23.s }, z25.s, z13.s\n"
    "4:"  // Left padding
    "subs x21, x21, #0x1\n"
    "st1w { z20.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z22.s }, p1, [x28]\n"
    "add x28, x28, x26, LSL #2\n"
    "st1w { z21.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    "st1w { z23.s }, p1, [x24]\n"
    "add x24, x24, x22, LSL #2\n"
    "bgt 4b\n"
    "5:"  // Left padding: End
    "adds XZR, x17, x7\n"
    "bne 10f\n"
    "cbz x10, 8f\n"
    "cmp x10, #0x1\n"
    "sub x14, x14, x10\n"
    "beq 7f\n"
    "6:"  // Unpadded: 2 priming loads
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z17.s }, p1/Z, [x13]\n"
    ".inst 0x658aaa3e  // bfcvt z30.h, p2/M, z17.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z28.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aab9e  // bfcvtnt z30.h, p2/M, z28.s\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa1f  // bfcvt z31.h, p2/M, z16.s\n"
    "ld1w { z15.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa9ff  // bfcvtnt z31.h, p2/M, z15.s\n"
    ".inst 0xc12513d0  // bfdot za.s[x8, 0], { z30.h-z31.h }, z5.h\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa00  // bfcvt z0.h, p2/M, z16.s\n"
    ".inst 0xc12613d1  // bfdot za.s[x8, 1], { z30.h-z31.h }, z6.h\n"
    "ld1w { z15.s }, p1/Z, [x20]\n"
    ".inst 0x648aa9e0  // bfcvtnt z0.h, p2/M, z15.s\n"
    ".inst 0xc12c13f0  // bfdot za.s[x8, 0], { z31.h-z0.h }, z12.h\n"
    ".inst 0xc12813f1  // bfdot za.s[x8, 1], { z31.h-z0.h }, z8.h\n"
    "7:"  // Unpadded: 1 priming loads
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z31.s }, p1/Z, [x13]\n"
    ".inst 0x658aabef  // bfcvt z15.h, p2/M, z31.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa0f  // bfcvtnt z15.h, p2/M, z16.s\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
    "ld1w { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa30  // bfcvtnt z16.h, p2/M, z17.s\n"
    ".inst 0xc12a11f0  // bfdot za.s[x8, 0], { z15.h-z16.h }, z10.h\n"
    "ld1w { z22.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaad1  // bfcvt z17.h, p2/M, z22.s\n"
    ".inst 0xc12b11f1  // bfdot za.s[x8, 1], { z15.h-z16.h }, z11.h\n"
    "ld1w { z18.s }, p1/Z, [x20]\n"
    ".inst 0x648aaa51  // bfcvtnt z17.h, p2/M, z18.s\n"
    ".inst 0xc12511f2  // bfdot za.s[x8, 2], { z15.h-z16.h }, z5.h\n"
    ".inst 0xc12611f3  // bfdot za.s[x8, 3], { z15.h-z16.h }, z6.h\n"
    ".inst 0xc1241210  // bfdot za.s[x8, 0], { z16.h-z17.h }, z4.h\n"
    ".inst 0xc1291211  // bfdot za.s[x8, 1], { z16.h-z17.h }, z9.h\n"
    ".inst 0xc12c1212  // bfdot za.s[x8, 2], { z16.h-z17.h }, z12.h\n"
    ".inst 0xc1281213  // bfdot za.s[x8, 3], { z16.h-z17.h }, z8.h\n"
    "8:"  // Unpadded: 0 priming loads
    "cbz x14, 16f\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x13]\n"
    ".inst 0x658aaa16  // bfcvt z22.h, p2/M, z16.s\n"
    "sub x14, x14, #0x1\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    "sub x11, x11, #0x1\n"
    ".inst 0x648aaa16  // bfcvtnt z22.h, p2/M, z16.s\n"
    "ld1w { z0.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa817  // bfcvt z23.h, p2/M, z0.s\n"
    "cmp x14, x11\n"
    "ld1w { z24.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    "csel x21, x14, x11, LT\n"
    ".inst 0x648aab17  // bfcvtnt z23.h, p2/M, z24.s\n"
    "ld1w { z0.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa818  // bfcvt z24.h, p2/M, z0.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    ".inst 0x648aaa18  // bfcvtnt z24.h, p2/M, z16.s\n"
    "sub x11, x11, x21\n"
    "cbz x21, 15f\n"
    "9:"  // Unpadded: Main loop
    ".inst 0xc12112d0  // bfdot za.s[x8, 0], { z22.h-z23.h }, z1.h\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z0.s }, p1/Z, [x13]\n"
    "subs x21, x21, #0x1\n"
    ".inst 0xc12e12d1  // bfdot za.s[x8, 1], { z22.h-z23.h }, z14.h\n"
    "ld1w { z20.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z19.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc12212f0  // bfdot za.s[x8, 0], { z23.h-z24.h }, z2.h\n"
    ".inst 0xc12712f1  // bfdot za.s[x8, 1], { z23.h-z24.h }, z7.h\n"
    "ld1w { z18.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc12a12d2  // bfdot za.s[x8, 2], { z22.h-z23.h }, z10.h\n"
    "ld1w { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc12b12d3  // bfdot za.s[x8, 3], { z22.h-z23.h }, z11.h\n"
    "ld1w { z28.s }, p1/Z, [x20]\n"
    ".inst 0xc12512d4  // bfdot za.s[x8, 4], { z22.h-z23.h }, z5.h\n"
    ".inst 0xc12612d5  // bfdot za.s[x8, 5], { z22.h-z23.h }, z6.h\n"
    ".inst 0x658aa816  // bfcvt z22.h, p2/M, z0.s\n"
    ".inst 0x648aaa96  // bfcvtnt z22.h, p2/M, z20.s\n"
    ".inst 0xc12412f2  // bfdot za.s[x8, 2], { z23.h-z24.h }, z4.h\n"
    ".inst 0xc12912f3  // bfdot za.s[x8, 3], { z23.h-z24.h }, z9.h\n"
    ".inst 0xc12c12f4  // bfdot za.s[x8, 4], { z23.h-z24.h }, z12.h\n"
    ".inst 0xc12812f5  // bfdot za.s[x8, 5], { z23.h-z24.h }, z8.h\n"
    ".inst 0x658aaa77  // bfcvt z23.h, p2/M, z19.s\n"
    ".inst 0x658aaa38  // bfcvt z24.h, p2/M, z17.s\n"
    ".inst 0xc0060810  // mova { z16.d-z17.d }, za.d[x8, #0]\n"
    ".inst 0x648aaa57  // bfcvtnt z23.h, p2/M, z18.s\n"
    ".inst 0x648aab98  // bfcvtnt z24.h, p2/M, z28.s\n"
    ".inst 0xc0060832  // mova { z18.d-z19.d }, za.d[x8, #1]\n"
    "add x8, x8, #0x2\n"
    ".inst 0xc1adcb30  // fclamp { z16.s-z19.s }, z25.s, z13.s\n"
    "st1w { z16.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z18.s }, p1, [x28]\n"
    "add x28, x28, x26, LSL #2\n"
    ".inst 0xc0040b44  // mova za.d[x8, #4], { z26.d-z27.d }\n"
    "st1w { z17.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    ".inst 0xc0040b45  // mova za.d[x8, #5], { z26.d-z27.d }\n"
    "st1w { z19.s }, p1, [x24]\n"
    "add x24, x24, x22, LSL #2\n"
    "bgt 9b\n"
    "b 15f\n"
    "10:"  // Padded
    "cbz x10, 13f\n"
    "cmp x10, #0x1\n"
    "sub x14, x14, x10\n"
    "beq 12f\n"
    "11:"  // Padded: 2 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa14  // bfcvt z20.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa14  // bfcvtnt z20.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa15  // bfcvt z21.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa15  // bfcvtnt z21.h, p2/M, z16.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1251290  // bfdot za.s[x8, 0], { z20.h-z21.h }, z5.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z23.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaaf6  // bfcvt z22.h, p2/M, z23.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa16  // bfcvtnt z22.h, p2/M, z16.s\n"
    ".inst 0xc1261291  // bfdot za.s[x8, 1], { z20.h-z21.h }, z6.h\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0xc12c12b0  // bfdot za.s[x8, 0], { z21.h-z22.h }, z12.h\n"
    ".inst 0xc12812b1  // bfdot za.s[x8, 1], { z21.h-z22.h }, z8.h\n"
    "12:"  // Padded: 1 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa13  // bfcvt z19.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa13  // bfcvtnt z19.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa14  // bfcvt z20.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa14  // bfcvtnt z20.h, p2/M, z16.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc12a1270  // bfdot za.s[x8, 0], { z19.h-z20.h }, z10.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z15.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa9f5  // bfcvt z21.h, p2/M, z15.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa15  // bfcvtnt z21.h, p2/M, z16.s\n"
    ".inst 0xc12b1271  // bfdot za.s[x8, 1], { z19.h-z20.h }, z11.h\n"
    ".inst 0xc1251272  // bfdot za.s[x8, 2], { z19.h-z20.h }, z5.h\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0xc1261273  // bfdot za.s[x8, 3], { z19.h-z20.h }, z6.h\n"
    ".inst 0xc1241290  // bfdot za.s[x8, 0], { z20.h-z21.h }, z4.h\n"
    ".inst 0xc1291291  // bfdot za.s[x8, 1], { z20.h-z21.h }, z9.h\n"
    ".inst 0xc12c1292  // bfdot za.s[x8, 2], { z20.h-z21.h }, z12.h\n"
    ".inst 0xc1281293  // bfdot za.s[x8, 3], { z20.h-z21.h }, z8.h\n"
    "13:"  // Padded: 0 priming loads
    "cbz x14, 16f\n"
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa16  // bfcvt z22.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa16  // bfcvtnt z22.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa17  // bfcvt z23.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa17  // bfcvtnt z23.h, p2/M, z16.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa18  // bfcvt z24.h, p2/M, z16.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "sub x14, x14, #0x1\n"
    ".inst 0x648aaa18  // bfcvtnt z24.h, p2/M, z16.s\n"
    "sub x11, x11, #0x1\n"
    "cmp x14, x11\n"
    "csel x21, x14, x11, LT\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "sub x11, x11, x21\n"
    "cbz x21, 15f\n"
    "14:"  // Padded: Main loop
    "mov x12, #0x0\n"
    ".inst 0xc12112d0  // bfdot za.s[x8, 0], { z22.h-z23.h }, z1.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z20.s }, p0/Z, [x13]\n"
    ".inst 0xc12e12d1  // bfdot za.s[x8, 1], { z22.h-z23.h }, z14.h\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z17.s }, p0/Z, [x20]\n"
    ".inst 0xc12212f0  // bfdot za.s[x8, 0], { z23.h-z24.h }, z2.h\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    ".inst 0xc12712f1  // bfdot za.s[x8, 1], { z23.h-z24.h }, z7.h\n"
    "ld1w { z18.s }, p0/Z, [x20]\n"
    "mov x12, #0x4\n"
    ".inst 0xc12a12d2  // bfdot za.s[x8, 2], { z22.h-z23.h }, z10.h\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc12b12d3  // bfdot za.s[x8, 3], { z22.h-z23.h }, z11.h\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc12512d4  // bfdot za.s[x8, 4], { z22.h-z23.h }, z5.h\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z15.s }, p0/Z, [x20]\n"
    "subs x21, x21, #0x1\n"
    ".inst 0xc12612d5  // bfdot za.s[x8, 5], { z22.h-z23.h }, z6.h\n"
    ".inst 0x658aaa96  // bfcvt z22.h, p2/M, z20.s\n"
    ".inst 0x648aaa76  // bfcvtnt z22.h, p2/M, z19.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0xc12412f2  // bfdot za.s[x8, 2], { z23.h-z24.h }, z4.h\n"
    ".inst 0xc12912f3  // bfdot za.s[x8, 3], { z23.h-z24.h }, z9.h\n"
    ".inst 0xc12c12f4  // bfdot za.s[x8, 4], { z23.h-z24.h }, z12.h\n"
    ".inst 0xc12812f5  // bfdot za.s[x8, 5], { z23.h-z24.h }, z8.h\n"
    ".inst 0x658aaa37  // bfcvt z23.h, p2/M, z17.s\n"
    ".inst 0x658aaa18  // bfcvt z24.h, p2/M, z16.s\n"
    ".inst 0xc0060810  // mova { z16.d-z17.d }, za.d[x8, #0]\n"
    ".inst 0x648aaa57  // bfcvtnt z23.h, p2/M, z18.s\n"
    ".inst 0x648aa9f8  // bfcvtnt z24.h, p2/M, z15.s\n"
    ".inst 0xc0060832  // mova { z18.d-z19.d }, za.d[x8, #1]\n"
    "add x8, x8, #0x2\n"
    ".inst 0xc1adcb30  // fclamp { z16.s-z19.s }, z25.s, z13.s\n"
    "st1w { z16.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z18.s }, p1, [x28]\n"
    "add x28, x28, x26, LSL #2\n"
    ".inst 0xc0040b44  // mova za.d[x8, #4], { z26.d-z27.d }\n"
    "st1w { z17.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    ".inst 0xc0040b45  // mova za.d[x8, #5], { z26.d-z27.d }\n"
    "st1w { z19.s }, p1, [x24]\n"
    "add x24, x24, x22, LSL #2\n"
    "bgt 14b\n"
    "15:"  // Main loop tail
    ".inst 0xc12112d0  // bfdot za.s[x8, 0], { z22.h-z23.h }, z1.h\n"
    ".inst 0xc12e12d1  // bfdot za.s[x8, 1], { z22.h-z23.h }, z14.h\n"
    ".inst 0xc12212f0  // bfdot za.s[x8, 0], { z23.h-z24.h }, z2.h\n"
    ".inst 0xc12712f1  // bfdot za.s[x8, 1], { z23.h-z24.h }, z7.h\n"
    ".inst 0xc12a12d2  // bfdot za.s[x8, 2], { z22.h-z23.h }, z10.h\n"
    ".inst 0xc12b12d3  // bfdot za.s[x8, 3], { z22.h-z23.h }, z11.h\n"
    ".inst 0xc12512d4  // bfdot za.s[x8, 4], { z22.h-z23.h }, z5.h\n"
    ".inst 0xc12612d5  // bfdot za.s[x8, 5], { z22.h-z23.h }, z6.h\n"
    ".inst 0xc0060810  // mova { z16.d-z17.d }, za.d[x8, #0]\n"
    ".inst 0xc0060832  // mova { z18.d-z19.d }, za.d[x8, #1]\n"
    ".inst 0xc1adcb30  // fclamp { z16.s-z19.s }, z25.s, z13.s\n"
    "st1w { z16.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    ".inst 0xc12412f2  // bfdot za.s[x8, 2], { z23.h-z24.h }, z4.h\n"
    "st1w { z18.s }, p1, [x28]\n"
    "add x28, x28, x26, LSL #2\n"
    ".inst 0xc12912f3  // bfdot za.s[x8, 3], { z23.h-z24.h }, z9.h\n"
    "st1w { z17.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    ".inst 0xc12c12f4  // bfdot za.s[x8, 4], { z23.h-z24.h }, z12.h\n"
    "st1w { z19.s }, p1, [x24]\n"
    "add x24, x24, x22, LSL #2\n"
    ".inst 0xc12812f5  // bfdot za.s[x8, 5], { z23.h-z24.h }, z8.h\n"
    "add x8, x8, #0x2\n"
    ".inst 0xc0040b44  // mova za.d[x8, #4], { z26.d-z27.d }\n"
    ".inst 0xc0040b45  // mova za.d[x8, #5], { z26.d-z27.d }\n"
    "16:"  // Main loop skip tail
    "cbz x11, 18f\n"
    "17:"  // Right padding loop
    ".inst 0xc006081c  // mova { z28.d-z29.d }, za.d[x8, #0]\n"
    "subs x11, x11, #0x1\n"
    ".inst 0xc006083e  // mova { z30.d-z31.d }, za.d[x8, #1]\n"
    "add x8, x8, #0x2\n"
    ".inst 0xc1adcb3c  // fclamp { z28.s-z31.s }, z25.s, z13.s\n"
    "st1w { z28.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z30.s }, p1, [x28]\n"
    "add x28, x28, x26, LSL #2\n"
    ".inst 0xc0040b44  // mova za.d[x8, #4], { z26.d-z27.d }\n"
    "st1w { z29.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    ".inst 0xc0040b45  // mova za.d[x8, #5], { z26.d-z27.d }\n"
    "st1w { z31.s }, p1, [x24]\n"
    "add x24, x24, x22, LSL #2\n"
    "bgt 17b\n"
    "18:"  // End
    "ldr x20, [%x[args], %[offsetof_Args_weights]]\n"
    "incb x20, ALL, MUL #9\n"
    "str x20, [%x[args], %[offsetof_Args_weights]]\n"
    "incw x15\n"
    "ldr x21, [%x[args], %[offsetof_Args_ld_in_vl]]\n"
    "whilelt p1.s, x15, x16\n"
    "ldr x20, [%x[args], %[offsetof_Args_inptr]]\n"
    "add x20, x20, x21, LSL #2\n"
    "str x20, [%x[args], %[offsetof_Args_inptr]]\n"
    "ldr x25, [%x[args], %[offsetof_Args_outptrs]]\n"
    "ldr x24, [%x[args], %[offsetof_Args_ld_out_vls]]\n"
    "ldp x23, x22, [x25, #0x0]\n"
    "ldp x21, x20, [x24, #0x0]\n"
    "add x23, x23, x21, LSL #2\n"
    "add x22, x22, x20, LSL #2\n"
    "stp x23, x22, [x25, #0x0]\n"
    "ldp x23, x22, [x25, #0x10]\n"
    "ldp x21, x20, [x24, #0x10]\n"
    "add x23, x23, x21, LSL #2\n"
    "add x22, x22, x20, LSL #2\n"
    "stp x23, x22, [x25, #0x10]\n"
    "b.any 1b\n"
    ".inst 0xd503467f  // SMSTOP\n"
    :
    : [args] "r" (&args), [ld_in_col] "r" (ld_in_col), [ld_in_row] "r" (ld_in_row), [offsetof_Args_bias] "I" (offsetof(Args, bias)), [offsetof_Args_clamp_max] "I" (offsetof(Args, clamp_max)), [offsetof_Args_clamp_min] "I" (offsetof(Args, clamp_min)), [offsetof_Args_current_channel] "I" (offsetof(Args, current_channel)), [offsetof_Args_inptr] "I" (offsetof(Args, inptr)), [offsetof_Args_input_cols] "I" (offsetof(Args, input_cols)), [offsetof_Args_ld_in_vl] "I" (offsetof(Args, ld_in_vl)), [offsetof_Args_ld_out_cols] "I" (offsetof(Args, ld_out_cols)), [offsetof_Args_ld_out_vls] "I" (offsetof(Args, ld_out_vls)), [offsetof_Args_n_channels] "I" (offsetof(Args, n_channels)), [offsetof_Args_outptrs] "I" (offsetof(Args, outptrs)), [offsetof_Args_output_cols] "I" (offsetof(Args, output_cols)), [offsetof_Args_pad_bottom] "I" (offsetof(Args, pad_bottom)), [offsetof_Args_pad_left] "I" (offsetof(Args, pad_left)), [offsetof_Args_pad_top] "I" (offsetof(Args, pad_top)), [offsetof_Args_weights] "I" (offsetof(Args, weights))
    : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SME2)
