#include "DialogueManager.h"

#include "../Engine/Asserts.h"
#include "../Engine/Stringtable.h"
#include "../Engine/Parsers/XML/XmlDocument.h"


DialogueManager* DialogueManager::Get()
{
	static std::shared_ptr<DialogueManager> manager = nullptr;

	if (manager == nullptr)
	{
		manager = Create();
	}

	return manager.get();
}

std::string DialogueManager::GetLine(hash_type dialogueID, const uint8_t index) const
{
	ASSERT(m_dialogueTree.contains(dialogueID));

	const std::vector<hash_type>& dialogue = m_dialogueTree.at(dialogueID);

	ASSERT_MSG(index < dialogue.size());

	hash_type stringID = dialogue[index];
	ASSERT(STRINGTABLE->Exists(HASH("DIALOGUE"), stringID));

	return STRINGTABLE->GetString(HASH("DIALOGUE"), stringID);
}

size_t DialogueManager::GetNumLines(hash_type dialogueID) const
{
	ASSERT(m_dialogueTree.contains(dialogueID));
	return m_dialogueTree.at(dialogueID).size();
}

DialogueManager::DialogueManager() = default;

bool DialogueManager::Init()
{
	XmlDocument doc;
	doc.Load("dialogue.xml");

	const auto& root = doc.Root().Child("DialogueTree");

	if (!LoadFromXML(*root))
	{
		return false;
	}

	return true;
}

bool DialogueManager::LoadFromXML(const XmlNode& node)
{
	std::unordered_map<hash_type, std::vector<hash_type>> dialogueTree;

	const std::vector<const XmlNode*> dialogueOptions = node.Children("Dialogue");
	for (const auto& dialogue : dialogueOptions)
	{
		ASSERT(dialogue != nullptr);

		const std::string treeName = dialogue->Attr("name", std::string{});
		if (treeName.empty())
		{
			ASSERT_MSG(false, "DialogueManager::LoadFromXML - No name provided for Dialogue!\n");
			return false;
		}

		hash_type treeID = HASH(treeName);
		if (dialogueTree.contains(treeID))
		{
			ASSERT_MSG(false, "DialogueManager::LoadFromXML - Dialogue tree %s already registered!\n", treeName.c_str());
			return false;
		}

		const auto lines = dialogue->Children();
		std::vector<hash_type> treeLines;
		treeLines.reserve(lines.size());

		for (const auto& line : lines)
		{
			ASSERT(line != nullptr);

			const std::string id = line->Attr("id", std::string{});
			if (id.empty())
			{
				ASSERT_MSG(false, "DialogueManager::LoadFromXML - No stringtable ID provided!\n");
				return false;
			}

			treeLines.emplace_back(HASH(id));
		}

		dialogueTree[treeID] = treeLines;
	}

	m_dialogueTree = std::move(dialogueTree);
	return true;
}
