#pragma once

#include <string>

namespace Core
{
	struct WindowAttributes
	{
		std::string title;
		uint32_t width;
		uint32_t height;
		uint32_t minWidth;
		uint32_t minHeight;

		static WindowAttributes DefaultAttributes()
		{
			return {
				.title = "Scene Viewer",
				.width = 1280,
				.height = 720,
				.minWidth = 960,
				.minHeight = 540,
			};
		}
	};
}