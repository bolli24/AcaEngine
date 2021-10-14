#pragma once

#include <cinttypes>
#include <type_traits>
#include <cstring>
#include <functional>
#include <concepts>

namespace utils {

template<typename K, typename T, typename Hash = std::hash<K>, typename Compare = std::equal_to<K>>
class HashMap
{
public:
	/// Handles are direct accesses into a specific hashmap.
	/// Any add or remove in the HM will invalidate the handle without notification.
	/// A handle might be usable afterwards, but there is no guaranty.
	template<typename MapT, typename DataT>
	class HandleT
	{
		MapT* map;
		uint32_t idx;

		HandleT(MapT* _map, uint32_t _idx = 0) :
			map(_map),
			idx(_idx)
		{
			if(_map)
			{
				if(_map->m_size == 0)
					map = nullptr;
				else {
					// Find a valid start value.
					while(idx < _map->m_capacity && _map->m_keys[idx].dist == 0xffffffff)
						++idx;
			
					if(idx == _map->m_capacity)
						_map = nullptr;
				}
			}
		}

		friend HashMap;
	public:
		const K& key() const { return map->m_keys[idx].key; }

		DataT& data() const { return map->m_data[idx]; }

		operator bool () const { return map != nullptr; }

		HandleT& operator ++ ()
		{
			++idx;
			// Move forward while the element is empty.
			while((idx < map->m_capacity) && (map->m_keys[idx].dist == 0xffffffff))
				++idx;
			// Set to invalid handle?
			if(idx >= map->m_capacity) { idx = 0; map = nullptr; }
			return *this;
		}

		bool operator == (const HandleT& _other) const { return map == _other.map && idx == _other.idx; }
		bool operator != (const HandleT& _other) const { return map != _other.map || idx != _other.idx; }

		// The dereference operator has no function other than making this handle compatible
		// for range based loops.
		const HandleT& operator * () const { return *this; }
	};

	typedef HandleT<HashMap, T> Handle;
	typedef HandleT<const HashMap, const T> ConstHandle;

	explicit HashMap(uint32_t _expectedElementCount = 15) :
		m_capacity(estimateCapacity(_expectedElementCount)),
		m_size(0)
	{
		m_keys = static_cast<Key*>(malloc(sizeof(Key) * m_capacity));
		m_data = static_cast<T*>(malloc(sizeof(T) * m_capacity));
		
		for(uint32_t i = 0; i < m_capacity; ++i)
			m_keys[i].dist = 0xffffffff;
	}

	HashMap(HashMap&& _other) noexcept :
		m_capacity(_other.m_capacity),
		m_size(_other.m_size),
		m_keys(_other.m_keys),
		m_data(_other.m_data)
	{
		_other.m_keys = nullptr;
		_other.m_data = nullptr;
	}

	HashMap& operator = (HashMap&& _rhs) noexcept
	{
		this->~HashMap();
		m_capacity = _rhs.m_capacity;
		m_size = _rhs.m_size;
		m_keys = _rhs.m_keys;
		m_data = _rhs.m_data;
		_rhs.m_keys = nullptr;
		_rhs.m_data = nullptr;
		return *this;
	}

	~HashMap()
	{
		if(m_keys && m_data)
		{
			// Call destructors of allocated keys and data
			for(uint32_t i = 0; i < m_capacity; ++i)
				if(m_keys[i].dist != 0xffffffff)
				{
					m_data[i].~T();
					m_keys[i].key.~K();
				}
		}
		free(m_keys);
		free(m_data);
	}

