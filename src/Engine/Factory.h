#ifndef FACTORY_H
#define FACTORY_H
#include <memory>

template <typename Base>
class Factory
{
public:
	template <typename... ConstructorArgs>
	static std::shared_ptr<Base> Create(ConstructorArgs&&... args)
	{
		auto base = std::shared_ptr<Base>(new Base(std::forward<ConstructorArgs>(args)...));

		if (base->Init())
		{
			return base;
		}

		return nullptr;
	}

protected:
	virtual bool Init() = 0;
	friend Base;

private:
	Factory() = default;
	~Factory() = default;
};

#endif
