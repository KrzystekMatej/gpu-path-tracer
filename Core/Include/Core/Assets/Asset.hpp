#pragma once
#include <concepts>
#include <Core/Assets/Key.hpp>

namespace Core::Assets
{
    struct IAsset
    {
        virtual ~IAsset() = default;
    };

    template<class Derived, AssetType TypeValue>
    struct AssetTyped : IAsset
    {
        static constexpr AssetType Type = TypeValue;
    };

    template<class T>
    concept Asset =
        std::derived_from<T, IAsset> &&
        requires { { T::Type } -> std::convertible_to<AssetType>; };
}