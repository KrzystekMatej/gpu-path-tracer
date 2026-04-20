#include <algorithm>
#include <cassert>
#include <Core/Runtime/Layer/Stack.hpp>

namespace Core::Runtime::Layer
{
	Base* Stack::PushTop(Core::Utils::Guid id, std::unique_ptr<Base> layer)
	{
		assert(layer != nullptr);

		Base* result = layer.get();
		layer->OnAttach();
		m_Layers.push_back({ id, std::move(layer) });
		return result;
	}

	Base* Stack::PushBottom(Core::Utils::Guid id, std::unique_ptr<Base> layer)
	{
		assert(layer != nullptr);

		Base* result = layer.get();
		layer->OnAttach();
		m_Layers.insert(m_Layers.begin(), Entry{ id, std::move(layer) });
		return result;
	}

	void Stack::PopTop()
	{
		if (m_Layers.empty())
			return;

		m_Layers.back().Instance->OnDetach();
		m_Layers.pop_back();
	}

	void Stack::PopBottom()
	{
		if (m_Layers.empty())
			return;

		m_Layers.front().Instance->OnDetach();
		m_Layers.erase(m_Layers.begin());
	}

	bool Stack::Remove(Core::Utils::Guid id)
	{
		auto it = Find(id);
		if (it == m_Layers.end())
			return false;

		it->Instance->OnDetach();
		m_Layers.erase(it);
		return true;
	}

	Base* Stack::Replace(Core::Utils::Guid id, std::unique_ptr<Base> layer)
	{
		assert(layer != nullptr);

		auto it = Find(id);
		if (it == m_Layers.end())
			return nullptr;

		Base* result = layer.get();

		it->Instance->OnDetach();
		layer->OnAttach();
		it->Instance = std::move(layer);

		return result;
	}

	void Stack::Update()
	{
		for (auto& entry : m_Layers)
			entry.Instance->OnUpdate();
	}

	void Stack::BuildUi()
	{
		for (auto& entry : m_Layers)
			entry.Instance->OnBuildUi();
	}

	void Stack::Render(Graphics::Services::SceneRenderer renderer)
	{
		for (auto& entry : m_Layers)
			entry.Instance->OnRender(renderer);
	}

	void Stack::Clear()
	{
		for (auto& entry : m_Layers)
			entry.Instance->OnDetach();

		m_Layers.clear();
	}

	std::vector<Stack::Entry>::iterator Stack::Find(Core::Utils::Guid id)
	{
		return std::find_if(
			m_Layers.begin(),
			m_Layers.end(),
			[id](const Entry& entry)
			{
				return entry.Id == id;
			});
	}

	std::vector<Stack::Entry>::const_iterator Stack::Find(Core::Utils::Guid id) const
	{
		return std::find_if(
			m_Layers.begin(),
			m_Layers.end(),
			[id](const Entry& entry)
			{
				return entry.Id == id;
			});
	}
}
