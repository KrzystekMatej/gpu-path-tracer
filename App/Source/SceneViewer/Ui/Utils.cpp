#include <App/SceneViewer/Ui/Utils.hpp>
#include <cmath>

namespace App::SceneViewer::Ui::Utils
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