#pragma once
#include <compare>
#include <Core/Assets/Key.hpp>

namespace Core::Assets
{
    template<Asset T>
    class Handle
    {
    public:
		Utils::Guid GetId() const { return m_Id; }
        friend bool operator==(Handle, Handle) = default;
    private:
        explicit constexpr Handle(Utils::Guid id) : m_Id(id) {}
        friend class Storage;

        Utils::Guid m_Id;
    };
}