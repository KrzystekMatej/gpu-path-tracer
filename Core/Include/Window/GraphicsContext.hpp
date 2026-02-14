#pragma once
#include <expected>
#include "GLFW/glfw3.h"
#include "Error/Error.hpp"

namespace Core
{
	struct GLVersion
	{
		int Major;
		int Minor;
		std::string Vendor;
		std::string Renderer;
		std::string GLSL;
	};

	class GraphicsContext
	{
	public:
		GraphicsContext(const GraphicsContext&) = delete;
		GraphicsContext& operator=(const GraphicsContext&) = delete;
		GraphicsContext(GraphicsContext&&) noexcept = default;
		GraphicsContext& operator=(GraphicsContext&&) noexcept = default;

		static std::expected<GraphicsContext, Error> Create(GLFWwindow* windowHandle);
		void MakeCurrent() const;
		void Detach() const;
		void SetSwapInterval(int interval) const;
		GLVersion GetVersion() const;
	private:
		GraphicsContext(GLFWwindow* windowHandle);

		GLFWwindow* m_WindowHandle;
	};
}