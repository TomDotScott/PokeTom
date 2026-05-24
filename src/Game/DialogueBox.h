#ifndef DIALOGUEBOX_H
#define DIALOGUEBOX_H
#include <string>
#include <SFML/Graphics/Text.hpp>

#include "../Engine/Asserts.h"
#include "../Engine/UI/UiManager.h"

class DialogueBox
{
	constexpr static auto DIALOGUE_PANEL_NAME = "DIALOGUE_PANEL";

public:
	template <typename... Args>
	static void SetText(const char* fmt, Args... args)
	{
		auto* dialoguePanel = UIMANAGER.GetElement<UiPanel>(DIALOGUE_PANEL_NAME);
		ASSERT(dialoguePanel != nullptr);

		auto* text = dynamic_cast<UiText*>(dialoguePanel->GetChild("DIALOGUE_TEXT"));
		ASSERT(text != nullptr);

		text->SetText(fmt, args...);
	}

	static void SetVisible(bool visible);

	static bool IsVisible();

	// TODO: This should be configurable somewhere; this will change from language to language
	static constexpr int MAX_CHARS = 52;
	static constexpr int MAX_LINES = 3;
};

#endif
