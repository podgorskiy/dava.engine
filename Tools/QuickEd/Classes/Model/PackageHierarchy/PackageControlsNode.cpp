#include "PackageControlsNode.h"

#include "ControlNode.h"
#include "PackageVisitor.h"

#include "PackageNode.h"
#include "UI/UIPackage.h"
#include "UI/UIControl.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(PackageNode *_parent)
    : ControlsContainerNode(_parent)
{
}

PackageControlsNode::~PackageControlsNode()
{
    for (ControlNode *node : nodes)
        node->Release();
    nodes.clear();
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

void PackageControlsNode::Accept(PackageVisitor *visitor)
{
    visitor->VisitControls(this);
}

String PackageControlsNode::GetName() const
{
    return "Controls";
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

void PackageControlsNode::RefreshControlProperties()
{
    for (ControlNode *node : nodes)
        node->RefreshProperties();
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
