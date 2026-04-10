#ifndef UPDATEABLE_H
#define UPDATEABLE_H

class IUpdateable
{
public:
	virtual ~IUpdateable() = default;
	virtual void Update(float deltaTime) = 0;
};

#endif // UPDATEABLE_H
