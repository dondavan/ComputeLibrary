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

void sme2_fp32bf16fp32_planar_3x3_s2_4rows_dot_za_impl(
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

  Args args = { inptr, ld_in_vl, pad_top, 9u - std::min(9u, pad_top + valid_input_rows), pad_left, weights, bias, valid_input_cols, output_cols, outptrs, outlds, outvllds, start_channel, valid_channels, act_min, act_max };

  __asm__ __volatile__(
    "ldr x7, [%x[args], %[offsetof_Args_pad_bottom]]\n"
    "mov x20, #0x9\n"
    ".inst 0xd503477f  // SMSTART ZA\n"
    "sub x20, x20, x7\n"
    "ldr x17, [%x[args], %[offsetof_Args_pad_top]]\n"
    "ptrue p2.b\n"
    "ld1rw { z4.s }, p2/Z, [%x[args], %[offsetof_Args_clamp_min]]\n"
    "ldr x16, [%x[args], %[offsetof_Args_n_channels]]\n"
    "whilelt p1.s, XZR, x16\n"
    "whilelt p9.s, XZR, x20\n"
    "ld1rw { z1.s }, p2/Z, [%x[args], %[offsetof_Args_clamp_max]]\n"
    "whilelt p8.s, XZR, x17\n"
    "eor p8.b, p2/Z, p8.b, p9.b\n"
    "ldr x15, [%x[args], %[offsetof_Args_current_channel]]\n"
    "1:"  // Channel loop
    "ldr x20, [%x[args], %[offsetof_Args_bias]]\n"
    "fmov z24.s, #0x0\n"
    "cbz x20, 2f\n"
    "ld1w { z24.s }, p1/Z, [x20, x15, LSL #2]\n"
    "2:"  // Load bias: Done
    "ldr x21, [%x[args], %[offsetof_Args_weights]]\n"
    "mov x20, x21\n"
    "ld1w { z18.s }, p2/Z, [x20]\n"
    "incb x20, ALL, MUL #3\n"
    "incb x21\n"
    "ld1w { z23.s }, p2/Z, [x20]\n"
    "incb x20, ALL, MUL #3\n"
    ".inst 0x658aaa4e  // bfcvt z14.h, p2/M, z18.s\n"
    "ld1w { z6.s }, p2/Z, [x20]\n"
    "mov x20, x21\n"
    ".inst 0x648aaaee  // bfcvtnt z14.h, p2/M, z23.s\n"
    "incb x21\n"
    "ld1w { z28.s }, p2/Z, [x20]\n"
    "incb x20, ALL, MUL #3\n"
    ".inst 0x658aa8c3  // bfcvt z3.h, p2/M, z6.s\n"
    ".inst 0x658aab88  // bfcvt z8.h, p2/M, z28.s\n"
    "ld1w { z10.s }, p2/Z, [x20]\n"
    "incb x20, ALL, MUL #3\n"
    "ldr x14, [%x[args], %[offsetof_Args_input_cols]]\n"
    ".inst 0x648aa948  // bfcvtnt z8.h, p2/M, z10.s\n"
    "ld1w { z2.s }, p2/Z, [x20]\n"
    "mov x21, x21\n"
    ".inst 0x658aa847  // bfcvt z7.h, p2/M, z2.s\n"
    "ldr x13, [%x[args], %[offsetof_Args_inptr]]\n"
    "ld1w { z9.s }, p2/Z, [x21]\n"
    "incb x21, ALL, MUL #3\n"
    ".inst 0x658aa920  // bfcvt z0.h, p2/M, z9.s\n"
    "sub x20, x14, #0x1\n"
    "ld1w { z6.s }, p2/Z, [x21]\n"
    "incb x21, ALL, MUL #3\n"
    "orr x23, x20, %x[ld_in_col], LSL #18\n"
    "mov z25.d, z24.d\n"
    "ld1w { z17.s }, p2/Z, [x21]\n"
    "orr x23, x16, x23, LSL #20\n"
    "mov x22, #0x9\n"
    "mov z26.d, z24.d\n"
    "add x21, x17, x7\n"
    "lsl x20, %x[ld_in_row], #0x2\n"
    "mov z27.d, z24.d\n"
    ".inst 0x648aa8c0  // bfcvtnt z0.h, p2/M, z6.s\n"
    ".inst 0x658aaa26  // bfcvt z6.h, p2/M, z17.s\n"
    "mov x8, #0x0\n"
    "ldr x11, [%x[args], %[offsetof_Args_output_cols]]\n"
    "lsl x23, x23, #0x2\n"
    "sub x22, x22, x21\n"
    "madd x20, x20, x17, x13\n"
    "3:"  // Issue prefetches
    "subs x22, x22, #0x1\n"
    ".inst 0xf8b74a9c  // rprfm pldstrm, x23, [x20]\n"
    "add x20, x20, %x[ld_in_col], LSL #2\n"
    "bgt 3b\n"
    "ldr x23, [%x[args], %[offsetof_Args_outptrs]]\n"
    "lsl x20, %x[ld_in_row], #0x2\n"
    "msub x13, x17, x20, x13\n"
    ".inst 0xc0040f00  // mova za.d[x8, #0], { z24.d-z27.d }\n"
    "ldr x20, [%x[args], %[offsetof_Args_ld_out_cols]]\n"
    ".inst 0xc0040f01  // mova za.d[x8, #1], { z24.d-z27.d }\n"
    "mov x22, #0x2\n"
    "ldp x10, x9, [x23], #0x10\n"
    ".inst 0xc0040f02  // mova za.d[x8, #2], { z24.d-z27.d }\n"
    "ldp x28, x27, [x20], #0x10\n"
    "ldr x21, [%x[args], %[offsetof_Args_pad_left]]\n"
    "ldp x26, x25, [x23], #0x10\n"
    "ldp x24, x23, [x20], #0x10\n"
    "cbz x21, 5f\n"
    "cmp x21, x22\n"
    "csel x20, x21, x22, LT\n"
    "sub x21, x21, x20\n"
    "sub x22, x22, x20\n"
    "cbz x21, 5f\n"
    ".inst 0xc0060c10  // mova { z16.d-z19.d }, za.d[x8, #0]\n"
    "and x22, x21, #0x1\n"
    "add x21, x21, #0x1\n"
    ".inst 0xc1a1c890  // fclamp { z16.s-z19.s }, z4.s, z1.s\n"
    "lsr x21, x21, #0x1\n"
    "sub x11, x11, x21\n"
    "4:"  // Left padding
    "subs x21, x21, #0x1\n"
    "st1w { z16.s }, p1, [x10]\n"
    "add x10, x10, x28, LSL #2\n"
    "st1w { z17.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z18.s }, p1, [x26]\n"
    "add x26, x26, x24, LSL #2\n"
    "st1w { z19.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    "bgt 4b\n"
    "5:"  // Left padding: End
    "adds XZR, x17, x7\n"
    "bne 10f\n"
    "cbz x22, 8f\n"
    "cmp x22, #0x1\n"
    "sub x14, x14, x22\n"
    "beq 7f\n"
    "6:"  // Unpadded: 2 priming loads
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z18.s }, p1/Z, [x13]\n"
    ".inst 0x658aaa53  // bfcvt z19.h, p2/M, z18.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z12.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa993  // bfcvtnt z19.h, p2/M, z12.s\n"
    "ld1w { z23.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaaf4  // bfcvt z20.h, p2/M, z23.s\n"
    "ld1w { z2.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa854  // bfcvtnt z20.h, p2/M, z2.s\n"
    "ld1w { z15.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa9f5  // bfcvt z21.h, p2/M, z15.s\n"
    "ld1w { z22.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaad5  // bfcvtnt z21.h, p2/M, z22.s\n"
    "ld1w { z30.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aabd6  // bfcvt z22.h, p2/M, z30.s\n"
    "ld1w { z12.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa996  // bfcvtnt z22.h, p2/M, z12.s\n"
    ".inst 0xc13e1270  // bfdot za.s[x8, 0], { z19.h-z22.h }, z14.h\n"
    "ld1w { z31.s }, p1/Z, [x20]\n"
    ".inst 0x658aabf7  // bfcvt z23.h, p2/M, z31.s\n"
    ".inst 0xc1331290  // bfdot za.s[x8, 0], { z20.h-z23.h }, z3.h\n"
    "7:"  // Unpadded: 1 priming loads
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z17.s }, p1/Z, [x13]\n"
    ".inst 0x658aaa30  // bfcvt z16.h, p2/M, z17.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z22.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaad0  // bfcvtnt z16.h, p2/M, z22.s\n"
    "ld1w { z28.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aab91  // bfcvt z17.h, p2/M, z28.s\n"
    "ld1w { z18.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa51  // bfcvtnt z17.h, p2/M, z18.s\n"
    "ld1w { z2.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa852  // bfcvt z18.h, p2/M, z2.s\n"
    "ld1w { z19.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa72  // bfcvtnt z18.h, p2/M, z19.s\n"
    "ld1w { z2.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa853  // bfcvt z19.h, p2/M, z2.s\n"
    "ld1w { z23.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaaf3  // bfcvtnt z19.h, p2/M, z23.s\n"
    ".inst 0xc1381210  // bfdot za.s[x8, 0], { z16.h-z19.h }, z8.h\n"
    "ld1w { z10.s }, p1/Z, [x20]\n"
    ".inst 0x658aa954  // bfcvt z20.h, p2/M, z10.s\n"
    ".inst 0xc1371230  // bfdot za.s[x8, 0], { z17.h-z20.h }, z7.h\n"
    "8:"  // Unpadded: 0 priming loads
    "cmp x14, #0x2\n"
    "blt 16f\n"
    "add x21, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x13]\n"
    ".inst 0x658aaa09  // bfcvt z9.h, p2/M, z16.s\n"
    "sub x14, x14, #0x2\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    "sub x11, x11, #0x1\n"
    ".inst 0x648aaa09  // bfcvtnt z9.h, p2/M, z16.s\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0a  // bfcvt z10.h, p2/M, z16.s\n"
    "lsr x20, x14, #0x1\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    "cmp x20, x11\n"
    ".inst 0x648aaa0a  // bfcvtnt z10.h, p2/M, z16.s\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0b  // bfcvt z11.h, p2/M, z16.s\n"
    "csel x22, x20, x11, LT\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa0b  // bfcvtnt z11.h, p2/M, z16.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0c  // bfcvt z12.h, p2/M, z16.s\n"
    "and x14, x14, #0x1\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa0c  // bfcvtnt z12.h, p2/M, z16.s\n"
    "sub x11, x11, x22\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    ".inst 0x658aaa0d  // bfcvt z13.h, p2/M, z16.s\n"
    "cbz x22, 15f\n"
    "9:"  // Unpadded: Main loop
    "add x21, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x13]\n"
    ".inst 0xc1301130  // bfdot za.s[x8, 0], { z9.h-z12.h }, z0.h\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "ld1w { z15.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0xc13e1131  // bfdot za.s[x8, 1], { z9.h-z12.h }, z14.h\n"
    ".inst 0x658aaa09  // bfcvt z9.h, p2/M, z16.s\n"
    "ld1w { z18.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1361150  // bfdot za.s[x8, 0], { z10.h-z13.h }, z6.h\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    "ld1w { z17.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1331151  // bfdot za.s[x8, 1], { z10.h-z13.h }, z3.h\n"
    ".inst 0x658aaa4a  // bfcvt z10.h, p2/M, z18.s\n"
    "ld1w { z30.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aabcb  // bfcvt z11.h, p2/M, z30.s\n"
    ".inst 0x648aa9e9  // bfcvtnt z9.h, p2/M, z15.s\n"
    "ld1w { z19.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa2a  // bfcvtnt z10.h, p2/M, z17.s\n"
    ".inst 0x648aaa6b  // bfcvtnt z11.h, p2/M, z19.s\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0c  // bfcvt z12.h, p2/M, z16.s\n"
    ".inst 0xc0060c10  // mova { z16.d-z19.d }, za.d[x8, #0]\n"
    "ld1w { z2.s }, p1/Z, [x21]\n"
    "add x21, x21, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa84c  // bfcvtnt z12.h, p2/M, z2.s\n"
    "add x8, x8, #0x1\n"
    "ld1w { z29.s }, p1/Z, [x13]\n"
    ".inst 0xc1381130  // bfdot za.s[x8, 0], { z9.h-z12.h }, z8.h\n"
    ".inst 0x658aaba9  // bfcvt z9.h, p2/M, z29.s\n"
    "subs x22, x22, #0x1\n"
    "ld1w { z22.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1a1c890  // fclamp { z16.s-z19.s }, z4.s, z1.s\n"
    "st1w { z16.s }, p1, [x10]\n"
    "ld1w { z16.s }, p1/Z, [x21]\n"
    ".inst 0x658aaa0d  // bfcvt z13.h, p2/M, z16.s\n"
    ".inst 0xc1371150  // bfdot za.s[x8, 0], { z10.h-z13.h }, z7.h\n"
    "add x10, x10, x28, LSL #2\n"
    "ld1w { z28.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aab8a  // bfcvt z10.h, p2/M, z28.s\n"
    "st1w { z17.s }, p1, [x9]\n"
    "ld1w { z31.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z18.s }, p1, [x26]\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0b  // bfcvt z11.h, p2/M, z16.s\n"
    "add x26, x26, x24, LSL #2\n"
    "ld1w { z17.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    "st1w { z19.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0c  // bfcvt z12.h, p2/M, z16.s\n"
    ".inst 0xc0040f02  // mova za.d[x8, #2], { z24.d-z27.d }\n"
    "ld1w { z16.s }, p1/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaac9  // bfcvtnt z9.h, p2/M, z22.s\n"
    ".inst 0x648aabea  // bfcvtnt z10.h, p2/M, z31.s\n"
    "ld1w { z31.s }, p1/Z, [x20]\n"
    ".inst 0x648aaa2b  // bfcvtnt z11.h, p2/M, z17.s\n"
    ".inst 0x648aaa0c  // bfcvtnt z12.h, p2/M, z16.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0x658aabed  // bfcvt z13.h, p2/M, z31.s\n"
    "bgt 9b\n"
    "b 15f\n"
    "10:"  // Padded
    "cbz x22, 13f\n"
    "cmp x22, #0x1\n"
    "sub x14, x14, x22\n"
    "beq 12f\n"
    "11:"  // Padded: 2 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa09  // bfcvt z9.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z18.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa49  // bfcvtnt z9.h, p2/M, z18.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z12.s }, p0/Z, [x20]\n"
    ".inst 0x658aa98a  // bfcvt z10.h, p2/M, z12.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z12.s }, p0/Z, [x20]\n"
    ".inst 0x648aa98a  // bfcvtnt z10.h, p2/M, z12.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z18.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa4b  // bfcvt z11.h, p2/M, z18.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa0b  // bfcvtnt z11.h, p2/M, z16.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0c  // bfcvt z12.h, p2/M, z16.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    ".inst 0x648aaa0c  // bfcvtnt z12.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa0d  // bfcvt z13.h, p2/M, z16.s\n"
    ".inst 0xc13e1130  // bfdot za.s[x8, 0], { z9.h-z12.h }, z14.h\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0xc1331150  // bfdot za.s[x8, 0], { z10.h-z13.h }, z3.h\n"
    "12:"  // Padded: 1 priming loads
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa0f  // bfcvt z15.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa0f  // bfcvtnt z15.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa70  // bfcvtnt z16.h, p2/M, z19.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z13.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa9b1  // bfcvt z17.h, p2/M, z13.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z12.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa991  // bfcvtnt z17.h, p2/M, z12.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z9.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa932  // bfcvt z18.h, p2/M, z9.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z11.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    ".inst 0x648aa972  // bfcvtnt z18.h, p2/M, z11.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z21.s }, p0/Z, [x20]\n"
    ".inst 0x658aaab3  // bfcvt z19.h, p2/M, z21.s\n"
    ".inst 0xc13811f0  // bfdot za.s[x8, 0], { z15.h-z18.h }, z8.h\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0xc1371210  // bfdot za.s[x8, 0], { z16.h-z19.h }, z7.h\n"
    "13:"  // Padded: 0 priming loads
    "cmp x14, #0x2\n"
    "blt 16f\n"
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa09  // bfcvt z9.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa09  // bfcvtnt z9.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa0a  // bfcvt z10.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa0a  // bfcvtnt z10.h, p2/M, z16.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0b  // bfcvt z11.h, p2/M, z16.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa0b  // bfcvtnt z11.h, p2/M, z16.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa0c  // bfcvt z12.h, p2/M, z16.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "mov x12, #0x8\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa0c  // bfcvtnt z12.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa0d  // bfcvt z13.h, p2/M, z16.s\n"
    "sub x14, x14, #0x2\n"
    "sub x11, x11, #0x1\n"
    "lsr x20, x14, #0x1\n"
    "cmp x20, x11\n"
    "csel x21, x20, x11, LT\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "and x14, x14, #0x1\n"
    "sub x11, x11, x21\n"
    "cbz x21, 15f\n"
    "14:"  // Padded: Main loop
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z18.s }, p0/Z, [x13]\n"
    ".inst 0xc1301130  // bfdot za.s[x8, 0], { z9.h-z12.h }, z0.h\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z17.s }, p0/Z, [x20]\n"
    ".inst 0xc13e1131  // bfdot za.s[x8, 1], { z9.h-z12.h }, z14.h\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0xc1361150  // bfdot za.s[x8, 0], { z10.h-z13.h }, z6.h\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    ".inst 0xc1331151  // bfdot za.s[x8, 1], { z10.h-z13.h }, z3.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa49  // bfcvt z9.h, p2/M, z18.s\n"
    ".inst 0x658aaa0a  // bfcvt z10.h, p2/M, z16.s\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z2.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa84b  // bfcvt z11.h, p2/M, z2.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z15.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aaa29  // bfcvtnt z9.h, p2/M, z17.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z28.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aab8c  // bfcvt z12.h, p2/M, z28.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "mov x12, #0x8\n"
    "ld1w { z17.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa6a  // bfcvtnt z10.h, p2/M, z19.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z13.s }, p0/Z, [x20]\n"
    ".inst 0x648aa9eb  // bfcvtnt z11.h, p2/M, z15.s\n"
    "mov x12, #0x0\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0x648aaa2c  // bfcvtnt z12.h, p2/M, z17.s\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa9ad  // bfcvt z13.h, p2/M, z13.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z21.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1a1c89c  // fclamp { z28.s-z31.s }, z4.s, z1.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z17.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    "st1w { z28.s }, p1, [x10]\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "mov x12, #0x4\n"
    "ld1w { z20.s }, p0/Z, [x20]\n"
    "st1w { z29.s }, p1, [x9]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    "st1w { z30.s }, p1, [x26]\n"
    "add x8, x8, #0x1\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1381130  // bfdot za.s[x8, 0], { z9.h-z12.h }, z8.h\n"
    ".inst 0x658aaa09  // bfcvt z9.h, p2/M, z16.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z18.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0xc1371150  // bfdot za.s[x8, 0], { z10.h-z13.h }, z7.h\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa2a  // bfcvt z10.h, p2/M, z17.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "mov x12, #0x8\n"
    "ld1w { z17.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa6b  // bfcvt z11.h, p2/M, z19.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    ".inst 0x658aaa0c  // bfcvt z12.h, p2/M, z16.s\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "subs x21, x21, #0x1\n"
    "add x10, x10, x28, LSL #2\n"
    "st1w { z31.s }, p1, [x25]\n"
    ".inst 0xc0040f02  // mova za.d[x8, #2], { z24.d-z27.d }\n"
    "add x9, x9, x27, LSL #2\n"
    "add x26, x26, x24, LSL #2\n"
    ".inst 0x648aaaa9  // bfcvtnt z9.h, p2/M, z21.s\n"
    ".inst 0x648aaa8a  // bfcvtnt z10.h, p2/M, z20.s\n"
    "add x25, x25, x23, LSL #2\n"
    ".inst 0x648aaa4b  // bfcvtnt z11.h, p2/M, z18.s\n"
    ".inst 0x648aaa2c  // bfcvtnt z12.h, p2/M, z17.s\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    ".inst 0x658aaa0d  // bfcvt z13.h, p2/M, z16.s\n"
    "bgt 14b\n"
    "15:"  // Main loop tail
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z17.s }, p0/Z, [x13]\n"
    ".inst 0xc1301130  // bfdot za.s[x8, 0], { z9.h-z12.h }, z0.h\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z2.s }, p0/Z, [x20]\n"
    ".inst 0xc13e1131  // bfdot za.s[x8, 1], { z9.h-z12.h }, z14.h\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0xc1361150  // bfdot za.s[x8, 0], { z10.h-z13.h }, z6.h\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z23.s }, p0/Z, [x20]\n"
    ".inst 0xc1331151  // bfdot za.s[x8, 1], { z10.h-z13.h }, z3.h\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa32  // bfcvt z18.h, p2/M, z17.s\n"
    ".inst 0x658aaa13  // bfcvt z19.h, p2/M, z16.s\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa14  // bfcvt z20.h, p2/M, z16.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z15.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aa852  // bfcvtnt z18.h, p2/M, z2.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa15  // bfcvt z21.h, p2/M, z16.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    ".inst 0x648aaaf3  // bfcvtnt z19.h, p2/M, z23.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    ".inst 0x648aa9f4  // bfcvtnt z20.h, p2/M, z15.s\n"
    ".inst 0x648aaa15  // bfcvtnt z21.h, p2/M, z16.s\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0xc0060c1c  // mova { z28.d-z31.d }, za.d[x8, #0]\n"
    "add x8, x8, #0x1\n"
    ".inst 0x658aaa16  // bfcvt z22.h, p2/M, z16.s\n"
    ".inst 0xc1381250  // bfdot za.s[x8, 0], { z18.h-z21.h }, z8.h\n"
    ".inst 0xc1a1c89c  // fclamp { z28.s-z31.s }, z4.s, z1.s\n"
    "st1w { z28.s }, p1, [x10]\n"
    "add x10, x10, x28, LSL #2\n"
    "st1w { z29.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    ".inst 0xc0040f02  // mova za.d[x8, #2], { z24.d-z27.d }\n"
    "add x13, x13, %x[ld_in_col], LSL #2\n"
    "st1w { z30.s }, p1, [x26]\n"
    "add x26, x26, x24, LSL #2\n"
    ".inst 0xc1371270  // bfdot za.s[x8, 0], { z19.h-z22.h }, z7.h\n"
    "st1w { z31.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    "16:"  // Main loop skip tail
    "cbz x14, 17f\n"  // Skip remainder inputs
    "mov x12, #0x0\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z16.s }, p0/Z, [x13]\n"
    ".inst 0x658aaa0f  // bfcvt z15.h, p2/M, z16.s\n"
    "add x20, x13, %x[ld_in_row], LSL #2\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x648aaa0f  // bfcvtnt z15.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z16.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa10  // bfcvt z16.h, p2/M, z16.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z2.s }, p0/Z, [x20]\n"
    ".inst 0x648aa850  // bfcvtnt z16.h, p2/M, z2.s\n"
    "mov x12, #0x4\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z10.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aa951  // bfcvt z17.h, p2/M, z10.s\n"
    ".inst 0x25704500  // psel p0.s, p1.s/Z, p8.s[w12, #1]\n"
    "ld1w { z30.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x648aabd1  // bfcvtnt z17.h, p2/M, z30.s\n"
    ".inst 0x25b04500  // psel p0.s, p1.s/Z, p8.s[w12, #2]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x658aaa72  // bfcvt z18.h, p2/M, z19.s\n"
    ".inst 0x25f04500  // psel p0.s, p1.s/Z, p8.s[w12, #3]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    "mov x12, #0x8\n"
    ".inst 0x648aaa72  // bfcvtnt z18.h, p2/M, z19.s\n"
    "add x20, x20, %x[ld_in_row], LSL #2\n"
    ".inst 0x25304500  // psel p0.s, p1.s/Z, p8.s[w12]\n"
    "ld1w { z19.s }, p0/Z, [x20]\n"
    ".inst 0x658aaa73  // bfcvt z19.h, p2/M, z19.s\n"
    ".inst 0xc13011f0  // bfdot za.s[x8, 0], { z15.h-z18.h }, z0.h\n"
    "sub x11, x11, #0x1\n"
    ".inst 0xc1361210  // bfdot za.s[x8, 0], { z16.h-z19.h }, z6.h\n"
    ".inst 0xc13e11f1  // bfdot za.s[x8, 1], { z15.h-z18.h }, z14.h\n"
    ".inst 0xc0060c08  // mova { z8.d-z11.d }, za.d[x8, #0]\n"
    ".inst 0xc1a1c888  // fclamp { z8.s-z11.s }, z4.s, z1.s\n"
    "st1w { z8.s }, p1, [x10]\n"
    "add x10, x10, x28, LSL #2\n"
    ".inst 0xc1331211  // bfdot za.s[x8, 1], { z16.h-z19.h }, z3.h\n"
    "add x8, x8, #0x1\n"
    "st1w { z9.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z10.s }, p1, [x26]\n"
    "add x26, x26, x24, LSL #2\n"
    ".inst 0xc0040f02  // mova za.d[x8, #2], { z24.d-z27.d }\n"
    "st1w { z11.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    "17:"  // Tail input: End
    "cbz x11, 19f\n"
    "18:"  // Right padding loop
    ".inst 0xc0060c08  // mova { z8.d-z11.d }, za.d[x8, #0]\n"
    "add x8, x8, #0x1\n"
    "subs x11, x11, #0x1\n"
    ".inst 0xc1a1c888  // fclamp { z8.s-z11.s }, z4.s, z1.s\n"
    "st1w { z8.s }, p1, [x10]\n"
    "add x10, x10, x28, LSL #2\n"
    ".inst 0xc0040f02  // mova za.d[x8, #2], { z24.d-z27.d }\n"
    "st1w { z9.s }, p1, [x9]\n"
    "add x9, x9, x27, LSL #2\n"
    "st1w { z10.s }, p1, [x26]\n"
    "add x26, x26, x24, LSL #2\n"
    "st1w { z11.s }, p1, [x25]\n"
    "add x25, x25, x23, LSL #2\n"
    "bgt 18b\n"
    "19:"  // End
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
