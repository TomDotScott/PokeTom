#include "DialogueBox.h"

void DialogueBox::SetVisible(const bool visible)
{
	auto* dialoguePanel = UIMANAGER.GetUiPanel("DIALOGUE_PANEL");
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
