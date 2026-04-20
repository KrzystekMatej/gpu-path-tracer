#pragma once
#include <memory>
#include <vector>
#include <Core/Runtime/Layer/Base.hpp>
#include <Core/Runtime/Layer/CommandService.hpp>
#include <Core/Utils/Guid.hpp>

namespace Core::Runtime::Layer
{
	class Stack final : public CommandService
	{
	public:
		Stack() = default;

		Base* PushTop(Core::Utils::Guid id, std::unique_ptr<Base> layer) override;
		Base* PushBottom(Core::Utils::Guid id, std::unique_ptr<Base> layer) override;
		void PopTop() override;
		void PopBottom() override;
		bool Remove(Core::Utils::Guid id) override;
		Base* Replace(Core::Utils::Guid id, std::unique_ptr<Base> layer) override;

		void Update();
		void BuildUi();
		void Render(Graphics::Services::SceneRenderer renderer);
		void Clear();

	private:
		struct Entry
		{
			Core::Utils::Guid Id = 0;
			std::unique_ptr<Base> Instance;
		};

	private:
		std::vector<Entry>::iterator Find(Core::Utils::Guid id);
		std::vector<Entry>::const_iterator Find(Core::Utils::Guid id) const;

	private:
		std::vector<Entry> m_Layers;
	};
}
