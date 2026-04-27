#ifndef DIALOGUE_MANAGER_H
#define DIALOGUE_MANAGER_H
#include "../Engine/Factory.h"
#include "../Engine/ISerialisable.h"

class DialogueManager : Factory<DialogueManager>, ISerialisable
{
public:
	static DialogueManager* Get();
	friend class Factory;

	const std::string& GetLine(const std::string& dialogueID, uint8_t index) const;
	uint8_t GetNumLines(const std::string& dialogueID) const;

private:
	std::unordered_map<std::string, std::vector<std::string>> m_dialogueTree;

	DialogueManager();
	bool Init() override;
	bool LoadFromXML(const XmlNode& node) override;
};


#define DIALOGUEMANAGER DialogueManager::Get()

#endif
