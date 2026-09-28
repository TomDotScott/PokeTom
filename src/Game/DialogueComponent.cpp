#include "DialogueComponent.h"

#include "DialogueBox.h"
#include "DialogueManager.h"
#include "../Engine/Entity.h"
#include "../Engine/StringUtils.h"

DialogueComponent::DialogueComponent(Entity* owner, const hash_type dialogueID, const bool loop, const float loopTimer) :
	m_owner(owner),
	m_dialogueID(dialogueID),
	m_canSpeak(true),
	m_loopDialogue(loop),
	m_loopCountdown(0.f),
	m_loopTime(loopTimer),
	m_dialogueIndex(0),
	m_pageIndex(0)
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
		m_pageIndex = 0;
		m_pages.clear();
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

	// Load the first line
	if (m_pages.empty())
	{
		if (!HasDialogueLeft())
		{
			m_canSpeak = false;
			DialogueBox::SetVisible(false);
			return;
		}

		m_pages = string_utils::WrapToPages(GetNextLine(), DialogueBox::MAX_CHARS, DialogueBox::MAX_LINES);
		m_pageIndex = 0;
	}

	// Show current page
	if (m_pageIndex < m_pages.size())
	{
		DialogueBox::SetText(m_pages[m_pageIndex].c_str());
		++m_pageIndex;
		return;
	}

	// Pages exhausted, advance to next line
	++m_dialogueIndex;
	m_pages.clear();
	m_pageIndex = 0;

	if (!HasDialogueLeft())
	{
		m_canSpeak = false;
		DialogueBox::SetVisible(false);
		return;
	}

	m_pages = string_utils::WrapToPages(GetNextLine(), DialogueBox::MAX_CHARS, DialogueBox::MAX_LINES);
	DialogueBox::SetText(m_pages[m_pageIndex].c_str());
	++m_pageIndex;
}

std::string DialogueComponent::GetNextLine() const
{
	return DIALOGUEMANAGER->GetLine(m_dialogueID, m_dialogueIndex);
}

bool DialogueComponent::HasDialogueLeft() const
{
	return m_dialogueIndex < DIALOGUEMANAGER->GetNumLines(m_dialogueID);
}