	// Add an element to the map.
	// Overwrites the current value if the key already exists.
	// KeyT is a template parameter to capture a forwarding reference.
	template<class KeyT, class DataT>
		requires (std::is_same_v<std::remove_cvref_t<KeyT>, K>)
	Handle add(KeyT&& _key, DataT&& _data)
	{
		using namespace std;

		// in case a const key is captured at KeyT we need a copy
		K key(std::forward<KeyT>(_key));
		uint32_t h = static_cast<uint32_t>(m_hash(key));
	restartAdd:
		uint32_t insertIdx = ~0;
		uint32_t d = 0;
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff) // while not empty cell
		{
			if(m_keyCompare(m_keys[idx].key, key)) // overwrite if keys are identically
			{
				m_data[idx] = std::forward<DataT>(_data);
				return Handle(this, idx);
			}
			// probing (collision)
			// Since we have encountered a collision: should we resize?
			if(m_size > 0.77 * m_capacity) {
				reserve(m_size * 2);
				// The resize changed everything beginning from the index
				// to the content of the target cell. Restart the search.
				goto restartAdd;
			}

			if(m_keys[idx].dist < d) // Swap and then insert the element from this location instead
			{
				swap(key, m_keys[idx].key);
				swap(d, m_keys[idx].dist);
				swap(_data, m_data[idx]);
				if(insertIdx == ~0u) insertIdx = idx;
			}
			++d;
		//	idx = (idx + 1) % m_capacity;
			if(++idx >= m_capacity) idx = 0;
		}
		new (&m_keys[idx].key)(K)(std::move(key));
		m_keys[idx].dist = d;
		new (&m_data[idx])(T)(std::forward<DataT>(_data));
		++m_size;
		if(insertIdx == ~0u) insertIdx = idx;
		return Handle(this, insertIdx);
	}

	// Remove an element if it exists
	void remove(const K& _key)
	{
		remove(find(_key));
	}

	// Remove an existing element
	void remove(const Handle& _element)
	{
		using namespace std;
		if(_element)
		{
			m_keys[_element.idx].dist = 0xffffffff;
			m_data[_element.idx].~T();
			m_keys[_element.idx].key.~K();
			--m_size;
			uint32_t i = _element.idx;
			uint32_t next = (_element.idx + 1) % m_capacity;
			while((m_keys[next].dist != 0) && (m_keys[next].dist != 0xffffffff))
			{
				new (&m_keys[i].key)(K)( move(m_keys[next].key) );
				new (&m_data[i])(T)( move(m_data[next]) );
				m_keys[i].dist = m_keys[next].dist - 1;
				m_keys[next].dist = 0xffffffff;
				i = next;
				if(++next >= m_capacity) next = 0;
			}
		}
	}

