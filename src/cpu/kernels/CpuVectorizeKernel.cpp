#include "src/cpu/kernels/CpuVectorizeKernel.h"

#include "arm_compute/core/ITensor.h"
#include "arm_compute/core/TensorInfo.h"
#include "arm_compute/core/Utils.h"

#include "src/core/common/Registrars.h"
#include "src/core/CPP/Validate.h"
#include "src/core/helpers/AutoConfiguration.h"
#include "src/core/helpers/WindowHelpers.h"
#include "src/cpu/kernels/vectorize/list.h"

namespace arm_compute
{
namespace cpu
{
namespace kernels
{

namespace
{
static const std::vector<CpuVectorizeKernel::VectorizeKernel> available_kernels = {
    /*
#ifdef ARM_COMPUTE_ENABLE_SVE
    // TBA
#endif // ARM_COMPUTE_ENABLE_SVE

#ifdef __aarch64__
    // TBA
#endif // __aarch64__
*/
    {"neon_vectorize_int_2_float32", [](const VectorizeKernelDataTypeISASelectorData &data) { return data.dt == DataType::F32; },
     REGISTER_FP32_NEON(arm_compute::cpu::neon_vectorize_int_2_float32)},

};
}

void CpuVectorizeKernel::configure(const ITensorInfo *src, const ITensorInfo *vector, ITensorInfo *dst)
{
    ARM_COMPUTE_ERROR_ON_NULLPTR(src);
    ARM_COMPUTE_ERROR_ON_NULLPTR(vector);
    
    const auto uk = CpuVectorizeKernel::get_implementation(
        VectorizeKernelDataTypeISASelectorData{dst->data_type(), CPUInfo::get().get_isa()}
    );
    std::cout << "src/cpu/kernels/CpuVectorizeKernel.cpp configure: " << std::endl;
    std::cout << "src:" << std::endl;
    for(auto v:src->tensor_shape())std::cout << v << " ";
    std::cout << std::endl;

    std::cout << "vector:" << std::endl;
    for(auto v:vector->tensor_shape())std::cout << v << " ";
    std::cout << std::endl;

    // Configure output tensor info.
    const TensorShape dst_shape(src->tensor_shape().x(),vector->tensor_shape().y());
    if (dst->tensor_shape().total_size() == 0)
    {
        auto_init_if_empty(*dst, TensorInfo(*vector->clone()).set_tensor_shape(dst_shape));
    }
    else
    {
        dst->set_tensor_shape(dst_shape);
    }

    ARM_COMPUTE_ERROR_ON_NULLPTR(uk);

    _run_method = uk->ukernel;
    _name       = std::string("CpuVectorizeKernel").append("/").append(uk->name);

    Window win;

    win = calculate_max_window(*dst, Steps());
    ICPPKernel::configure(win);
    
}

Status CpuVectorizeKernel::validate(const ITensorInfo *src,  const ITensorInfo *vector, ITensorInfo *dst)
{
    ARM_COMPUTE_UNUSED(src);
    ARM_COMPUTE_UNUSED(vector);
    ARM_COMPUTE_UNUSED(dst);
    return Status{};
}


size_t CpuVectorizeKernel::get_mws(const CPUInfo &platform, size_t thread_count) const
{
    ARM_COMPUTE_UNUSED(thread_count);
    ARM_COMPUTE_UNUSED(platform);

    if (_split_dimension == Window::DimX)
    {
        // Don't split the work load too small if the tensor has been reinterpreted as 1D.
        // This number is loosely chosen as threading overhead in each platform varies wildly.
        return 1536;
    }
    return default_mws;
}

void CpuVectorizeKernel::run_op(ITensorPack &tensors, const Window &window, const ThreadInfo &info)
{

    ARM_COMPUTE_UNUSED(info);
    ARM_COMPUTE_ERROR_ON_UNCONFIGURED_KERNEL(this);
    ARM_COMPUTE_ERROR_ON_INVALID_SUBWINDOW(IKernel::window(), window);

    ARM_COMPUTE_ERROR_ON(tensors.empty());
    ARM_COMPUTE_ERROR_ON(_run_method == nullptr);

    const ITensor *src      = tensors.get_const_tensor(TensorType::ACL_SRC_0);
    const ITensor *vector   = tensors.get_const_tensor(TensorType::ACL_SRC_1);
    ITensor       *dst      = tensors.get_tensor(TensorType::ACL_DST);


    std::cout << "src/cpu/kernels/CpuTokenEmbedKernel.cpp reshaped" << std::endl;

    size_t reshape_input_x = src->info()->valid_region().shape.x();
    /* Runtime input reshape */
    if(src->info()->tensor_shape().x() != reshape_input_x)
    {
        dst->info()->set_valid_region(dst->info()->valid_region().set(0,0,reshape_input_x));
        std::cout << "dst: "<< dst->info()->valid_region().shape.x() << std::endl;

    }

    _run_method(src, vector, dst, window);
}

const char *CpuVectorizeKernel::name() const
{
    return _name.c_str();
}

const std::vector<CpuVectorizeKernel::VectorizeKernel> &CpuVectorizeKernel::get_available_kernels()
{
    return available_kernels;
}

} // namespace kernels
} // namespace cpu
} // namespace arm_compute