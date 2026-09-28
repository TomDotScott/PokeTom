#include "UILayer.h"

UILayer::UILayer() : m_finished(false)
{
}

bool UILayer::IsFinished() const
{
	return m_finished;
}

void UILayer::Update(float /*deltaTime*/)
{
}

void UILayer::OnActivate(const BattleState& /*ctx*/, const LayerResult& /*prevLayerResult*/)
{
	m_finished = false;
}

void UILayer::OnDeactivate()
{
}
