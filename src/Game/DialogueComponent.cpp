#include "DialogueComponent.h"

#include "DialogueBox.h"
#include "DialogueManager.h"
#include "../Engine/Entity.h"

DialogueComponent::DialogueComponent(Entity* owner, std::string dialogueID, const bool loop, const float loopTimer) :
	m_owner(owner),
	m_dialogueID(std::move(dialogueID)),
	m_canSpeak(true),
	m_loopDialogue(loop),
	m_loopCountdown(0.f),
	m_loopTime(loopTimer),
	m_dialogueIndex(0)
{
}

void DialogueComponent::Update(const float deltaTime)
{
	if (!m_owner->IsActive())
	{
		return;
	}

	if (m_canSpeak)
	{
		return;
	}

	if (!m_loopDialogue)
	{
		return;
	}

	m_loopCountdown += deltaTime;
	if (m_loopCountdown > m_loopTime)
	{
		m_canSpeak = true;
		m_dialogueIndex = 0;
	}
}

void DialogueComponent::OnPlayerInteractPressed()
{
	IUpdateable::OnPlayerInteractPressed();

	if (!m_canSpeak)
	{
		return;
	}

	if (!DialogueBox::IsVisible())
	{
		DialogueBox::SetVisible(true);
	}

	if (m_dialogueIndex > DIALOGUEMANAGER->GetNumLines(m_dialogueID) - 1)
	{
		m_canSpeak = false;
		DialogueBox::SetVisible(false);
	}
	else
	{
		// TODO: Replace any strings within {}
		// TODO: Probably make it a member of the STRINGTABLE?
		DialogueBox::SetText(GetNextLine().c_str());

		m_dialogueIndex++;
	}
}

const std::string& DialogueComponent::GetNextLine() const
{
	return DIALOGUEMANAGER->GetLine(m_dialogueID, m_dialogueIndex);
}
