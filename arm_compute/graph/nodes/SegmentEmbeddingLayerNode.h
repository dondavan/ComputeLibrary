#ifndef ARM_COMPUTE_GRAPH_SEGMENTEMBEDDINGLAYERNODE_H
#define ARM_COMPUTE_GRAPH_SEGMENTEMBEDDINGLAYERNODE_H

#include "arm_compute/graph/INode.h"

namespace arm_compute
{
namespace graph
{
/** Segment embedding Node */
class SegmentEmbeddingLayerNode final : public INode
{
public:
    /** Constructor  */
    SegmentEmbeddingLayerNode();

    // Inherited overridden methods:
    NodeType         type() const override;
    bool             forward_descriptors() override;
    TensorDescriptor configure_output(size_t idx) const override;
    void             accept(INodeVisitor &v) override;

private:
};
} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_SEGMENTEMBEDDINGLAYERNODE_H */