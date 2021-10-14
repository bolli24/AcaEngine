#pragma once

namespace utils {
	// Simple type index with some runtime overhead.
	// Use static_type_info::getTypeIndex() instead if only hashes are needed!
	class TypeIndex
	{
		static int s_counter;
	public:
		template<typename T>
		static int value()
		{
			static int id = s_counter++;
			return id;
		}
	};

}