// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef BASE_OBJECT_H
#define BASE_OBJECT_H

#include <sstream>
#include <stdexcept>
#include <string>

class BaseObject
{
public:
    virtual ~BaseObject() = default;
    std::string GetName() const
    {
        return this->m_name;
    }

    std::string GetFullName() const
    {
        std::string parentFullName = (this->m_parent == nullptr ? "" : this->m_parent->GetFullName());
        return parentFullName.append(".").append(this->m_name);
    }

    static bool NameIsValid(const std::string& name)
    {
        // A name cannot be empty, cannot contain periods, and cannot contain whitespace
        return (!name.empty() && name.find_first_of(". \t\r\n") == name.npos);
    }

protected:
    explicit BaseObject(const std::string& name, BaseObject* parent = nullptr) :
        m_name(name),
        m_parent(parent)
    {
    }

    void ValidateName()
    {
        if (!NameIsValid(this->m_name))
        {
            throw std::invalid_argument("A name cannot be empty, cannot contain periods, and cannot contain whitespace");
        }
    }

    BaseObject* GetParent() const
    {
        return this->m_parent;
    }

    void SetParent(BaseObject* parent)
    {
        if (this->m_parent == nullptr)
        {
            this->m_parent = parent;
        }
        else
        {
            std::ostringstream exceptionMessage;
            exceptionMessage << "Object '" << this->GetFullName() << "' already has a parent";
            throw std::invalid_argument(exceptionMessage.str());
        }
    }

private:
    const std::string m_name;
    BaseObject* m_parent;
};

#endif // BASE_OBJECT_H
