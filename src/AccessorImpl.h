// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef ACCESSOR_IMPL_H
#define ACCESSOR_IMPL_H

#include "AccessorFramework/Accessor.h"
#include "Director.h"
#include "Port.h"

// Description
// The Accessor::Impl class implements the Accessor class defined in Accessor.h. In addition, it exposes additional
// functionality for internal use, such as public methods for getting the accessor's ports or parent objects.
//
class Accessor::Impl : public BaseObject
{
public:
    virtual ~Impl();
    void SetParent(CompositeAccessor::Impl* parent);
    int GetPriority() const;
    virtual void ResetPriority();
    virtual std::shared_ptr<Director> GetDirector() const;
    bool HasInputPorts() const;
    bool HasOutputPorts() const;
    InputPort* GetInputPort(const std::string& portName) const;
    OutputPort* GetOutputPort(const std::string& portName) const;
    std::vector<const InputPort*> GetInputPorts() const;
    std::vector<const OutputPort*> GetOutputPorts() const;
    bool operator<(const Accessor::Impl& other) const;
    bool operator>(const Accessor::Impl& other) const;

    virtual bool IsComposite() const = 0;
    virtual void Initialize() = 0;

    static const int DefaultAccessorPriority;

protected:
    // Accessor Methods
    int ScheduleCallback(std::function<void()> callback, int delayInMilliseconds, bool repeat);
    void ClearScheduledCallback(int callbackId);
    bool NewPortNameIsValid(const std::string& newPortName) const;
    virtual void AddInputPort(const std::string& portName);
    virtual void AddInputPorts(const std::vector<std::string>& portNames);
    virtual void AddOutputPort(const std::string& portName);
    virtual void AddOutputPorts(const std::vector<std::string>& portNames);
    void ConnectMyInputToMyOutput(const std::string& myInputPortName, const std::string& myOutputPortName);
    void ConnectMyOutputToMyInput(const std::string& myOutputPortName, const std::string& myInputPortName);
    IEvent* GetLatestInput(const std::string& inputPortName) const;
    void SendOutput(const std::string& outputPortName, std::shared_ptr<IEvent> output);

    // Internal Methods
    Impl(const std::string& name, const std::vector<std::string>& inputPortNames = {}, const std::vector<std::string>& connectedOutputPortNames = {});
    size_t GetNumberOfInputPorts() const;
    size_t GetNumberOfOutputPorts() const;
    std::vector<InputPort*> GetOrderedInputPorts() const;
    std::vector<OutputPort*> GetOrderedOutputPorts() const;
    bool HasInputPortWithName(const std::string& portName) const;
    bool HasOutputPortWithName(const std::string& portName) const;
    void AddOutputPort(const std::string& portName, bool isSpontaneous);

    int m_priority;

private:
    friend class Accessor;
    friend void InputPort::ReceiveData(std::shared_ptr<IEvent> input);

    void AlertNewInput(); // should only be called in InputPort::ReceiveData() by input ports belonging to this accessor
    void ValidatePortName(const std::string& portName) const;

    std::set<int> m_callbackIds;
    std::map<std::string, std::unique_ptr<InputPort>> m_inputPorts;
    std::vector<InputPort*> m_orderedInputPorts;
    std::map<std::string, std::unique_ptr<OutputPort>> m_outputPorts;
    std::vector<OutputPort*> m_orderedOutputPorts;
};

#endif // ACCESSOR_IMPL_H
