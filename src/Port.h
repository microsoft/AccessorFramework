// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef PORT_H
#define PORT_H

#include <queue>
#include <set>
#include <vector>
#include <AccessorFramework/Accessor.h>
#include <AccessorFramework/Event.h>
#include "BaseObject.h"

// Description
// A port sends and receives events. A port that sends an event is called a source, and a port that receives an event is
// called a destination. Despite their names, both input and output ports can send and receive events; the names imply
// where the port can send the event. An input port can receive an event from an output port on the same accessor (i.e.
// feedback loop), an output port on a peer accessor, or an input port on a parent accessor. A connected output port can
// receive events from an input port on the same accessor. A spontaneous output port cannot have a source; a spontaneous
// output is produced without any prompting from an input. For example, a timer that fires are regular intervals would be
// considered spontaneous output. Both connected and spontaneous output ports can send events to an input port on the
// same accessor (i.e. feedback loop), an input port on a peer accessor, or an output port on a parent accessor. All
// ports are given a name upon instantiation. The name of a port must be unique among that accessor's ports; no two ports
// on an accessor can have the same name.
//
class Port : public BaseObject
{
public:
    Port(const std::string& name, Accessor::Impl* owner);
    Accessor::Impl* GetOwner() const;
    virtual bool IsSpontaneous() const;
    bool IsConnectedToSource() const;
    const Port* GetSource() const;
    std::vector<const Port*> GetDestinations() const;

    void SendData(std::shared_ptr<IEvent> data);
    virtual void ReceiveData(std::shared_ptr<IEvent> data) = 0;

    static void Connect(Port* source, Port* destination);

private:
    static void ValidateConnection(Port* source, Port* destination);

    Port* m_source;
    std::vector<Port*> m_destinations;
};

class InputPort final : public Port
{
public:
    InputPort(const std::string& name, Accessor::Impl* owner);
    IEvent* GetLatestInput() const;
    std::shared_ptr<IEvent> ShareLatestInput() const;
    int GetInputQueueLength() const;
    bool IsWaitingForInputHandler() const;
    void DequeueLatestInput(); // should only be called by port's owner in AtomicAccessor::Impl::ProcessInputs()
    void ReceiveData(std::shared_ptr<IEvent> input) override;

private:
    void QueueInput(std::shared_ptr<IEvent> input);

    bool m_waitingForInputHandler;
    std::queue<std::shared_ptr<IEvent>> m_inputQueue;
};

class OutputPort final : public Port
{
public:
    OutputPort(const std::string& name, Accessor::Impl* owner, bool spontaneous);
    bool IsSpontaneous() const override;
    void ReceiveData(std::shared_ptr<IEvent> input) override;

private:
    const bool m_spontaneous;
};

#endif // PORT_H
