#include "cepch.h"
#include "LayerStack.h"

namespace CatEngine 
{
	LayerStack::LayerStack()
	{
	}
	LayerStack::~LayerStack()
	{
        CE_PROFILE_FUNCTION();
		for (Layer* layer : m_Layers) {
			layer->OnDetach();
			delete layer;
		}
	}
	void LayerStack::PushLayer(Layer* layer)
	{
        CE_PROFILE_FUNCTION();
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}
	void LayerStack::PushOverlay(Layer* overlay)
	{
        CE_PROFILE_FUNCTION();
		m_Layers.emplace_back(overlay);
	}
	void LayerStack::PopLayer(Layer* layer)
	{
        CE_PROFILE_FUNCTION();
		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end()) 
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}
	void LayerStack::PopOverlay(Layer* overlay)
	{
        CE_PROFILE_FUNCTION();
		auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (it != m_Layers.end()) 
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
	}
}
