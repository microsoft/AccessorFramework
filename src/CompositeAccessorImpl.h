// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef COMPOSITE_ACCESSOR_IMPL_H
#define COMPOSITE_ACCESSOR_IMPL_H

#include "AccessorImpl.h"
#include "UniquePriorityQueue.h"

// Description
// The CompositeAccessor::Impl class implements the CompositeAccessor class defined in Accessor.h. In addition, it
// exposes additional functionality for internal use, such as public methods for getting the contained child accessors
// and scheduling reactions for its children. Children's reactions are handled by storing a pointer to the child
// requesting a reaction on a private queue and scheduling an immediate callback for invoking reactions for all children
// on the queue. This mechanism ensures that a child can only schedule one reaction at a time and ensures that children
// react in priority order.
//
class CompositeAccessor::Impl : public Accessor::Impl
{
public:
    explicit Impl(
        const std::string& name,
        CompositeAccessor* container,
        std::function<void(Accessor&)> initializeFunction,
        const std::vector<std::string>& inputPortNames = {},
        const std::vector<std::string>& connectedOutputPortNames = {});

    bool HasChildWithName(const std::string& childName) const;
    Accessor::Impl* GetChild(const std::string& childName) const;
    std::vector<Accessor::Impl*> GetChildren() const;
    void ScheduleReaction(Accessor::Impl* child, int priority);
    void ProcessChildEventQueue();

    void ResetPriority() override;
    bool IsComposite() const override;
    void Initialize() override;

protected:
    // CompositeAccessor Methods
    bool NewChildNameIsValid(const std::string& newChildName) const;
    void AddChild(std::unique_ptr<Accessor> child);
    void ConnectMyInputToChildInput(const std::string& myInputPortName, const std::string& childName, const std::string& childInputPortName);
    void ConnectChildOutputToMyOutput(const std::string& childName, const std::string& childOutputPortName, const std::string& myOutputPortName);
    void ConnectChildren(
        const std::string& sourceChildName,
        const std::string& sourceChildOutputPortName,
        const std::string& destinationChildName,
        const std::string& destinationChildInputPortName);
    virtual void ChildrenChanged();

    // Internal Methods
    void ResetChildrenPriorities() const;

private:
    friend class CompositeAccessor;

    // Returns true if accessor A has lower priority (i.e. higher priority value) than accessor B, and false otherwise.
    // When used in a priority queue, elements will be sorted from highest priority (i.e. lowest priority value) to
    // lowest priority (i.e. highest priority value).
    struct GreaterAccessorImplPtrs
    {
        bool operator()(Accessor::Impl* const a, Accessor::Impl* const b) const
        {
            if (a != nullptr && b != nullptr)
            {
                return (*a > *b);
            }
            else if (a != nullptr && b == nullptr)
            {
                return false;
            }
            else if (a == nullptr && b != nullptr)
            {
                return true;
            }
            else
            {
                // both are nullptr
                return false;
            }
        }
    };

    bool m_reactionRequested;
    std::map<std::string, std::unique_ptr<Accessor>> m_children;
    std::vector<Accessor::Impl*> m_orderedChildren;
    unique_priority_queue<Accessor::Impl*, std::vector<Accessor::Impl*>, GreaterAccessorImplPtrs> m_childEventQueue;
};

#endif // COMPOSITE_ACCESSOR_IMPL_H
