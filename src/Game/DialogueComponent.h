#ifndef DIALOGUE_COMPONENT_H
#define DIALOGUE_COMPONENT_H
#include <string>

#include "../Engine/IUpdateable.h"

class Entity;

class DialogueComponent : public IUpdateable
{
public:
	DialogueComponent(Entity* owner, std::string dialogueID, bool loop, float loopTimer);
	void Update(float deltaTime) override;
	void OnPlayerInteractPressed() override;

	const std::string& GetNextLine() const;

	bool HasDialogueLeft() const;

private:
	Entity* m_owner;
	std::string m_dialogueID;
	bool m_canSpeak;
	bool m_loopDialogue;
	float m_loopCountdown;
	float m_loopTime;
	uint8_t m_dialogueIndex;

};

#endif
