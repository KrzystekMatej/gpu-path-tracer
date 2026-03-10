#pragma once
#include <cstdint>

namespace Core::Input
{
    enum class KeyCode : uint16_t
    {
        Unknown = 0,

        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,

        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        Semicolon,
        Equal,

        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        World1,
        World2,

        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,

        RightArrow,
        LeftArrow,
        DownArrow,
        UpArrow,

        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        F25,

        Keypad0,
        Keypad1,
        Keypad2,
        Keypad3,
        Keypad4,
        Keypad5,
        Keypad6,
        Keypad7,
        Keypad8,
        Keypad9,
        KeypadDecimal,
        KeypadDivide,
        KeypadMultiply,
        KeypadSubtract,
        KeypadAdd,
        KeypadEnter,
        KeypadEqual,

        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper,
        RightShift,
        RightControl,
        RightAlt,
        RightSuper,
        Menu,
        Count,
    };

    enum class Modifier : uint8_t
    {
        None     = 0,
        Shift    = 1 << 0,
        Control  = 1 << 1,
        Alt      = 1 << 2,
        Super    = 1 << 3,
        CapsLock = 1 << 4,
        NumLock  = 1 << 5
    };

    using ModifierMask = uint8_t;

    constexpr ModifierMask operator|(Modifier lhs, Modifier rhs) noexcept
    {
        return static_cast<ModifierMask>(lhs) | static_cast<ModifierMask>(rhs);
    }

    constexpr ModifierMask operator|(ModifierMask lhs, Modifier rhs) noexcept
    {
        return lhs | static_cast<ModifierMask>(rhs);
    }

    constexpr ModifierMask& operator|=(ModifierMask& lhs, Modifier rhs) noexcept
    {
        lhs = lhs | rhs;
        return lhs;
    }

    constexpr bool HasModifier(ModifierMask mask, Modifier modifier) noexcept
    {
        return (mask & static_cast<ModifierMask>(modifier)) != 0;
    }

    enum class MouseButton : uint8_t
    {
        Left,
        Right,
        Middle,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8,
        Unknown,
        Count,
    };
}
