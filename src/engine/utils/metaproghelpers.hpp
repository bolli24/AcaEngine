#pragma once

namespace utils {

	// Allows access to a functor's signature through template argument deduction.
	// For a usage example see Registry::execute().
	template<typename T, typename... Args>
	struct UnpackFunction
	{
		UnpackFunction(void(T::*)(Args ...) const) {};
		UnpackFunction(void(T::*)(Args ...)) {};
	};

	// Helper for type deduction that does not impose any conditions on T.
	template<typename T>
	struct TypeHolder
	{
		using type = T;
	};

	// Determine whether a parameter pack contains a specific type.
	template<typename What, typename ... Args>
	struct contains_type
	{
		static constexpr bool value{ (std::is_same_v<What, Args> || ...) };
	};

	// Version that handles a tuple.
	template<typename What, typename... Args>
	struct contains_type<What, std::tuple<Args...>>
	{
		static constexpr bool value{ (std::is_same_v<What, Args> || ...) };
	};

	template<typename What, typename ... Args>
	constexpr bool contains_type_v = contains_type<What, Args...>::value;
}