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

const std::string& DialogueManager::GetLine(const std::string& dialogueID, const uint8_t index) const
{
	ASSERT(m_dialogueTree.contains(dialogueID));

	const std::vector<std::string>& dialogue = m_dialogueTree.at(dialogueID);
	ASSERT_MSG(index < dialogue.size());

	return dialogue[index];
}

uint8_t DialogueManager::GetNumLines(const std::string& dialogueID) const
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
	std::unordered_map<std::string, std::vector<std::string>> dialogueTree;

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

		if (dialogueTree.contains(treeName))
		{
			ASSERT_MSG(false, "DialogueManager::LoadFromXML - Dialogue tree %s already registered!\n", treeName.c_str());
			return false;
		}

		const auto lines = dialogue->Children();
		std::vector<std::string> treeLines;
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

			if (STRINGTABLE->GetString("DIALOGUE", id).empty())
			{
				ASSERT_MSG(false, "String with ID %s not present in the DIALOGUE stringtable!", id.c_str());
				return false;
			}

			treeLines.emplace_back(STRINGTABLE->GetString("DIALOGUE", id));
		}

		dialogueTree[treeName] = treeLines;
	}

	m_dialogueTree = std::move(dialogueTree);
	return true;
}
