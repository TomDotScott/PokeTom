#ifndef UPDATEABLE_H
#define UPDATEABLE_H
#include <map>

class Updateable
{
public:
	friend class UpdateableRegistry;

	Updateable();
	virtual ~Updateable();

	virtual void Update(float deltaTime) = 0;

private:
	uint32_t m_id;
};

class UpdateableRegistry
{
public:
	bool RegisterUpdateableInstance(Updateable* u);
	bool UnregisterUpdateableInstance(uint32_t id);

	void UpdateAll(float deltaTime);

private:
	std::map<uint32_t, Updateable*> m_instances;
};

extern UpdateableRegistry UPDATEABLE_REGISTRY;

#endif // UPDATEABLE_H
