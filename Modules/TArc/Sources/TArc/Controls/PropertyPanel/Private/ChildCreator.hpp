#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.hpp"

#include "Functional/Signal.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class ChildCreator
{
public:
    ChildCreator();
    ~ChildCreator();

    std::shared_ptr<const PropertyNode> CreateRoot(const Reflection& reflectedRoot);
    void UpdateSubTree(const std::shared_ptr<const PropertyNode>& parent);
    void RemoveNode(const std::shared_ptr<const PropertyNode>& parent);
    void Clear();

    void RegisterExtension(const std::shared_ptr<ChildCreatorExtension>& extension);
    void UnregisterExtension(const std::shared_ptr<ChildCreatorExtension>& extension);

    Signal<std::shared_ptr<const PropertyNode> /*parent*/, std::shared_ptr<const PropertyNode> /*child*/, size_t /*childPosition*/> nodeCreated;
    Signal<std::shared_ptr<const PropertyNode> /*child*/> nodeRemoved;

private:
    std::shared_ptr<ChildCreatorExtension> extensions;
    std::unordered_map<std::shared_ptr<const PropertyNode>, std::vector<std::shared_ptr<const PropertyNode>>> propertiesIndex;
    std::shared_ptr<IChildAllocator> allocator;
};

} // namespace TArc
} // namespace DAVA