#include "src/cpu/operators/CpuTokenEmbed.h"

#include "arm_compute/runtime/NEON/NEScheduler.h"

#include "src/common/IOperator.h"
#include "src/common/utils/LegacySupport.h"
#include "src/common/utils/Log.h"
#include "src/cpu/CpuContext.h"
#include "src/cpu/kernels/CpuTokenEmbedKernel.h"
#include "src/cpu/utils/CpuAuxTensorHandler.h"
#include "src/core/helpers/MemoryHelpers.h"

#include <memory>

using namespace arm_compute::experimental;

namespace arm_compute
{
namespace cpu
{
void CpuTokenEmbed::configure(const ITensorInfo *input, const ITensorInfo *vocab,  ITensorInfo *output, const TokenEmbeddingLayerInfo &tkemb_info)
{
    ARM_COMPUTE_LOG_PARAMS(input, output, tkemb_info);

    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp 0  " << std::endl;
    auto k = std::make_unique<kernels::CpuTokenEmbedKernel>();
    k->configure(input, vocab, output, tkemb_info);
    _kernel = std::move(k);

    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp -1  " << std::endl;
    _PE_kernel = std::make_unique<kernels::CpuPositionalEncodingKernel>();
    _PE_kernel->configure(vocab,output,tkemb_info.d_model());

    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp -2  " << std::endl;
    _aux_mem[Token2PositionalAuxTensorIdx] =
        MemoryInfo(offset_int_vec(Token2PositionalAuxTensorIdx), MemoryLifetime::Temporary, _tmp_t2p.total_size());
    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp -3  " << std::endl;
}

Status
CpuTokenEmbed::validate(const ITensorInfo *input, const ITensorInfo *vocab, const ITensorInfo *output,const TokenEmbeddingLayerInfo &tkemb_info)
{
    ARM_COMPUTE_UNUSED(input);
    ARM_COMPUTE_UNUSED(vocab);
    ARM_COMPUTE_UNUSED(output);
    ARM_COMPUTE_UNUSED(tkemb_info);
    return Status{};
}

void CpuTokenEmbed::run(ITensorPack &tensors)
{
    ARM_COMPUTE_ERROR_ON_MSG(tensors.empty(), "No inputs provided");
    auto split_dimension = static_cast<kernels::CpuTokenEmbedKernel *>(_kernel.get())->get_split_dimension_hint();

    const ITensor *src   = tensors.get_const_tensor(TensorType::ACL_SRC_0);
    const ITensor *vocab = tensors.get_const_tensor(TensorType::ACL_SRC_1);
    ITensor       *dst   = tensors.get_tensor(TensorType::ACL_DST);

    CpuAuxTensorHandler Token2PositionalTensorHandler(offset_int_vec(Token2PositionalAuxTensorIdx), _tmp_t2p, tensors, true);
    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp 1  " << std::endl;
    ITensorPack token_pack{{ACL_SRC_0, src}, {ACL_SRC_1, vocab}, {ACL_DST, Token2PositionalTensorHandler.get()}};
    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp 2  " << std::endl;
    NEScheduler::get().schedule_op(_kernel.get(), split_dimension, _kernel->window(), token_pack);
    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp 3  " << std::endl;
    ITensorPack positional_pack{{ACL_SRC_0, Token2PositionalTensorHandler.get()}, {ACL_DST, dst}};
    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp 4  " << std::endl;
    NEScheduler::get().schedule_op(_PE_kernel.get(),Window::DimY,_PE_kernel->window(), positional_pack);
    std::cout << "src/cpu/operators/CpuTokenEmbed.cpp 5  " << std::endl;
}


} // namespace cpu
} // namespace arm_compute
