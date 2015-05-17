#include "PackageControlsNode.h"

#include "ControlNode.h"

#include "../PackageSerializer.h"
#include "PackageNode.h"
#include "PackageRef.h"
#include "UI/UIPackage.h"
#include "UI/UIControl.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(PackageNode *_parent, PackageRef *_packageRef)
    : ControlsContainerNode(_parent)
    , name("Controls")
    , packageRef(SafeRetain(_packageRef))
{
}

PackageControlsNode::~PackageControlsNode()
{
    for (ControlNode *node : nodes)
        node->Release();
    nodes.clear();
    
    SafeRelease(packageRef);
}

void PackageControlsNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == nullptr);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
}

void PackageControlsNode::InsertAtIndex(int index, ControlNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);

    nodes.insert(nodes.begin() + index, SafeRetain(node));
}

void PackageControlsNode::Remove(ControlNode *node)
{
    auto it = find(nodes.begin(), nodes.end(), node);
    if (it != nodes.end())
    {
        DVASSERT(node->GetParent() == this);
        node->SetParent(NULL);

        nodes.erase(it);
        SafeRelease(node);
    }
    else
    {
        DVASSERT(false);
    }
}

int PackageControlsNode::GetCount() const
{
    return (int) nodes.size();
}

ControlNode *PackageControlsNode::Get(int index) const
{
    return nodes[index];
}

String PackageControlsNode::GetName() const
{
    return name;
}

void PackageControlsNode::SetName(const DAVA::String &name)
{
    this->name = name;
}

PackageRef *PackageControlsNode::GetPackageRef() const
{
    return packageRef;
}

int PackageControlsNode::GetFlags() const
{
    return IsReadOnly() ? FLAG_READ_ONLY : 0;
}

bool PackageControlsNode::IsEditingSupported() const
{
    return false;
}

bool PackageControlsNode::IsInsertingSupported() const
{
    return !IsReadOnly();
}

bool PackageControlsNode::CanInsertControl(ControlNode *node, DAVA::int32 pos) const
{
    return !IsReadOnly();
}

bool PackageControlsNode::CanRemove() const
{
    return false;
}

bool PackageControlsNode::CanCopy() const
{
    return false;
}

ControlNode *PackageControlsNode::FindControlNodeByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetControl()->GetName() == name)
            return *it;
    }
    return NULL;
}

void PackageControlsNode::Serialize(PackageSerializer *serializer) const
{
    Serialize(serializer, nodes);
}

void PackageControlsNode::Serialize(PackageSerializer *serializer, const DAVA::Vector<ControlNode*> &nodes) const
{
    serializer->BeginArray("Controls");
    
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Serialize(serializer);
    
    serializer->EndArray();
}
