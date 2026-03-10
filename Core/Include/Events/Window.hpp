#pragma once

namespace Core::Events
{
	struct FramebufferResized
    {
        uint32_t width;
        uint32_t height;
    };

	struct WindowCloseRequested {};
}