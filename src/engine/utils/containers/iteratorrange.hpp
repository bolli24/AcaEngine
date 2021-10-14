#pragma once

#include <utility>

namespace utils {

	// View on a generic (type erased) container that permits iteration.
	// The Accessor defines the required operations with the interface:
	/*	template<typename Value>
		struct Accessor
		{
			static Key key(const Container& _this, SizeType _index);
			static Value& value(Container& _this, SizeType _index);
			static const Value& value(const Container& _this, SizeType _index);
		};
	* */
	template<typename Container, typename Key, typename Value, typename Accessor>
	class IteratorRange
	{
		using SizeType = typename Container::SizeType;
	public:
		IteratorRange(Container& _target) : m_target(_target) {}

		class Iterator
		{
		public:
			Iterator(Container& _target, SizeType _ind) : m_target(_target), m_index(_ind) {}

			Key key() const { return Accessor::key(m_target, m_index); }
			Value& value() const { return Accessor::value(m_target, m_index); }

			std::pair<Key, Value&> operator*()
			{
				return std::pair<Key, Value&>(Accessor::key(m_target, m_index), Accessor::value(m_target, m_index));
			}
			std::pair<Key, const Value&> operator*() const
			{
				return std::pair<Key, const Value&>(Accessor::key(m_target, m_index), Accessor::value(m_target, m_index));
			}

			Iterator& operator++() { ++m_index; return *this; }
			Iterator operator++(int) { Iterator tmp(*this);  ++m_index; return tmp; }
			bool operator==(const Iterator& _oth) const { ASSERT(&m_target == &_oth.m_target, "Comparing iterators of different containers."); return m_index == _oth.m_index; }
			bool operator!=(const Iterator& _oth) const { ASSERT(&m_target == &_oth.m_target, "Comparing iterators of different containers."); return m_index != _oth.m_index; }
		private:
			Container& m_target;
			SizeType m_index;
		};

		Iterator begin() const { return Iterator(m_target, 0); }
		Iterator end() const { return Iterator(m_target, m_target.size()); }

	private:
		Container& m_target;
	};

	template<typename Container, typename Key, typename Value, typename Accessor>
	class ConstIteratorRange
	{
		using SizeType = typename Container::SizeType;
	public:
		ConstIteratorRange(const Container& _target) : m_target(_target) {}

		class Iterator
		{
		public:
			Iterator(const Container& _target, SizeType _ind) : m_target(_target), m_index(_ind) {}

			Key key() const { return Accessor::key(m_target, m_index); }
			const Value& value() const { return Accessor::value(m_target, m_index); }

			std::pair<Key, const Value&> operator*() const 
			{ 
				return std::pair<Key, const Value&>(Accessor::key(m_target, m_index), Accessor::value(m_target, m_index));
			}

			Iterator& operator++() { ++m_index; return *this; }
			Iterator operator++(int) { Iterator tmp(*this);  ++m_index; return tmp; }
			bool operator==(const Iterator& _oth) const { ASSERT(&m_target == &_oth.m_target, "Comparing iterators of different containers."); return m_index == _oth.m_index; }
			bool operator!=(const Iterator& _oth) const { ASSERT(&m_target == &_oth.m_target, "Comparing iterators of different containers."); return m_index != _oth.m_index; }
		private:
			const Container& m_target;
			SizeType m_index;
		};

		Iterator begin() const { return Iterator(m_target, 0); }
		Iterator end() const { return Iterator(m_target, m_target.size()); }

	private:
		const Container& m_target;
	};
}