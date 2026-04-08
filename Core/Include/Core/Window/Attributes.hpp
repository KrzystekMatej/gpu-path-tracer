#pragma once
#include <string>

namespace Core::Window
{
	struct Attributes
	{
		std::string title;
		uint32_t width;
		uint32_t height;
		uint32_t minWidth;
		uint32_t minHeight;

		static Attributes DefaultAttributes()
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