#pragma once

#pragma once
#include "RendererComponent.h"

namespace nu
{
	class ModelComponent : public RendererComponent
	{
	public:
		CLASS_PROTOTYPE(ModelComponent)

		void Draw(const Renderer& renderer) override;

		//void Read(const json::value_t& value) override;

	private:
	};
}