#include <App/Ui/Utils.hpp>
#include <cmath>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace App::Ui::Utils
{
	float GetStackHeight(std::initializer_list<float> itemHeights, float itemSpacingY, float paddingY)
	{
		if (itemHeights.size() == 0)
			return 2.0f * paddingY;

		float height = 2.0f * paddingY;
		bool first = true;

		for (float itemHeight : itemHeights)
		{
			if (!first)
				height += itemSpacingY;

			height += itemHeight;
			first = false;
		}

		return std::ceil(height);
	}

	void BuildResponsiveInputInt(const char* label, const char* id, int* value, bool expandInputs)
	{
		if (expandInputs)
		{
			ImGui::TextUnformatted(label);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputInt(id, value);
		}
		else
		{
			ImGui::InputInt(label, value);
		}
	}
	
	void BuildResponsiveInputUInt(const char* label, const char* id, uint32_t* value, bool expandInputs)
	{
		if (expandInputs)
		{
			ImGui::TextUnformatted(label);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputScalar(id, ImGuiDataType_U32, value);
		}
		else
		{
			ImGui::InputScalar(label, ImGuiDataType_U32, value);
		}
	}

	void BuildResponsiveInputString(const char* label, const char* id, std::string& value, bool expandInputs)
	{
		if (expandInputs)
		{
			ImGui::TextUnformatted(label);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::InputText(id, &value);
		}
		else
		{
			ImGui::InputText(label, &value);
		}
	}

	bool BeginResponsiveCombo(const char* label, const char* id, const char* previewValue, bool expandInputs)
	{
		if (expandInputs)
		{
			ImGui::TextUnformatted(label);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			return ImGui::BeginCombo(id, previewValue);
		}

		return ImGui::BeginCombo(label, previewValue);
	}
}
