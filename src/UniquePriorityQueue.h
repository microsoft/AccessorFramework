// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef UNIQUE_PRIORITY_QUEUE_H
#define UNIQUE_PRIORITY_QUEUE_H

#include <queue>
#include <set>
#include <vector>

// Description
// A priority_queue<T> that contains at most one instance of a given element
//
template<class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>>
class unique_priority_queue
{
public:
    T top() const
    {
        return this->m_queue.top();
    }

    void push(T newElement)
    {
        if (this->m_elementsInQueue.find(newElement) == this->m_elementsInQueue.end())
        {
            this->m_queue.push(newElement);
            this->m_elementsInQueue.insert(newElement);
        }
    }

    void pop()
    {
        this->m_elementsInQueue.erase(this->m_elementsInQueue.find(this->top()));
        this->m_queue.pop();
    }

    bool empty() const
    {
        return this->m_queue.empty();
    }

private:
    std::priority_queue<T, Container, Compare> m_queue;
    std::set<T> m_elementsInQueue;
};

#endif // UNIQUE_PRIORITY_QUEUE_H
