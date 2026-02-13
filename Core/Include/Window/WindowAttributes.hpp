#pragma once

#include <string>

namespace Core
{
	struct WindowAttributes
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		uint32_t MinWidth;
		uint32_t MinHeight;

		static WindowAttributes DefaultAttributes()
		{
			return {
				.Title = "Scene Viewer",
				.Width = 1280,
				.Height = 720,
				.MinWidth = 960,
				.MinHeight = 540,
			};
		}
	};
}