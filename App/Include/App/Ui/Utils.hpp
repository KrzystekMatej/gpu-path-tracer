#pragma once
#include <cstdint>
#include <initializer_list>

namespace App::Ui::Utils
{
	float GetStackHeight(std::initializer_list<float> itemHeights, float itemSpacingY, float paddingY);
	void BuildResponsiveInputInt(const char* label, const char* id, int* value, bool expandInputs);
	void BuildResponsiveInputUInt(const char* label, const char* id, uint32_t* value, bool expandInputs);
	bool BeginResponsiveCombo(const char* label, const char* id, const char* previewValue, bool expandInputs);
}
