#ifndef UPDATEABLE_H
#define UPDATEABLE_H

class IUpdateable
{
public:
	virtual ~IUpdateable() = default;
	virtual void Update(float deltaTime) = 0;

	virtual void OnActivate()
	{
	}

	virtual void OnDeactivate()
	{
	}

	virtual void OnDestroyed()
	{
	}

	virtual void OnPlayerInteractPressed()
	{
	}
};

#endif // UPDATEABLE_H
