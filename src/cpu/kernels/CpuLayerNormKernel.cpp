#include "src/cpu/kernels/CpuLayerNormKernel.h"

#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/Validate.h"

#include "src/common/utils/Validate.h"
#include "src/core/helpers/AutoConfiguration.h"
#include "src/core/helpers/WindowHelpers.h"

namespace arm_compute
{
namespace cpu
{
namespace kernels
{

namespace
{

    void layer_norm_fp32(const ITensor *src, ITensor *dst, const Window &window,float epsilon,
                                                                                float gamma,
                                                                                float beta)
    {
        const int  window_step_y  = 1;
        const auto window_start_y = static_cast<int>(window.y().start());
        const auto window_end_y   = static_cast<int>(window.y().end());

        Window win = window.collapse_if_possible(window, Window::DimZ);
        win.set(Window::DimY, Window::Dimension(0, 1, 1));

        Iterator input(src, win);
        Iterator output(dst, win);


        int count = 0;
        execute_window_loop(
        win,
        [&](const Coordinates &)
        {
            const auto input_ptr  = reinterpret_cast<const float *>(input.ptr());
            const auto output_ptr = reinterpret_cast<float *>(output.ptr());
            float mean = 0;
            float var = 0;
            count ++;

            int y = window_start_y;
            for (; y <= (window_end_y - window_step_y); y += window_step_y)
            {
                mean+= *(input_ptr + y);
                std::cout << *(input_ptr + y) << " ";
            }
            std::cout << std::endl;
            ARM_COMPUTE_UNUSED(epsilon);
            ARM_COMPUTE_UNUSED(input_ptr);
            ARM_COMPUTE_UNUSED(output_ptr);
            std::cout << window_end_y - window_step_y <<" " << count << " " << gamma << " " << beta<< std::endl;
        },
        input, output);

    }

}

void CpuLayerNormKernel::configure(const ITensorInfo *input,
                                    ITensorInfo       *output,
                                    LayerNormLayerInfo   info)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(input, output);
    ARM_COMPUTE_ERROR_THROW_ON(validate(input, output, info));

    _info = info;

    TensorShape out_shape = input->tensor_shape();
    // Auto initialize if empty
    set_shape_if_empty(*output, out_shape);
    set_data_type_if_unknown(*output, input->data_type());

    Window win = calculate_max_window(*input, Steps());
    ICPPKernel::configure(win);

}

Status CpuLayerNormKernel::validate(const ITensorInfo *input,
                                    const ITensorInfo *output,
                                    LayerNormLayerInfo   info)
{
    ARM_COMPUTE_UNUSED(input);
    ARM_COMPUTE_UNUSED(output);
    ARM_COMPUTE_UNUSED(info);
    return Status{};
}

void CpuLayerNormKernel::run_op(ITensorPack &tensors, const Window &window, const ThreadInfo &thread_info)
{
    std::cout << "src/cpu/kernels/CpuLayerNormKernel.cpp" << std::endl;
    ARM_COMPUTE_UNUSED(thread_info);
    const ITensor *src = tensors.get_const_tensor(TensorType::ACL_SRC);
    ITensor       *dst  = tensors.get_tensor(TensorType::ACL_DST);
    layer_norm_fp32(src,dst,window,_info.epsilon(),_info.gamma(),_info.beta());
}

const char *CpuLayerNormKernel::name() const
{
    return "NELayerNormLayerKernel";
}

} // namespace kernels
} // namespace cpu
} // namespace arm_compute
