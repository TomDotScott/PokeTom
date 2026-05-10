#ifndef GAMECONTEXT_H
#define GAMECONTEXT_H
#include <cstdint>
#include "../Engine/EntityRegistry.h"
#include "WorldDefinition.h"

struct GameContext
{
	EntityRegistry m_Entities;
	WorldDefinition m_World;
	Renderer m_Renderer;
	uint32_t m_PlayerEntityID;
};

#endif // GAMECONTEXT_H
