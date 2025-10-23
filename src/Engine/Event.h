#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H

#include <functional>
#include <utility>

template <typename... Params>
class Event
{
public:
	size_t On(std::function<void(Params...)> callback)
	{
		m_callbacks.emplace_back(Callback{ std::move(callback), false, ++m_nextID });

		return m_nextID;
	}

	size_t Once(std::function<void(Params...)> callback)
	{
		m_callbacks.emplace_back(Callback{ std::move(callback), true, ++m_nextID });

		return m_nextID;
	}

	void Off(size_t id)
	{
		m_callbacks.erase(
			std::remove_if(m_callbacks.begin(), m_callbacks.end(),
				[id](const Callback& cb) { return cb.m_callbackID == id; }),
			m_callbacks.end()
		);
	}


	void Fire(Params... args)
	{
		for (int i = 0; i < m_callbacks.size();)
		{
			auto& func = m_callbacks[i];

			func.m_callback(args...);

			if (func.m_once)
			{
				m_callbacks.erase(m_callbacks.begin() + i);
			}
			else
			{
				++i;
			}
		}
	}

private:
	struct Callback
	{
		std::function<void(Params...)> m_callback;
		bool m_once;
		size_t m_callbackID;
	};

	std::vector<Callback> m_callbacks;

	size_t m_nextID = 0x69;
};


#endif
