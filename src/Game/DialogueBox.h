#ifndef DIALOGUEBOX_H
#define DIALOGUEBOX_H
#include <string>
#include <SFML/Graphics/Text.hpp>

#include "../Engine/Asserts.h"
#include "../Engine/UI/UiManager.h"

class DialogueBox
{
public:
	template<typename... Args>
	static void SetText(const char* fmt, Args... args)
	{
		auto* text = dynamic_cast<UiText*>(UIMANAGER.GetUiPanel("DIALOGUE_PANEL")->GetChild("DIALOGUE_TEXT"));
		ASSERT(text != nullptr);

		text->SetText(fmt, args...);
	}

	static void SetVisible(bool visible);
};

#endif
