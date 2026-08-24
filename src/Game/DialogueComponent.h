#ifndef DIALOGUE_COMPONENT_H
#define DIALOGUE_COMPONENT_H
#include <string>
#include <vector>

#include "../Engine/Hash.h"
#include "../Engine/IUpdateable.h"

class Entity;

class DialogueComponent : public IUpdateable
{
public:
	DialogueComponent(Entity* owner, hash_type dialogueID, bool loop, float loopTimer);
	void Update(float deltaTime) override;
	void OnPlayerInteractPressed() override;

	std::string GetNextLine() const;

	bool HasDialogueLeft() const;

private:
	Entity* m_owner;
	hash_type m_dialogueID;
	bool m_canSpeak;
	bool m_loopDialogue;
	float m_loopCountdown;
	float m_loopTime;
	uint8_t m_dialogueIndex;

	// Lines are bigger than the box so we want to split them into lines and pages
	// to fit properly!
	std::vector<std::string> m_pages;
	uint8_t m_pageIndex;
};

#endif
