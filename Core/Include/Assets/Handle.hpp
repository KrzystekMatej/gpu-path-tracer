#pragma once
#include <compare>
#include "Assets/Key.hpp"

namespace Core::Assets
{
    template<Asset T>
    class Handle
    {
    public:
        friend bool operator==(Handle, Handle) = default;

    private:
        explicit constexpr Handle(AssetId id) : m_Id(id) {}
        friend class Storage;

        AssetId m_Id;
    };
}