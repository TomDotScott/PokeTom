#ifndef DIALOGUE_MANAGER_H
#define DIALOGUE_MANAGER_H
#include "../Engine/Hash.h"
#include "../Engine/Factory.h"
#include "../Engine/ISerialisable.h"
#include "../Engine/Hash.h"

class DialogueManager : Factory<DialogueManager>, ISerialisable
{
public:
	static DialogueManager* Get();
	friend class Factory;

	std::string GetLine(hash_type dialogueID, uint8_t index) const;
	size_t GetNumLines(hash_type dialogueID) const;

private:
	// KEY = DIALOGUE TREE ID, VALUE = List of StringIDs belonging to the dialogue tree
	std::unordered_map<hash_type, std::vector<hash_type>> m_dialogueTree;

	DialogueManager();
	bool Init() override;
	bool LoadFromXML(const XmlNode& node) override;
};


#define DIALOGUEMANAGER DialogueManager::Get()

#endif
