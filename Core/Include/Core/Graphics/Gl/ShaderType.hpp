#pragma once

namespace Core::Graphics::Gl
{
	enum class ShaderType
	{
		Vertex,
		Fragment, 
		Geometry,
		Compute,
		TessellationControl,
		TessellationEvaluation
	};
}