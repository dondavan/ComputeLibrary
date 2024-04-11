/*
 * Copyright (c) 2021, 2023 Arm Limited.
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
#ifdef ARM_COMPUTE_ENABLE_SVE

#include "arm_gemm.hpp"
#include "../../utils.hpp"

#include <cassert>

namespace arm_gemm {

void sve_hybrid_s8s32_dot_6x4VL_a64fx (
    unsigned int num_strings, const unsigned int *string_lengths, IndirectInputArg<int8_t> A_arg,
    size_t M, size_t N, const int8_t *B_ptr, IndirectOutputArg<int32_t> output_arg,
    const int32_t *, Activation, bool accumulate
)
{
    struct KernelArgs {
        unsigned int num_strings = {};
        const unsigned int *string_lengths = {};
        size_t N = {};
        const int8_t *B_ptr = {};
        size_t output_offset = {};
        size_t input_initial_col = {};
        size_t input_offset = {};
    } ka;

    unsigned long flags=0;
    void *output_ptr;
    void *input_ptr;

    if (output_arg.is_indirect) {
        output_ptr=(void *)(output_arg.indirect.ptr);
        ka.output_offset=output_arg.indirect.offset;
        flags |= 0x4;
    } else {
        output_ptr=(void *)(output_arg.direct.base);
        ka.output_offset=output_arg.direct.stride;
    }

    if (A_arg.is_indirect) {
        input_ptr=(void *)(A_arg.indirect.ptr);
        ka.input_offset=A_arg.indirect.start_row;
        ka.input_initial_col=A_arg.indirect.start_col;
        flags |= 0x8;
    } else {
        assert(num_strings==1);
        input_ptr=(void *)(A_arg.direct.base);
        ka.input_offset=A_arg.direct.stride;
    }
    if (accumulate) {
        flags |= 0x1;
    }
    ka.num_strings = num_strings;
    ka.string_lengths = string_lengths;
    ka.N = N;
    ka.B_ptr = B_ptr;
    __asm__ __volatile__(
      "ptrue p4.b\n"
      "1:"  // Row loop
      "cmp %x[M], #0x6\n"
      "bge 51f\n"
      "cmp %x[M], #0x4\n"
      "bgt 41f\n"
      "beq 31f\n"
      "cmp %x[M], #0x2\n"
      "bgt 21f\n"
      "beq 11f\n"
      "ldr x11, [%x[args_ptr], %[offsetof_N]]\n"
      "ldr x10, [%x[args_ptr], %[offsetof_B_ptr]]\n"
      "mov x9, %x[output_ptr]\n"
      "2:"  // Height 1: Column loop
      "mov x20, #0x0\n"
      "whilelt p3.s, x20, x11\n"
      "incw x20\n"
      "whilelt p2.s, x20, x11\n"
      "incw x20\n"
      "whilelt p1.s, x20, x11\n"
      "incw x20\n"
      "whilelt p0.s, x20, x11\n"
      "tbz %x[flags], #0, 3f\n"
      "ld1w { z8.s }, p3/Z, [x9]\n"
      "ld1w { z9.s }, p2/Z, [x9, #1, MUL VL]\n"
      "ld1w { z10.s }, p1/Z, [x9, #2, MUL VL]\n"
      "ld1w { z11.s }, p0/Z, [x9, #3, MUL VL]\n"
      "b 4f\n"
      "3:"  // Height 1: no accumulate
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "4:"  // Height 1: setup done
      "mov x28, #0x0\n"
      "5:"  // Height 1: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w27, [x20, x28, LSL #0x2]\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 6f\n"
      "ldr x20, [%x[input_ptr], x28, LSL #0x3]\n"
      "add x20, x20, x21, LSL #3\n"
      "ldr x26, [x20, #0x0]\n"
      "cbnz x28, 7f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x26, x26, x20\n"
      "b 7f\n"
      "6:"  // Height 1: setup direct input
      "mov x26, %x[input_ptr]\n"
      "7:"  // Height 1: input setup done
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "ble 9f\n"
      "8:"  // Height 1: Multiply loop: Main loop
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z9.s, z7.b, z0.b\n"
      "ld1b { z17.b }, p4/Z, [x10, #2, MUL VL]\n"
      "ld1b { z16.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "add x26, x26, #0x4\n"
      "sdot z10.s, z17.b, z0.b\n"
      "sdot z11.s, z16.b, z0.b\n"
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "bgt 8b\n"
      "9:"  // Height 1: Multiply loop: Main loop skip
      "ldr w20, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z9.s, z7.b, z0.b\n"
      "ld1b { z17.b }, p4/Z, [x10, #2, MUL VL]\n"
      "ld1b { z16.b }, p4/Z, [x10, #3, MUL VL]\n"
      "add x28, x28, #0x1\n"
      "cmp x28, x20\n"
      "sdot z10.s, z17.b, z0.b\n"
      "sdot z11.s, z16.b, z0.b\n"
      "addvl x10, x10, #4\n"
      "bne 5b\n"
      "st1w { z8.s }, p3, [x9]\n"
      "st1w { z9.s }, p2, [x9, #1, MUL VL]\n"
      "st1w { z10.s }, p1, [x9, #2, MUL VL]\n"
      "st1w { z11.s }, p0, [x9, #3, MUL VL]\n"
      "addvl x9, x9, #4\n"
      "10:"  // Height 1: Writeback done
      "decw x11, ALL, MUL #4\n"
      "cmp x11, XZR\n"
      "bgt 2b\n"
      "b 62f\n"
      "11:"  // Height 2
      "ldr x11, [%x[args_ptr], %[offsetof_N]]\n"
      "ldr x10, [%x[args_ptr], %[offsetof_B_ptr]]\n"
      "mov x9, %x[output_ptr]\n"
      "12:"  // Height 2: Column loop
      "mov x20, #0x0\n"
      "whilelt p3.s, x20, x11\n"
      "incw x20\n"
      "whilelt p2.s, x20, x11\n"
      "incw x20\n"
      "whilelt p1.s, x20, x11\n"
      "incw x20\n"
      "whilelt p0.s, x20, x11\n"
      "tbz %x[flags], #0, 13f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x20, x9, x20, LSL #2\n"
      "ld1w { z8.s }, p3/Z, [x9]\n"
      "ld1w { z9.s }, p2/Z, [x9, #1, MUL VL]\n"
      "ld1w { z10.s }, p1/Z, [x9, #2, MUL VL]\n"
      "ld1w { z11.s }, p0/Z, [x9, #3, MUL VL]\n"
      "ld1w { z12.s }, p3/Z, [x20]\n"
      "ld1w { z13.s }, p2/Z, [x20, #1, MUL VL]\n"
      "ld1w { z14.s }, p1/Z, [x20, #2, MUL VL]\n"
      "ld1w { z15.s }, p0/Z, [x20, #3, MUL VL]\n"
      "b 14f\n"
      "13:"  // Height 2: no accumulate
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "mov z12.s, #0x0\n"
      "mov z13.s, #0x0\n"
      "mov z14.s, #0x0\n"
      "mov z15.s, #0x0\n"
      "14:"  // Height 2: setup done
      "mov x28, #0x0\n"
      "15:"  // Height 2: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w27, [x20, x28, LSL #0x2]\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 16f\n"
      "ldr x20, [%x[input_ptr], x28, LSL #0x3]\n"
      "add x20, x20, x21, LSL #3\n"
      "ldr x26, [x20, #0x0]\n"
      "ldr x25, [x20, #0x8]\n"
      "cbnz x28, 17f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "b 17f\n"
      "16:"  // Height 2: setup direct input
      "mov x26, %x[input_ptr]\n"
      "add x25, x26, x21\n"
      "17:"  // Height 2: input setup done
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "ble 19f\n"
      "18:"  // Height 2: Multiply loop: Main loop
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "ld1b { z17.b }, p4/Z, [x10, #2, MUL VL]\n"
      "add x26, x26, #0x4\n"
      "sdot z9.s, z7.b, z0.b\n"
      "sdot z13.s, z7.b, z1.b\n"
      "ld1b { z16.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "subs x27, x27, #0x4\n"
      "add x25, x25, #0x4\n"
      "sdot z10.s, z17.b, z0.b\n"
      "sdot z14.s, z17.b, z1.b\n"
      "sdot z11.s, z16.b, z0.b\n"
      "sdot z15.s, z16.b, z1.b\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "bgt 18b\n"
      "19:"  // Height 2: Multiply loop: Main loop skip
      "ldr w20, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "ld1b { z17.b }, p4/Z, [x10, #2, MUL VL]\n"
      "sdot z9.s, z7.b, z0.b\n"
      "sdot z13.s, z7.b, z1.b\n"
      "ld1b { z16.b }, p4/Z, [x10, #3, MUL VL]\n"
      "add x28, x28, #0x1\n"
      "cmp x28, x20\n"
      "sdot z10.s, z17.b, z0.b\n"
      "sdot z14.s, z17.b, z1.b\n"
      "addvl x10, x10, #4\n"
      "sdot z11.s, z16.b, z0.b\n"
      "sdot z15.s, z16.b, z1.b\n"
      "bne 15b\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x20, x9, x20, LSL #2\n"
      "st1w { z8.s }, p3, [x9]\n"
      "st1w { z9.s }, p2, [x9, #1, MUL VL]\n"
      "st1w { z10.s }, p1, [x9, #2, MUL VL]\n"
      "st1w { z11.s }, p0, [x9, #3, MUL VL]\n"
      "addvl x9, x9, #4\n"
      "st1w { z12.s }, p3, [x20]\n"
      "st1w { z13.s }, p2, [x20, #1, MUL VL]\n"
      "st1w { z14.s }, p1, [x20, #2, MUL VL]\n"
      "st1w { z15.s }, p0, [x20, #3, MUL VL]\n"
      "20:"  // Height 2: Writeback done
      "decw x11, ALL, MUL #4\n"
      "cmp x11, XZR\n"
      "bgt 12b\n"
      "b 62f\n"
      "21:"  // Height 3
      "ldr x11, [%x[args_ptr], %[offsetof_N]]\n"
      "ldr x10, [%x[args_ptr], %[offsetof_B_ptr]]\n"
      "mov x9, %x[output_ptr]\n"
      "22:"  // Height 3: Column loop
      "mov x20, #0x0\n"
      "whilelt p3.s, x20, x11\n"
      "incw x20\n"
      "whilelt p2.s, x20, x11\n"
      "incw x20\n"
      "whilelt p1.s, x20, x11\n"
      "incw x20\n"
      "whilelt p0.s, x20, x11\n"
      "tbz %x[flags], #0, 23f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x21, x9, x20, LSL #2\n"
      "add x20, x21, x20, LSL #2\n"
      "ld1w { z8.s }, p3/Z, [x9]\n"
      "ld1w { z9.s }, p2/Z, [x9, #1, MUL VL]\n"
      "ld1w { z10.s }, p1/Z, [x9, #2, MUL VL]\n"
      "ld1w { z11.s }, p0/Z, [x9, #3, MUL VL]\n"
      "ld1w { z12.s }, p3/Z, [x21]\n"
      "ld1w { z13.s }, p2/Z, [x21, #1, MUL VL]\n"
      "ld1w { z14.s }, p1/Z, [x21, #2, MUL VL]\n"
      "ld1w { z15.s }, p0/Z, [x21, #3, MUL VL]\n"
      "ld1w { z16.s }, p3/Z, [x20]\n"
      "ld1w { z17.s }, p2/Z, [x20, #1, MUL VL]\n"
      "ld1w { z18.s }, p1/Z, [x20, #2, MUL VL]\n"
      "ld1w { z19.s }, p0/Z, [x20, #3, MUL VL]\n"
      "b 24f\n"
      "23:"  // Height 3: no accumulate
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "mov z12.s, #0x0\n"
      "mov z13.s, #0x0\n"
      "mov z14.s, #0x0\n"
      "mov z15.s, #0x0\n"
      "mov z16.s, #0x0\n"
      "mov z17.s, #0x0\n"
      "mov z18.s, #0x0\n"
      "mov z19.s, #0x0\n"
      "24:"  // Height 3: setup done
      "mov x28, #0x0\n"
      "25:"  // Height 3: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w27, [x20, x28, LSL #0x2]\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 26f\n"
      "ldr x20, [%x[input_ptr], x28, LSL #0x3]\n"
      "add x20, x20, x21, LSL #3\n"
      "ldr x26, [x20, #0x0]\n"
      "ldr x25, [x20, #0x8]\n"
      "ldr x24, [x20, #0x10]\n"
      "cbnz x28, 27f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "b 27f\n"
      "26:"  // Height 3: setup direct input
      "mov x26, %x[input_ptr]\n"
      "add x25, x26, x21\n"
      "add x24, x25, x21\n"
      "27:"  // Height 3: input setup done
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "ble 29f\n"
      "28:"  // Height 3: Multiply loop: Main loop
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x26, x26, #0x4\n"
      "subs x27, x27, #0x4\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z9.s, z7.b, z0.b\n"
      "ld1b { z21.b }, p4/Z, [x10, #2, MUL VL]\n"
      "add x25, x25, #0x4\n"
      "sdot z13.s, z7.b, z1.b\n"
      "sdot z17.s, z7.b, z2.b\n"
      "ld1b { z20.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "add x24, x24, #0x4\n"
      "sdot z10.s, z21.b, z0.b\n"
      "sdot z14.s, z21.b, z1.b\n"
      "sdot z18.s, z21.b, z2.b\n"
      "sdot z11.s, z20.b, z0.b\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "sdot z15.s, z20.b, z1.b\n"
      "sdot z19.s, z20.b, z2.b\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "bgt 28b\n"
      "29:"  // Height 3: Multiply loop: Main loop skip
      "ldr w20, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x28, x28, #0x1\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z9.s, z7.b, z0.b\n"
      "ld1b { z21.b }, p4/Z, [x10, #2, MUL VL]\n"
      "cmp x28, x20\n"
      "sdot z13.s, z7.b, z1.b\n"
      "sdot z17.s, z7.b, z2.b\n"
      "ld1b { z20.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z21.b, z0.b\n"
      "sdot z14.s, z21.b, z1.b\n"
      "sdot z18.s, z21.b, z2.b\n"
      "sdot z11.s, z20.b, z0.b\n"
      "sdot z15.s, z20.b, z1.b\n"
      "sdot z19.s, z20.b, z2.b\n"
      "bne 25b\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x21, x9, x20, LSL #2\n"
      "add x20, x21, x20, LSL #2\n"
      "st1w { z8.s }, p3, [x9]\n"
      "st1w { z9.s }, p2, [x9, #1, MUL VL]\n"
      "st1w { z10.s }, p1, [x9, #2, MUL VL]\n"
      "st1w { z11.s }, p0, [x9, #3, MUL VL]\n"
      "addvl x9, x9, #4\n"
      "st1w { z12.s }, p3, [x21]\n"
      "st1w { z13.s }, p2, [x21, #1, MUL VL]\n"
      "st1w { z14.s }, p1, [x21, #2, MUL VL]\n"
      "st1w { z15.s }, p0, [x21, #3, MUL VL]\n"
      "st1w { z16.s }, p3, [x20]\n"
      "st1w { z17.s }, p2, [x20, #1, MUL VL]\n"
      "st1w { z18.s }, p1, [x20, #2, MUL VL]\n"
      "st1w { z19.s }, p0, [x20, #3, MUL VL]\n"
      "30:"  // Height 3: Writeback done
      "decw x11, ALL, MUL #4\n"
      "cmp x11, XZR\n"
      "bgt 22b\n"
      "b 62f\n"
      "31:"  // Height 4
      "ldr x11, [%x[args_ptr], %[offsetof_N]]\n"
      "ldr x10, [%x[args_ptr], %[offsetof_B_ptr]]\n"
      "mov x9, %x[output_ptr]\n"
      "32:"  // Height 4: Column loop
      "mov x20, #0x0\n"
      "whilelt p3.s, x20, x11\n"
      "incw x20\n"
      "whilelt p2.s, x20, x11\n"
      "incw x20\n"
      "whilelt p1.s, x20, x11\n"
      "incw x20\n"
      "whilelt p0.s, x20, x11\n"
      "tbz %x[flags], #0, 33f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x22, x9, x20, LSL #2\n"
      "add x21, x22, x20, LSL #2\n"
      "ld1w { z8.s }, p3/Z, [x9]\n"
      "add x20, x21, x20, LSL #2\n"
      "ld1w { z9.s }, p2/Z, [x9, #1, MUL VL]\n"
      "ld1w { z10.s }, p1/Z, [x9, #2, MUL VL]\n"
      "ld1w { z11.s }, p0/Z, [x9, #3, MUL VL]\n"
      "ld1w { z12.s }, p3/Z, [x22]\n"
      "ld1w { z13.s }, p2/Z, [x22, #1, MUL VL]\n"
      "ld1w { z14.s }, p1/Z, [x22, #2, MUL VL]\n"
      "ld1w { z15.s }, p0/Z, [x22, #3, MUL VL]\n"
      "ld1w { z16.s }, p3/Z, [x21]\n"
      "ld1w { z17.s }, p2/Z, [x21, #1, MUL VL]\n"
      "ld1w { z18.s }, p1/Z, [x21, #2, MUL VL]\n"
      "ld1w { z19.s }, p0/Z, [x21, #3, MUL VL]\n"
      "ld1w { z20.s }, p3/Z, [x20]\n"
      "ld1w { z21.s }, p2/Z, [x20, #1, MUL VL]\n"
      "ld1w { z22.s }, p1/Z, [x20, #2, MUL VL]\n"
      "ld1w { z23.s }, p0/Z, [x20, #3, MUL VL]\n"
      "b 34f\n"
      "33:"  // Height 4: no accumulate
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "mov z12.s, #0x0\n"
      "mov z13.s, #0x0\n"
      "mov z14.s, #0x0\n"
      "mov z15.s, #0x0\n"
      "mov z16.s, #0x0\n"
      "mov z17.s, #0x0\n"
      "mov z18.s, #0x0\n"
      "mov z19.s, #0x0\n"
      "mov z20.s, #0x0\n"
      "mov z21.s, #0x0\n"
      "mov z22.s, #0x0\n"
      "mov z23.s, #0x0\n"
      "34:"  // Height 4: setup done
      "mov x28, #0x0\n"
      "35:"  // Height 4: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w27, [x20, x28, LSL #0x2]\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 36f\n"
      "ldr x20, [%x[input_ptr], x28, LSL #0x3]\n"
      "add x20, x20, x21, LSL #3\n"
      "ldr x26, [x20, #0x0]\n"
      "ldr x25, [x20, #0x8]\n"
      "ldr x24, [x20, #0x10]\n"
      "ldr x23, [x20, #0x18]\n"
      "cbnz x28, 37f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "add x23, x23, x20\n"
      "b 37f\n"
      "36:"  // Height 4: setup direct input
      "mov x26, %x[input_ptr]\n"
      "add x25, x26, x21\n"
      "add x24, x25, x21\n"
      "add x23, x24, x21\n"
      "37:"  // Height 4: input setup done
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1rw { z3.s }, p4/Z, [x23]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "ble 39f\n"
      "38:"  // Height 4: Multiply loop: Main loop
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x26, x26, #0x4\n"
      "subs x27, x27, #0x4\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z20.s, z6.b, z3.b\n"
      "ld1b { z25.b }, p4/Z, [x10, #2, MUL VL]\n"
      "add x25, x25, #0x4\n"
      "sdot z9.s, z7.b, z0.b\n"
      "sdot z13.s, z7.b, z1.b\n"
      "add x24, x24, #0x4\n"
      "add x23, x23, #0x4\n"
      "sdot z17.s, z7.b, z2.b\n"
      "sdot z21.s, z7.b, z3.b\n"
      "ld1b { z24.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z25.b, z0.b\n"
      "sdot z14.s, z25.b, z1.b\n"
      "sdot z18.s, z25.b, z2.b\n"
      "sdot z22.s, z25.b, z3.b\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "sdot z11.s, z24.b, z0.b\n"
      "sdot z15.s, z24.b, z1.b\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "sdot z19.s, z24.b, z2.b\n"
      "sdot z23.s, z24.b, z3.b\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1rw { z3.s }, p4/Z, [x23]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "bgt 38b\n"
      "39:"  // Height 4: Multiply loop: Main loop skip
      "ldr w20, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x28, x28, #0x1\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z20.s, z6.b, z3.b\n"
      "ld1b { z25.b }, p4/Z, [x10, #2, MUL VL]\n"
      "cmp x28, x20\n"
      "sdot z9.s, z7.b, z0.b\n"
      "sdot z13.s, z7.b, z1.b\n"
      "sdot z17.s, z7.b, z2.b\n"
      "sdot z21.s, z7.b, z3.b\n"
      "ld1b { z24.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z25.b, z0.b\n"
      "sdot z14.s, z25.b, z1.b\n"
      "sdot z18.s, z25.b, z2.b\n"
      "sdot z22.s, z25.b, z3.b\n"
      "sdot z11.s, z24.b, z0.b\n"
      "sdot z15.s, z24.b, z1.b\n"
      "sdot z19.s, z24.b, z2.b\n"
      "sdot z23.s, z24.b, z3.b\n"
      "bne 35b\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x22, x9, x20, LSL #2\n"
      "add x21, x22, x20, LSL #2\n"
      "st1w { z8.s }, p3, [x9]\n"
      "add x20, x21, x20, LSL #2\n"
      "st1w { z9.s }, p2, [x9, #1, MUL VL]\n"
      "st1w { z10.s }, p1, [x9, #2, MUL VL]\n"
      "st1w { z11.s }, p0, [x9, #3, MUL VL]\n"
      "addvl x9, x9, #4\n"
      "st1w { z12.s }, p3, [x22]\n"
      "st1w { z13.s }, p2, [x22, #1, MUL VL]\n"
      "st1w { z14.s }, p1, [x22, #2, MUL VL]\n"
      "st1w { z15.s }, p0, [x22, #3, MUL VL]\n"
      "st1w { z16.s }, p3, [x21]\n"
      "st1w { z17.s }, p2, [x21, #1, MUL VL]\n"
      "st1w { z18.s }, p1, [x21, #2, MUL VL]\n"
      "st1w { z19.s }, p0, [x21, #3, MUL VL]\n"
      "st1w { z20.s }, p3, [x20]\n"
      "st1w { z21.s }, p2, [x20, #1, MUL VL]\n"
      "st1w { z22.s }, p1, [x20, #2, MUL VL]\n"
      "st1w { z23.s }, p0, [x20, #3, MUL VL]\n"
      "40:"  // Height 4: Writeback done
      "decw x11, ALL, MUL #4\n"
      "cmp x11, XZR\n"
      "bgt 32b\n"
      "b 62f\n"
      "41:"  // Height 5
      "ldr x11, [%x[args_ptr], %[offsetof_N]]\n"
      "ldr x10, [%x[args_ptr], %[offsetof_B_ptr]]\n"
      "mov x9, %x[output_ptr]\n"
      "42:"  // Height 5: Column loop
      "mov x20, #0x0\n"
      "whilelt p3.s, x20, x11\n"
      "incw x20\n"
      "whilelt p2.s, x20, x11\n"
      "incw x20\n"
      "whilelt p1.s, x20, x11\n"
      "incw x20\n"
      "whilelt p0.s, x20, x11\n"
      "tbz %x[flags], #0, 43f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x23, x9, x20, LSL #2\n"
      "add x22, x23, x20, LSL #2\n"
      "ld1w { z8.s }, p3/Z, [x9]\n"
      "add x21, x22, x20, LSL #2\n"
      "add x20, x21, x20, LSL #2\n"
      "ld1w { z9.s }, p2/Z, [x9, #1, MUL VL]\n"
      "ld1w { z10.s }, p1/Z, [x9, #2, MUL VL]\n"
      "ld1w { z11.s }, p0/Z, [x9, #3, MUL VL]\n"
      "ld1w { z12.s }, p3/Z, [x23]\n"
      "ld1w { z13.s }, p2/Z, [x23, #1, MUL VL]\n"
      "ld1w { z14.s }, p1/Z, [x23, #2, MUL VL]\n"
      "ld1w { z15.s }, p0/Z, [x23, #3, MUL VL]\n"
      "ld1w { z16.s }, p3/Z, [x22]\n"
      "ld1w { z17.s }, p2/Z, [x22, #1, MUL VL]\n"
      "ld1w { z18.s }, p1/Z, [x22, #2, MUL VL]\n"
      "ld1w { z19.s }, p0/Z, [x22, #3, MUL VL]\n"
      "ld1w { z20.s }, p3/Z, [x21]\n"
      "ld1w { z21.s }, p2/Z, [x21, #1, MUL VL]\n"
      "ld1w { z22.s }, p1/Z, [x21, #2, MUL VL]\n"
      "ld1w { z23.s }, p0/Z, [x21, #3, MUL VL]\n"
      "ld1w { z24.s }, p3/Z, [x20]\n"
      "ld1w { z25.s }, p2/Z, [x20, #1, MUL VL]\n"
      "ld1w { z26.s }, p1/Z, [x20, #2, MUL VL]\n"
      "ld1w { z27.s }, p0/Z, [x20, #3, MUL VL]\n"
      "b 44f\n"
      "43:"  // Height 5: no accumulate
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "mov z12.s, #0x0\n"
      "mov z13.s, #0x0\n"
      "mov z14.s, #0x0\n"
      "mov z15.s, #0x0\n"
      "mov z16.s, #0x0\n"
      "mov z17.s, #0x0\n"
      "mov z18.s, #0x0\n"
      "mov z19.s, #0x0\n"
      "mov z20.s, #0x0\n"
      "mov z21.s, #0x0\n"
      "mov z22.s, #0x0\n"
      "mov z23.s, #0x0\n"
      "mov z24.s, #0x0\n"
      "mov z25.s, #0x0\n"
      "mov z26.s, #0x0\n"
      "mov z27.s, #0x0\n"
      "44:"  // Height 5: setup done
      "mov x28, #0x0\n"
      "45:"  // Height 5: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w27, [x20, x28, LSL #0x2]\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 46f\n"
      "ldr x20, [%x[input_ptr], x28, LSL #0x3]\n"
      "add x20, x20, x21, LSL #3\n"
      "ldr x26, [x20, #0x0]\n"
      "ldr x25, [x20, #0x8]\n"
      "ldr x24, [x20, #0x10]\n"
      "ldr x23, [x20, #0x18]\n"
      "ldr x22, [x20, #0x20]\n"
      "cbnz x28, 47f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "add x23, x23, x20\n"
      "add x22, x22, x20\n"
      "b 47f\n"
      "46:"  // Height 5: setup direct input
      "mov x26, %x[input_ptr]\n"
      "add x25, x26, x21\n"
      "add x24, x25, x21\n"
      "add x23, x24, x21\n"
      "add x22, x23, x21\n"
      "47:"  // Height 5: input setup done
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1rw { z3.s }, p4/Z, [x23]\n"
      "ld1rw { z4.s }, p4/Z, [x22]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "ble 49f\n"
      "48:"  // Height 5: Multiply loop: Main loop
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x26, x26, #0x4\n"
      "subs x27, x27, #0x4\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z20.s, z6.b, z3.b\n"
      "add x25, x25, #0x4\n"
      "add x24, x24, #0x4\n"
      "sdot z24.s, z6.b, z4.b\n"
      "sdot z9.s, z7.b, z0.b\n"
      "ld1b { z29.b }, p4/Z, [x10, #2, MUL VL]\n"
      "add x23, x23, #0x4\n"
      "sdot z13.s, z7.b, z1.b\n"
      "sdot z17.s, z7.b, z2.b\n"
      "add x22, x22, #0x4\n"
      "sdot z21.s, z7.b, z3.b\n"
      "sdot z25.s, z7.b, z4.b\n"
      "ld1b { z28.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z29.b, z0.b\n"
      "sdot z14.s, z29.b, z1.b\n"
      "sdot z18.s, z29.b, z2.b\n"
      "sdot z22.s, z29.b, z3.b\n"
      "sdot z26.s, z29.b, z4.b\n"
      "sdot z11.s, z28.b, z0.b\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "sdot z15.s, z28.b, z1.b\n"
      "sdot z19.s, z28.b, z2.b\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "sdot z23.s, z28.b, z3.b\n"
      "sdot z27.s, z28.b, z4.b\n"
      "ld1rw { z3.s }, p4/Z, [x23]\n"
      "ld1rw { z4.s }, p4/Z, [x22]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "bgt 48b\n"
      "49:"  // Height 5: Multiply loop: Main loop skip
      "ldr w20, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x28, x28, #0x1\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z20.s, z6.b, z3.b\n"
      "cmp x28, x20\n"
      "sdot z24.s, z6.b, z4.b\n"
      "sdot z9.s, z7.b, z0.b\n"
      "ld1b { z29.b }, p4/Z, [x10, #2, MUL VL]\n"
      "sdot z13.s, z7.b, z1.b\n"
      "sdot z17.s, z7.b, z2.b\n"
      "sdot z21.s, z7.b, z3.b\n"
      "sdot z25.s, z7.b, z4.b\n"
      "ld1b { z28.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z29.b, z0.b\n"
      "sdot z14.s, z29.b, z1.b\n"
      "sdot z18.s, z29.b, z2.b\n"
      "sdot z22.s, z29.b, z3.b\n"
      "sdot z26.s, z29.b, z4.b\n"
      "sdot z11.s, z28.b, z0.b\n"
      "sdot z15.s, z28.b, z1.b\n"
      "sdot z19.s, z28.b, z2.b\n"
      "sdot z23.s, z28.b, z3.b\n"
      "sdot z27.s, z28.b, z4.b\n"
      "bne 45b\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x23, x9, x20, LSL #2\n"
      "add x22, x23, x20, LSL #2\n"
      "st1w { z8.s }, p3, [x9]\n"
      "add x21, x22, x20, LSL #2\n"
      "add x20, x21, x20, LSL #2\n"
      "st1w { z9.s }, p2, [x9, #1, MUL VL]\n"
      "st1w { z10.s }, p1, [x9, #2, MUL VL]\n"
      "st1w { z11.s }, p0, [x9, #3, MUL VL]\n"
      "addvl x9, x9, #4\n"
      "st1w { z12.s }, p3, [x23]\n"
      "st1w { z13.s }, p2, [x23, #1, MUL VL]\n"
      "st1w { z14.s }, p1, [x23, #2, MUL VL]\n"
      "st1w { z15.s }, p0, [x23, #3, MUL VL]\n"
      "st1w { z16.s }, p3, [x22]\n"
      "st1w { z17.s }, p2, [x22, #1, MUL VL]\n"
      "st1w { z18.s }, p1, [x22, #2, MUL VL]\n"
      "st1w { z19.s }, p0, [x22, #3, MUL VL]\n"
      "st1w { z20.s }, p3, [x21]\n"
      "st1w { z21.s }, p2, [x21, #1, MUL VL]\n"
      "st1w { z22.s }, p1, [x21, #2, MUL VL]\n"
      "st1w { z23.s }, p0, [x21, #3, MUL VL]\n"
      "st1w { z24.s }, p3, [x20]\n"
      "st1w { z25.s }, p2, [x20, #1, MUL VL]\n"
      "st1w { z26.s }, p1, [x20, #2, MUL VL]\n"
      "st1w { z27.s }, p0, [x20, #3, MUL VL]\n"
      "50:"  // Height 5: Writeback done
      "decw x11, ALL, MUL #4\n"
      "cmp x11, XZR\n"
      "bgt 42b\n"
      "b 62f\n"
      "51:"  // Height 6
      "ldr x21, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "mov x20, #0x18\n"
      "ldr x11, [%x[args_ptr], %[offsetof_N]]\n"
      "mov x9, %x[output_ptr]\n"
      "ldr x10, [%x[args_ptr], %[offsetof_B_ptr]]\n"
      "madd %x[output_ptr], x21, x20, %x[output_ptr]\n"
      "52:"  // Height 6: Column loop
      "mov x20, #0x0\n"
      "whilelt p3.s, x20, x11\n"
      "incw x20\n"
      "whilelt p2.s, x20, x11\n"
      "incw x20\n"
      "whilelt p1.s, x20, x11\n"
      "incw x20\n"
      "whilelt p0.s, x20, x11\n"
      "tbz %x[flags], #0, 53f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x24, x9, x20, LSL #2\n"
      "add x23, x24, x20, LSL #2\n"
      "ld1w { z8.s }, p3/Z, [x9]\n"
      "add x22, x23, x20, LSL #2\n"
      "add x21, x22, x20, LSL #2\n"
      "ld1w { z9.s }, p2/Z, [x9, #1, MUL VL]\n"
      "ld1w { z10.s }, p1/Z, [x9, #2, MUL VL]\n"
      "add x20, x21, x20, LSL #2\n"
      "ld1w { z11.s }, p0/Z, [x9, #3, MUL VL]\n"
      "ld1w { z12.s }, p3/Z, [x24]\n"
      "ld1w { z13.s }, p2/Z, [x24, #1, MUL VL]\n"
      "ld1w { z14.s }, p1/Z, [x24, #2, MUL VL]\n"
      "ld1w { z15.s }, p0/Z, [x24, #3, MUL VL]\n"
      "ld1w { z16.s }, p3/Z, [x23]\n"
      "ld1w { z17.s }, p2/Z, [x23, #1, MUL VL]\n"
      "ld1w { z18.s }, p1/Z, [x23, #2, MUL VL]\n"
      "ld1w { z19.s }, p0/Z, [x23, #3, MUL VL]\n"
      "ld1w { z20.s }, p3/Z, [x22]\n"
      "ld1w { z21.s }, p2/Z, [x22, #1, MUL VL]\n"
      "ld1w { z22.s }, p1/Z, [x22, #2, MUL VL]\n"
      "ld1w { z23.s }, p0/Z, [x22, #3, MUL VL]\n"
      "ld1w { z24.s }, p3/Z, [x21]\n"
      "ld1w { z25.s }, p2/Z, [x21, #1, MUL VL]\n"
      "ld1w { z26.s }, p1/Z, [x21, #2, MUL VL]\n"
      "ld1w { z27.s }, p0/Z, [x21, #3, MUL VL]\n"
      "ld1w { z28.s }, p3/Z, [x20]\n"
      "ld1w { z29.s }, p2/Z, [x20, #1, MUL VL]\n"
      "ld1w { z30.s }, p1/Z, [x20, #2, MUL VL]\n"
      "ld1w { z31.s }, p0/Z, [x20, #3, MUL VL]\n"
      "b 54f\n"
      "53:"  // Height 6: no accumulate
      "mov z8.s, #0x0\n"
      "mov z9.s, #0x0\n"
      "mov z10.s, #0x0\n"
      "mov z11.s, #0x0\n"
      "mov z12.s, #0x0\n"
      "mov z13.s, #0x0\n"
      "mov z14.s, #0x0\n"
      "mov z15.s, #0x0\n"
      "mov z16.s, #0x0\n"
      "mov z17.s, #0x0\n"
      "mov z18.s, #0x0\n"
      "mov z19.s, #0x0\n"
      "mov z20.s, #0x0\n"
      "mov z21.s, #0x0\n"
      "mov z22.s, #0x0\n"
      "mov z23.s, #0x0\n"
      "mov z24.s, #0x0\n"
      "mov z25.s, #0x0\n"
      "mov z26.s, #0x0\n"
      "mov z27.s, #0x0\n"
      "mov z28.s, #0x0\n"
      "mov z29.s, #0x0\n"
      "mov z30.s, #0x0\n"
      "mov z31.s, #0x0\n"
      "54:"  // Height 6: setup done
      "mov x28, #0x0\n"
      "55:"  // Height 6: String loop
      "ldr x20, [%x[args_ptr], %[offsetof_string_lengths]]\n"
      "ldr w27, [x20, x28, LSL #0x2]\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 56f\n"
      "ldr x20, [%x[input_ptr], x28, LSL #0x3]\n"
      "add x20, x20, x21, LSL #3\n"
      "ldr x26, [x20, #0x0]\n"
      "ldr x25, [x20, #0x8]\n"
      "ldr x24, [x20, #0x10]\n"
      "ldr x23, [x20, #0x18]\n"
      "ldr x22, [x20, #0x20]\n"
      "ldr x21, [x20, #0x28]\n"
      "cbnz x28, 57f\n"
      "ldr x20, [%x[args_ptr], %[offsetof_input_initial_col]]\n"
      "add x26, x26, x20\n"
      "add x25, x25, x20\n"
      "add x24, x24, x20\n"
      "add x23, x23, x20\n"
      "add x22, x22, x20\n"
      "add x21, x21, x20\n"
      "b 57f\n"
      "56:"  // Height 6: setup direct input
      "mov x26, %x[input_ptr]\n"
      "add x25, x26, x21\n"
      "add x24, x25, x21\n"
      "add x23, x24, x21\n"
      "add x22, x23, x21\n"
      "add x21, x22, x21\n"
      "57:"  // Height 6: input setup done
      "subs x27, x27, #0x4\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1rw { z3.s }, p4/Z, [x23]\n"
      "ld1rw { z4.s }, p4/Z, [x22]\n"
      "ld1rw { z5.s }, p4/Z, [x21]\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "ble 59f\n"
      "58:"  // Height 6: Multiply loop: Main loop
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x26, x26, #0x4\n"
      "subs x27, x27, #0x4\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z20.s, z6.b, z3.b\n"
      "add x25, x25, #0x4\n"
      "add x24, x24, #0x4\n"
      "sdot z24.s, z6.b, z4.b\n"
      "sdot z28.s, z6.b, z5.b\n"
      "ld1b { z6.b }, p4/Z, [x10, #2, MUL VL]\n"
      "add x23, x23, #0x4\n"
      "sdot z9.s, z7.b, z0.b\n"
      "sdot z13.s, z7.b, z1.b\n"
      "add x22, x22, #0x4\n"
      "add x21, x21, #0x4\n"
      "sdot z17.s, z7.b, z2.b\n"
      "sdot z21.s, z7.b, z3.b\n"
      "sdot z25.s, z7.b, z4.b\n"
      "sdot z29.s, z7.b, z5.b\n"
      "ld1b { z7.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z6.b, z0.b\n"
      "sdot z14.s, z6.b, z1.b\n"
      "sdot z18.s, z6.b, z2.b\n"
      "sdot z22.s, z6.b, z3.b\n"
      "sdot z26.s, z6.b, z4.b\n"
      "sdot z30.s, z6.b, z5.b\n"
      "ld1b { z6.b }, p4/Z, [x10]\n"
      "sdot z11.s, z7.b, z0.b\n"
      "sdot z15.s, z7.b, z1.b\n"
      "ld1rw { z0.s }, p4/Z, [x26]\n"
      "ld1rw { z1.s }, p4/Z, [x25]\n"
      "sdot z19.s, z7.b, z2.b\n"
      "sdot z23.s, z7.b, z3.b\n"
      "ld1rw { z2.s }, p4/Z, [x24]\n"
      "ld1rw { z3.s }, p4/Z, [x23]\n"
      "sdot z27.s, z7.b, z4.b\n"
      "sdot z31.s, z7.b, z5.b\n"
      "ld1rw { z4.s }, p4/Z, [x22]\n"
      "ld1rw { z5.s }, p4/Z, [x21]\n"
      "ld1b { z7.b }, p4/Z, [x10, #1, MUL VL]\n"
      "bgt 58b\n"
      "59:"  // Height 6: Multiply loop: Main loop skip
      "ldr w20, [%x[args_ptr], %[offsetof_num_strings]]\n"
      "sdot z8.s, z6.b, z0.b\n"
      "sdot z12.s, z6.b, z1.b\n"
      "add x28, x28, #0x1\n"
      "sdot z16.s, z6.b, z2.b\n"
      "sdot z20.s, z6.b, z3.b\n"
      "cmp x28, x20\n"
      "sdot z24.s, z6.b, z4.b\n"
      "sdot z28.s, z6.b, z5.b\n"
      "ld1b { z6.b }, p4/Z, [x10, #2, MUL VL]\n"
      "sdot z9.s, z7.b, z0.b\n"
      "sdot z13.s, z7.b, z1.b\n"
      "sdot z17.s, z7.b, z2.b\n"
      "sdot z21.s, z7.b, z3.b\n"
      "sdot z25.s, z7.b, z4.b\n"
      "sdot z29.s, z7.b, z5.b\n"
      "ld1b { z7.b }, p4/Z, [x10, #3, MUL VL]\n"
      "addvl x10, x10, #4\n"
      "sdot z10.s, z6.b, z0.b\n"
      "sdot z14.s, z6.b, z1.b\n"
      "sdot z18.s, z6.b, z2.b\n"
      "sdot z22.s, z6.b, z3.b\n"
      "sdot z26.s, z6.b, z4.b\n"
      "sdot z30.s, z6.b, z5.b\n"
      "sdot z11.s, z7.b, z0.b\n"
      "sdot z15.s, z7.b, z1.b\n"
      "sdot z19.s, z7.b, z2.b\n"
      "sdot z23.s, z7.b, z3.b\n"
      "sdot z27.s, z7.b, z4.b\n"
      "sdot z31.s, z7.b, z5.b\n"
      "bne 55b\n"
      "ldr x20, [%x[args_ptr], %[offsetof_output_offset]]\n"
      "add x24, x9, x20, LSL #2\n"
      "add x23, x24, x20, LSL #2\n"
      "st1w { z8.s }, p3, [x9]\n"
      "add x22, x23, x20, LSL #2\n"
      "add x21, x22, x20, LSL #2\n"
      "st1w { z9.s }, p2, [x9, #1, MUL VL]\n"
      "add x20, x21, x20, LSL #2\n"
      "st1w { z10.s }, p1, [x9, #2, MUL VL]\n"
      "st1w { z11.s }, p0, [x9, #3, MUL VL]\n"
      "addvl x9, x9, #4\n"
      "st1w { z12.s }, p3, [x24]\n"
      "st1w { z13.s }, p2, [x24, #1, MUL VL]\n"
      "st1w { z14.s }, p1, [x24, #2, MUL VL]\n"
      "st1w { z15.s }, p0, [x24, #3, MUL VL]\n"
      "st1w { z16.s }, p3, [x23]\n"
      "st1w { z17.s }, p2, [x23, #1, MUL VL]\n"
      "st1w { z18.s }, p1, [x23, #2, MUL VL]\n"
      "st1w { z19.s }, p0, [x23, #3, MUL VL]\n"
      "st1w { z20.s }, p3, [x22]\n"
      "st1w { z21.s }, p2, [x22, #1, MUL VL]\n"
      "st1w { z22.s }, p1, [x22, #2, MUL VL]\n"
      "st1w { z23.s }, p0, [x22, #3, MUL VL]\n"
      "st1w { z24.s }, p3, [x21]\n"
      "st1w { z25.s }, p2, [x21, #1, MUL VL]\n"
      "st1w { z26.s }, p1, [x21, #2, MUL VL]\n"
      "st1w { z27.s }, p0, [x21, #3, MUL VL]\n"
      "st1w { z28.s }, p3, [x20]\n"
      "st1w { z29.s }, p2, [x20, #1, MUL VL]\n"
      "st1w { z30.s }, p1, [x20, #2, MUL VL]\n"
      "st1w { z31.s }, p0, [x20, #3, MUL VL]\n"
      "60:"  // Height 6: Writeback done
      "decw x11, ALL, MUL #4\n"
      "cmp x11, XZR\n"
      "bgt 52b\n"
      "subs %x[M], %x[M], #0x6\n"
      "beq 62f\n"
      "ldr x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "tbz %x[flags], #3, 61f\n"
      "add x21, x21, #0x6\n"
      "str x21, [%x[args_ptr], %[offsetof_input_offset]]\n"
      "b 1b\n"
      "61:"  // Update direct input
      "mov x20, #0x6\n"
      "madd %x[input_ptr], x20, x21, %x[input_ptr]\n"
      "b 1b\n"
      "62:"  // Exit
      : [M] "+&r" (M), [input_ptr] "+&r" (input_ptr), [output_ptr] "+&r" (output_ptr)
      : [args_ptr] "r" (&ka), [flags] "r" (flags), [offsetof_B_ptr] "I" (offsetof(KernelArgs, B_ptr)), [offsetof_N] "I" (offsetof(KernelArgs, N)), [offsetof_input_initial_col] "I" (offsetof(KernelArgs, input_initial_col)), [offsetof_input_offset] "I" (offsetof(KernelArgs, input_offset)), [offsetof_num_strings] "I" (offsetof(KernelArgs, num_strings)), [offsetof_output_offset] "I" (offsetof(KernelArgs, output_offset)), [offsetof_string_lengths] "I" (offsetof(KernelArgs, string_lengths))
      : "cc", "memory", "p0", "p1", "p2", "p3", "p4", "x9", "x10", "x11", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
    );
}

} // namespace arm_gemm
#endif  // ARM_COMPUTE_ENABLE_SVE