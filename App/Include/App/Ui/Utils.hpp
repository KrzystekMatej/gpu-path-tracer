#pragma once
#include <initializer_list>
#include "imgui.h"

namespace App::Ui::Utils
{
	float GetStackHeight(std::initializer_list<float> itemHeights, float itemSpacingY, float paddingY);
	void BuildResponsiveInputInt(const char* label, const char* id, int* value, bool expandInputs);
	bool BeginResponsiveCombo(const char* label, const char* id, const char* previewValue, bool expandInputs);
}