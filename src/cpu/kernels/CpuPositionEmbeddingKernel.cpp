#include "src/cpu/kernels/CpuPositionEmbeddingKernel.h"

#include "arm_compute/core/Error.h"
#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/ITensor.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/core/utils/misc/ShapeCalculator.h"
#include "arm_compute/core/Validate.h"

#include "src/core/helpers/AutoConfiguration.h"
#include "src/core/helpers/WindowHelpers.h"

#include <cmath>

namespace arm_compute
{
namespace cpu
{
namespace kernels
{

namespace
{
/**  Vectorize pretrained position embedding*/
template <typename T>
void run_positional_encoding(const Window &window, ITensor *src, ITensor *vector, ITensor *dst, const unsigned int d_model)
{
    std::cout << "src/cpu/kernels/CpuPositionEmbeddingKernel.cpp" << std::endl;
    ARM_COMPUTE_UNUSED(src);

    Window win = window;
    win.set(Window::DimX, Window::Dimension(0,1,1));
    win.set(Window::DimY, Window::Dimension(0,1,1));
    const unsigned int window_start_x   = static_cast<unsigned int>(window.x().start());
    const unsigned int window_end_x     = static_cast<unsigned int>(window.x().end());

    const unsigned int vector_depth     = vector->info()->tensor_shape().y();


    ARM_COMPUTE_UNUSED(win);
    ARM_COMPUTE_UNUSED(dst);

    std::cout << "window " << window_start_x  << " " <<   window_end_x  << std::endl;
    std::cout << "src " << src->info()->tensor_shape().x()  << " " <<   src->info()->tensor_shape().y()  << std::endl;
    std::cout << "vector " << vector->info()->tensor_shape().x()  << " " <<   vector->info()->tensor_shape().y()  << std::endl;
    std::cout << "dst " << dst->info()->tensor_shape().x()  << " " <<   dst->info()->tensor_shape().y()  << std::endl;
    std::cout << vector_depth << std::endl;

    unsigned int offset_vector,offset_dst;

    Iterator dst_iter(dst,win);
    Iterator vector_iter(vector,win);

    const auto dst_ptr      = reinterpret_cast<float *>(dst_iter.ptr());
    const auto vector_ptr   = reinterpret_cast<float *>(vector_iter.ptr());

    execute_window_loop(win,
        [&](const Coordinates &)
        {
            for(unsigned int x = window_start_x; x < window_end_x; x++)
            {
                offset_dst     = x * vector_depth;
                offset_vector  = x * vector_depth;
                std::memcpy(dst_ptr + offset_dst, vector_ptr + offset_vector, (vector_depth) * sizeof(*vector_ptr));
                std::cout << *(src->buffer()+x) << "  ";
                std::cout << *(dst_ptr + offset_dst) << "  ";
                std::cout << *(dst_ptr + offset_dst + dst->info()->tensor_shape().y()-1) << std::endl;
                
            }
        }, src_iter);

}

}

void CpuPositionEmbeddingKernel::configure(const ITensorInfo *src, const ITensorInfo *pos, ITensorInfo *dst, const unsigned int d_model)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src, dst);

    _d_model = d_model;

    // Configure output tensor info.
    auto_init_if_empty(*dst, TensorInfo(*pos->clone()));

    // Configure kernel window
    Window win = calculate_max_window(*pos, Steps());
    ICpuKernel::configure(win);
}


Status CpuPositionEmbeddingKernel::validate(const ITensorInfo *src, const ITensorInfo *pos, const ITensorInfo *dst, const unsigned int d_model)
{
    ARM_COMPUTE_UNUSED(pos);
    ARM_COMPUTE_UNUSED(src);
    ARM_COMPUTE_UNUSED(dst);
    ARM_COMPUTE_UNUSED(d_model);

    return Status{};
}

void CpuPositionEmbeddingKernel::run_op(ITensorPack &tensors, const Window &window, const ThreadInfo &info)
{
    ARM_COMPUTE_UNUSED(info);

    auto src = tensors.get_tensor(TensorType::ACL_SRC_0);
    auto pos = tensors.get_tensor(TensorType::ACL_SRC_1);
    auto dst = tensors.get_tensor(TensorType::ACL_DST);

    run_positional_encoding<float>(window, src, pos, dst, _d_model);
}

const char * CpuPositionEmbeddingKernel::name() const
{
    return "CpuPositionEmbeddingKernel";
}

} // namespace kernels
} // namespace cpu
} // namespace arm_compute