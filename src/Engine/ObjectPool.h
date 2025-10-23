#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H
#include <bitset>
#include <stack>
#include <array>

template<typename T, std::size_t N, bool IS_CONST>
class BitsetIterator
{
public:
	using value_type = std::conditional_t<IS_CONST, const T, T>;
	using pointer = value_type*;
	using reference = value_type&;

	BitsetIterator(const std::bitset<N>* bitset, std::array<T, N>* pool, const std::size_t pos) :
		m_bitset(bitset),
		m_pos(pos),
		m_pool(pool)
	{
		AdvanceToNext();
	}

	reference operator*() const
	{
		return (*m_pool)[m_pos];
	}

	pointer operator->() const
	{
		return (*m_pool)[m_pos];
	}

	BitsetIterator& operator++()
	{
		++m_pos;
		AdvanceToNext();
		return *this;
	}

	BitsetIterator operator++(int)
	{
		BitsetIterator* temp = this;
		++(*this);
		return temp;
	}

	bool operator==(const BitsetIterator& rhs)
	{
		return m_pos == rhs.m_pos && m_bitset == rhs.m_bitset;
	}

	bool operator!=(const BitsetIterator& rhs)
	{
		return !(*this == rhs);
	}

private:
	const std::bitset<N>* m_bitset;
	std::size_t m_pos;
	std::array<T, N>* m_pool;

	void AdvanceToNext()
	{
		while (m_pos < N && !(m_bitset->test(m_pos)))
		{
			m_pos++;
		}
	}
};

template<typename T, std::size_t N>
class BitsetRange
{
public:
	using iterator = BitsetIterator<T, N, false>;
	using const_iterator = BitsetIterator<T, N, true>;

	explicit BitsetRange(const std::bitset<N>& bitset, std::array<T, N>& pool) :
		m_bitset(bitset),
		m_pool(pool)
	{
	}

	iterator begin()
	{
		return iterator(&m_bitset, &m_pool, 0);
	}

	iterator end()
	{
		return iterator(&m_bitset, &m_pool, N);
	}

	const_iterator begin() const
	{
		return const_iterator(&m_bitset, &m_pool, 0);
	}

	const_iterator end() const
	{
		return const_iterator(&m_bitset, &m_pool, N);
	}

	const_iterator cbegin() const
	{
		return const_iterator(&m_bitset, &m_pool, 0);
	}

	const_iterator cend() const
	{
		return const_iterator(&m_bitset, &m_pool, N);
	}

private:
	const std::bitset<N>& m_bitset;
	std::array<T, N>& m_pool;
};



template<typename T, std::size_t N>
class ObjectPool
{
public:
	using iterator = typename BitsetRange<T, N>::iterator;
	using const_iterator = typename BitsetRange<T, N>::const_iterator;

	template<class... Ts>
	explicit ObjectPool(/*Ts... objectConstructorArgs*/) :
		m_range(m_bitset, m_pool),
		m_capacity(N),
		m_size(0)
	{
		for (int i = m_capacity - 1; i >= 0; --i)
		{
			m_freeList.push(i);
		}

		printf("DONE\n");
	}

	template<class... Ts>
	T* ActivateObject(Ts&&... args)
	{
		if (m_freeList.empty())
		{
			// The list is empty!
			return nullptr;
		}

		size_t index = m_freeList.top();
		m_freeList.pop();

		m_size++;

		m_pool[index].OnActivate(std::forward<Ts>(args)...);

		m_bitset.set(index);

		return &m_pool[index];
	}

	void DeactivateObject(T* obj)
	{
		const size_t index = obj - &m_pool[0];

		if (index < 0 || index > m_capacity)
		{
			printf("FAILED TO DEACTIVATE OBJECT\n");
		}

		obj->OnDeactivate();

		m_freeList.push(index);

		m_bitset.reset(index);

		m_size--;
	}

	size_t GetInUseCount() const
	{
		return m_size;
	}

	size_t GetCapacity() const
	{
		return m_capacity;
	}

	iterator begin()
	{
		return m_range.begin();
	}

	iterator end()
	{
		return m_range.end();
	}

	const_iterator begin() const
	{
		return m_range.begin();
	}

	const_iterator end() const
	{
		return m_range.end();
	}

	const_iterator cbegin() const
	{
		return m_range.cbegin();
	}

	const_iterator cend() const
	{
		return m_range.cend();
	}

	T* GetObjectAtIndex(const std::size_t index)
	{
		if (!m_bitset.test(index))
		{
			return nullptr;
		}

		return &m_pool[index];
	}

private:
	std::array<T, N> m_pool;
	std::stack<size_t> m_freeList;

	// Keeping track of the active objects with a bitset
	std::bitset<N> m_bitset;
	BitsetRange<T, N> m_range;

	size_t m_capacity;
	size_t m_size;
};

#endif
