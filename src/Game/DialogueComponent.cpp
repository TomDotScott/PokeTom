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

		m_pages = WrapToPages(GetNextLine());
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

	m_pages = WrapToPages(GetNextLine());
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

std::vector<std::string> DialogueComponent::WrapToPages(const std::string& text)
{
	// Split the string into words so that we don't split m
	// idword (lol)
	std::vector<std::string> words;
	std::istringstream stream(text);
	std::string word;
	while (stream >> word)
	{
		words.emplace_back(std::move(word));
	}

	std::vector<std::string> pages;
	std::string currentLine;
	std::vector<std::string> currentPageLines;

	auto flush = [&]()
	{
		if (!currentLine.empty())
		{
			currentPageLines.emplace_back(std::move(currentLine));
		}

		std::string page;
		for (size_t i = 0; i < currentPageLines.size(); ++i)
		{
			if (i > 0)
			{
				page += "\n";
			}

			page += currentPageLines[i];
		}

		pages.emplace_back(std::move(page));
		currentPageLines.clear();
		currentLine.clear();
	};

	for (const auto& w : words)
	{
		const size_t len = currentLine.empty() ? w.size() : currentLine.size() + 1 + w.size();

		if (!currentLine.empty() && len > DialogueBox::MAX_CHARS)
		{
			currentPageLines.emplace_back(std::move(currentLine));
			currentLine.clear();

			if (currentPageLines.size() >= DialogueBox::MAX_LINES)
			{
				flush();
			}
		}

		if (!currentLine.empty())
			currentLine += ' ';

		currentLine += w;
	}

	if (!currentLine.empty() || !currentPageLines.empty())
	{
		flush();
	}

	return pages;
}
