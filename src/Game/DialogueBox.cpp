#include "DialogueBox.h"


void DialogueBox::SetVisible(const bool visible)
{
	auto* dialoguePanel = UIMANAGER.GetElement<UiPanel>(DIALOGUE_PANEL_NAME);
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
	const auto* dialoguePanel = UIMANAGER.GetElement<UiPanel>(DIALOGUE_PANEL_NAME);
	ASSERT(dialoguePanel != nullptr);

	return dialoguePanel->IsActive();
}
