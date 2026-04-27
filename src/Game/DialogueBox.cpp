#include "DialogueBox.h"

namespace 
{
	const char* PANEL_NAME = "DIALOGUE_PANEL";
}


void DialogueBox::SetVisible(const bool visible)
{
	auto* dialoguePanel = UIMANAGER.GetUiPanel(PANEL_NAME);
	ASSERT(dialoguePanel != nullptr);

	if (visible)
	{
		dialoguePanel->OnActivate();
	}
	else
	{
		dialoguePanel->OnDeactivate();
	}
}

bool DialogueBox::IsVisible()
{
	const auto* dialoguePanel = UIMANAGER.GetUiPanel(PANEL_NAME);
	ASSERT(dialoguePanel != nullptr);

	return dialoguePanel->IsActive();
}
