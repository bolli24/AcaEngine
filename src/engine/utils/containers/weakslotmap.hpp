#pragma once

#include "../../utils/assert.hpp"
#include "../../utils/metaProgHelpers.hpp"
#include "iteratorrange.hpp"
#include <vector>
#include <limits>
#include <utility>
#include <concepts>
#include <memory>

namespace utils {
	template<std::integral Key, bool TrivialDestruct = false>
	class WeakSlotMap
	{
	protected:
		constexpr static Key INVALID_SLOT = std::numeric_limits<Key>::max();
	public:
		using SizeType = Key;

		template<std::movable Value>
		WeakSlotMap(utils::TypeHolder<Value>, SizeType _initialSize = 4)
			: m_elementSize(sizeof(Value)),
			m_destructor(destroyElement<Value>),
			m_move(moveElement<Value>)
		{
			m_valuesToSlots.reserve(_initialSize);
			m_values.reset(new char[index(capacity())]);

			static_assert(std::is_trivially_destructible_v<Value> || !TrivialDestruct,
				"Managed elements require a destructor call.");
		}

		WeakSlotMap(WeakSlotMap&& _oth) noexcept
			: m_elementSize(_oth.m_elementSize),
			m_destructor(_oth.m_destructor),
			m_move(_oth.m_move),
			m_values(std::move(_oth.m_values)),
			m_slots(std::move(_oth.m_slots)),
			m_valuesToSlots(std::move(_oth.m_valuesToSlots))
		{
		}

		WeakSlotMap& operator=(WeakSlotMap&& _oth) noexcept
		{
			m_elementSize = _oth.m_elementSize;
			m_destructor = _oth.m_destructor;
			m_move = _oth.m_move;
			m_values = std::move(_oth.m_values);
			m_slots = std::move(_oth.m_slots);
			m_valuesToSlots = std::move(_oth.m_valuesToSlots);

			return *this;
		}


		~WeakSlotMap()
		{
			destroyValues();
		}

		template<std::movable Value, typename... Args>
		Value& emplace(Key _key, Args&&... _args)
		{
			// increase slots if necessary
			if (m_slots.size() <= _key)
				m_slots.resize(_key + 1, INVALID_SLOT);
			else if(m_slots[_key] != INVALID_SLOT) // already exists
				return at<Value>(_key);

			const SizeType oldCapacity = static_cast<SizeType>(m_valuesToSlots.size());
			m_slots[_key] = size();
			m_valuesToSlots.emplace_back(_key);
			const SizeType newCapacity = static_cast<SizeType>(m_valuesToSlots.size());

			if (oldCapacity != newCapacity)
			{
				char* newBuf = new char[index(newCapacity)];
				for (SizeType i = 0; i < oldCapacity; ++i)
				{
					new(&newBuf[index(i)]) Value(std::move(get<Value>(i)));
					get<Value>(i).~Value();
				}

				m_values.reset(newBuf);
			}
			
			return *new (&get<Value>(oldCapacity)) Value (std::forward<Args>(_args)...);
		}

		void erase(Key _key)
		{
			ASSERT(contains(_key), "Trying to delete a non existing element.");

			const Key ind = m_slots[_key];
			m_slots[_key] = INVALID_SLOT;
			// effectively get() but without knowing the type
			char* back = &m_values[index(size() - 1)];

			if (ind+1 < size())
			{
				m_move(&m_values[index(ind)], back);
				m_slots[m_valuesToSlots.back()] = ind;
				m_valuesToSlots[ind] = m_valuesToSlots.back();
			}

			if constexpr (!TrivialDestruct) m_destructor(back);
			m_valuesToSlots.pop_back();
		}

		void clear()
		{
			destroyValues();
			m_slots.clear();
			m_valuesToSlots.clear();
		}

		// iterators
		template<typename Value>
		struct Accessor
		{
			static Key key(const WeakSlotMap& _this, SizeType _index) { return _this.m_valuesToSlots[_index]; }
			static Value& value(WeakSlotMap& _this, SizeType _index) { return _this.get<Value>(_index); }
			static const Value& value(const WeakSlotMap& _this, SizeType _index) { return _this.get<Value>(_index); }
		};

		template<typename Value>
		auto iterate() { return IteratorRange<WeakSlotMap, Key, Value, Accessor<Value>>(*this); }
		template<typename Value>
		auto iterate() const { return ConstIteratorRange<WeakSlotMap, Key, Value, Accessor<Value>>(*this); }

		// access operations
		bool contains(Key _key) const { return _key < m_slots.size() && m_slots[_key] != INVALID_SLOT; }
		
		template<typename Value>
		Value& at(Key _key) 
		{
			ASSERT(contains(_key), "Trying to access a non existing element.");
			return reinterpret_cast<Value&>(m_values[index(m_slots[_key])]); 
		}
		template<typename Value>
		const Value& at(Key _key) const 
		{
			ASSERT(contains(_key), "Trying to access a non existing element.");
			return reinterpret_cast<const Value&>(m_values[index(m_slots[_key])]); 
		}

		SizeType size() const { return static_cast<SizeType>(m_valuesToSlots.size()); }
		SizeType capacity() const { return static_cast<SizeType>(m_valuesToSlots.capacity()); }
		bool empty() const { return m_valuesToSlots.empty(); }
	private:
		// access through internal index
		template<typename Value>
		Value& get(SizeType _ind) { return reinterpret_cast<Value&>(m_values[index(_ind)]); }
		template<typename Value>
		const Value& get(SizeType _ind) const { return reinterpret_cast<const Value&>(m_values[index(_ind)]); }
		size_t index(SizeType _ind) const { return static_cast<size_t>(_ind) * m_elementSize; }

		void destroyValues()
		{
			if constexpr (TrivialDestruct) return;

			for (SizeType i = 0; i < size(); ++i)
				m_destructor(&m_values[index(i)]);
		}

		template<typename Value>
		static void destroyElement(void* ptr)
		{
			static_cast<Value*>(ptr)->~Value();
		}
		using Destructor = void(*)(void*);

		template<typename Value>
		static void moveElement(void* dst, void* src)
		{
			*static_cast<Value*>(dst) = std::move(*static_cast<Value*>(src));
		}
		using Move = void(*)(void*, void*);

		int m_elementSize;
		Destructor m_destructor;
		Move m_move;

		std::unique_ptr<char[]> m_values;
		std::vector<Key> m_slots;
		std::vector<Key> m_valuesToSlots;
	};
}