	Handle find(const K& _key) noexcept
	{
		uint32_t d = 0;
		uint32_t h = (uint32_t)m_hash(_key);
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff && d <= m_keys[idx].dist)
		{
			if(m_keyCompare(m_keys[idx].key, _key))
				return Handle(this, idx);
			if(++idx >= m_capacity) idx = 0;
			++d;
		}
		return Handle(nullptr, 0);
	}
	ConstHandle find(const K& _key) const noexcept
	{
		// COPY OF find() <noconst>
		uint32_t d = 0;
		uint32_t h = (uint32_t)m_hash(_key);
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff && d <= m_keys[idx].dist)
		{
			if(m_keyCompare(m_keys[idx].key, _key))
				return ConstHandle(this, idx);
			if(++idx >= m_capacity) idx = 0;
			++d;
		}
		return ConstHandle(nullptr, 0);
	}

	/// Get access to an element. If it was not in the map before it will be added with default construction.
	T& operator [] (const K& _key)
		requires std::is_default_constructible_v<T>
	{
		uint32_t d = 0;
		uint32_t h = (uint32_t)m_hash(_key);
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff && d <= m_keys[idx].dist)
		{
			if(m_keyCompare(m_keys[idx].key, _key))
				return m_data[idx];
			if(++idx >= m_capacity) idx = 0;
			++d;
		}
		// Stopped at an empty cell -> insert here.
		if(m_keys[idx].dist == 0xffffffff)
		{
			new (&m_keys[idx].key)(K)(_key);
			new (&m_data[idx])(T)(); // New default element
			m_keys[idx].dist = d;
			++m_size;
		} else { // Stopped because of a collision.
			if(m_size > 0.77 * m_capacity)
				reserve(m_size * 2);
			idx = reinsertUnique(_key, T(), h);
		}
		return m_data[idx];
	}

	// Change the capacity if possible. It cannot be decreased below 'size'.
	void resize(uint32_t _newCapacity)
	{
		using namespace std;
		//if(_newCapacity == m_capacity) return;
		if(_newCapacity < m_size) _newCapacity = m_size;

		HashMap tmp(_newCapacity);
		// Find all data sets and readd them to the new temporary hm
		for(uint32_t i = 0; i < m_capacity; ++i)
		{
			if(m_keys[i].dist != 0xffffffff)
			{
				// We can use a reduced version of add, since we know the element is unique
				// and will not cause a resize.
				uint32_t h = (uint32_t)m_hash(m_keys[i].key);
				tmp.reinsertUnique(move(m_keys[i].key), move(m_data[i]), h);
				// destructor still needs to be called
				// todo: compare performance with destructor call here
			//	m_keys[i].dist = 0xffffffff;
			}
		}

		// Use the temporary map now and let the old memory be destroyed.
		swap(*this, tmp);
	}

	void reserve(uint32_t _exptectedElementCount)
	{
		resize(estimateCapacity(_exptectedElementCount));
	}

	/// Remove all elements from the set but keep the capacity.
	void clear()
	{
		if(m_size > 0)
		{
			for(uint32_t i = 0; i < m_capacity; ++i)
				if(m_keys[i].dist != 0xffffffff)
				{
					m_data[i].~T();
					m_keys[i].key.~K();
					m_keys[i].dist = 0xffffffff;
				}
			m_size = 0;
		}
	}

	uint32_t size() const { return m_size; }

	/// Returns the first element found in the map or an invalid handle when the map is empty.
	Handle begin()
	{
		return Handle(this);
	}
	ConstHandle begin() const
	{
		return ConstHandle(this);
	}


	/// Return the invalid handle for range based for loops
	Handle end()
	{
		return Handle(nullptr);
	}
	ConstHandle end() const
	{
		return ConstHandle(nullptr);
	}

private:
	uint32_t m_capacity;
	uint32_t m_size;

	struct Key
	{
		K key;
		uint32_t dist; // robin hood cashing offset
	};

	Key* m_keys;
	T* m_data;
	Hash m_hash;
	Compare m_keyCompare;
	
	static uint32_t estimateCapacity(uint32_t _exptectedElementCount)
	{
		 // Try to keep the capacity odd (prime would be even better)
		return (uint32_t(_exptectedElementCount * 1.3) | 1) + 2;
	}

	/*uint32_t hash(const uint32_t* _key, unsigned _numWords)
	{
		// TODO: general purpose hash function
		return *_key;
	}*/

	/// Kernel of the Add method, but without resizing,  hash computation and
	/// key compares. I.e. this method assumes that the element is not contained, but
	/// space is available.
	/// \returns The internal index for interal use (may be used to create the Handle).
	template<class DataT>
	uint32_t reinsertUnique(K _key, DataT&& _data, uint32_t h)
	{
		using namespace std;
		uint32_t insertIdx = ~0;
		uint32_t d = 0;
		uint32_t idx = h % m_capacity;
		while(m_keys[idx].dist != 0xffffffff) // while not empty cell
		{
			if(m_keys[idx].dist < d) // Swap and then insert the element from this location instead
			{
				swap(_key, m_keys[idx].key);
				swap(d, m_keys[idx].dist);
				swap(_data, m_data[idx]);
				if(insertIdx == ~0u) insertIdx = idx;
			}
			++d;
			//	idx = (idx + 1) % m_capacity;
			if(++idx >= m_capacity) idx = 0;
		}
		new (&m_keys[idx].key)(K)(move(_key));
		m_keys[idx].dist = d;
		new (&m_data[idx])(T)(move(_data));
		++m_size;
		if(insertIdx == ~0u) insertIdx = idx;
		return insertIdx;
	}
};

} // namespace utils