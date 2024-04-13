#include "src/cpu/operators/CpuPositionEmbed.h"

#include "arm_compute/runtime/NEON/NEScheduler.h"

#include "src/common/IOperator.h"
#include "src/common/utils/LegacySupport.h"
#include "src/common/utils/Log.h"
#include "src/cpu/CpuContext.h"
#include "src/cpu/kernels/CpuVectorizeKernel.h"


namespace arm_compute
{
namespace cpu
{
void CpuPositionEmbed::configure(const ITensorInfo *input, const ITensorInfo *position,  ITensorInfo *output)
{
    ARM_COMPUTE_LOG_PARAMS(input, output);

    auto k = std::make_unique<kernels::CpuVectorizeKernel>();
    k->configure(input, position, output);
    _kernel = std::move(k);

}

Status
CpuPositionEmbed::validate(const ITensorInfo *input, const ITensorInfo *position, const ITensorInfo *output)
{
    ARM_COMPUTE_UNUSED(input);
    ARM_COMPUTE_UNUSED(position);
    ARM_COMPUTE_UNUSED(output);
    return Status{};
}

void CpuPositionEmbed::run(ITensorPack &tensors)
{
    ARM_COMPUTE_ERROR_ON_MSG(tensors.empty(), "No inputs provided");
    auto split_dimension = static_cast<kernels::CpuVectorizeKernel *>(_kernel.get())->get_split_dimension_hint();

    ARM_COMPUTE_UNUSED(split_dimension);
    ARM_COMPUTE_UNUSED(tensors);
    std::cout << "src/cpu/operators/CpuPositionEmbed.cpp" << std::endl;


    NEScheduler::get().schedule_op(_kernel.get(), split_dimension, _kernel->window(), tensors);
}


} // namespace cpu
} // namespace arm_compute