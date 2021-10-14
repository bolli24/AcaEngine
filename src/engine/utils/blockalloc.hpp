#pragma once

#include <utility>
#include <memory>

namespace utils {
	/// @brief A simple allocator which maintains memory blocks holding multiple 
	///		elements of the same type.
	/// @param T The type of objects to handle.
	/// @param ElemPerBlock Number elements to hold in a single memory block.
	///		A larger value leads to fewer allocations but more wasted space if 
	///		the lifetimes differ.
	template<typename T, int ElemPerBlock>
	class BlockAllocator
	{
	public:
		BlockAllocator()
			: first(new Node()), current(first.get())
		{

		}

		/// @brief Create a new object.
		/// Arguments are forwarded to the constructor of T.
		template<typename... Args>
		T* create(Args&&... args)
		{
			if (current->numElements == ElemPerBlock)
			{
				Node* prev = current;
				current = new Node();
				prev->next = current;
			}
			char* ptr = current->buffer + current->numElements * sizeof(T);
			++current->numElements;

			return new (ptr) T (std::forward<Args>(args)...);
		}

		// Delete all objects and free all but one block.
		void reset()
		{
			first.reset(new Node());
			current = first.get();
		}

	private:
		struct Node
		{
			~Node()
			{
				T* ptr = reinterpret_cast<T*>(buffer);
				for (int i = 0; i < numElements; ++i)
					ptr[i].~T();

				if (next) delete next;
			}

			char buffer[sizeof(T) * ElemPerBlock];
			Node* next = nullptr;
			int numElements = 0;
		};

		std::unique_ptr<Node> first;
		Node* current;
	};
}