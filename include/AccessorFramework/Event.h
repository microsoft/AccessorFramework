// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef EVENT_H
#define EVENT_H

// Description
// An Event is a data structure that is passed between ports. It may or may not contain a payload.
//
class IEvent {};

template <class T>
class Event : public IEvent
{
public:
    Event(const T& payload) : payload(payload) {}
    const T payload;
};

#endif // EVENT_H
