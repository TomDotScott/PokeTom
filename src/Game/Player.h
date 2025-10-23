#ifndef PLAYER_H
#define PLAYER_H
#include "../Engine/Gameobject.h"
#include "../Engine/Input/InputMapper.h"

class Player final : public GameObject, Updatable
{
public:
	Player();

	void Update() override;

private:
	enum eInputs : uint8_t
	{
	};

	InputMapper m_mapper;
};


#endif
