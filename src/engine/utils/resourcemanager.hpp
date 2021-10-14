#pragma once

#include "containers/hashmap.hpp"
#include <spdlog/spdlog.h>
#include <string>
#include <cinttypes>
#include <functional>
#include <utility>

namespace utils {
		
	/// Mixin which creates a singleton resource manager for a specific type.
	/// \details To use this manager declare a new type in some header e.g.
	///		using TexMan = ResourceManager<Texture>;.
	///		Then you can access resources TexMan::get("bla.png"). You don't need to unload
	///		resources, but if you want to, there is a clear.
	///		TODO: if a resource is changed during runtime it is reloaded automatically.
	/// \tparam TLoader a type which must have a static load(string...), a static
	///		unload(handle) method and an inner type definition for the handle type:
	///		TLoader::Handle.
	/// \tparam Register A functor that is called with a wrapped clear() method of 
	///		this manager on construction.
	
	template<typename T>
	concept resource_register = requires { T::registerResources(std::function<void()>{}); };
	
	namespace details {
		struct ResourceRegisterDummy { static void registerResources(std::function<void()>) {}; };
	}

	template<typename TLoader, resource_register Register = details::ResourceRegisterDummy>
	class ResourceManager
	{
	public:
		/// Find a resource and load only if necessary.
		/// \param [inout] _args Additional arguments which might be required by
		///		the resource's load() funtion
		template<typename... Args>
		static typename TLoader::Handle get(const char* _name, Args&&... _args);
		
		/// Call to unload all resources. Should always be done on shut-down!
		static void clear();
		
		~ResourceManager();
	private:
		/// Private construction
		ResourceManager();
		
		/// Singleton access
		static ResourceManager& inst();
		
		/// Compute a hash for a string
		struct FastStringHash
		{
			uint32_t operator () (const std::string& _string);
		};

		utils::HashMap<std::string, typename TLoader::Handle, FastStringHash> m_resourceMap;
	};

#define RESOURCE_PATH "../resources/"s

	// ********************************************************************************************* //
	// IMPLEMENTATION																				 //
	// ********************************************************************************************* //
	template<typename TLoader, resource_register Register>
	ResourceManager<TLoader, Register>::ResourceManager()
	{
		Register::registerResources([]() {ResourceManager<TLoader, Register>::clear(); });
	}

	template<typename TLoader, resource_register Register>
	ResourceManager<TLoader, Register>::~ResourceManager()
	{
		clear();
	}

	template<typename TLoader, resource_register Register>
	ResourceManager<TLoader, Register>& ResourceManager<TLoader, Register>::inst()
	{
		static ResourceManager theOnlyInstance;
		return theOnlyInstance;
	}

	template<typename TLoader, resource_register Register>
	template<typename... Args>
	typename TLoader::Handle ResourceManager<TLoader, Register>::get(const char* _name, Args&&... _args)
	{
		using namespace std::string_literals;
		std::string name(RESOURCE_PATH + _name);
		// Search in hash map
		auto handle = inst().m_resourceMap.find(name);
		if(handle) {
		//	pa::logPedantic("Reusing resource '", _name, "'.");
			return handle.data();
		}

		// Add/Load new element
		handle = inst().m_resourceMap.add(move(name), 
			TLoader::load(name.c_str(), 
				std::forward<Args>(_args)...));
		return handle.data();
	}

	template<typename TLoader, resource_register Register>
	void ResourceManager<TLoader, Register>::clear()
	{
		for(auto it : inst().m_resourceMap)
			TLoader::unload(it.data());
		inst().m_resourceMap.clear();
	}

	template<typename TLoader, resource_register Register>
	uint32_t ResourceManager<TLoader, Register>::FastStringHash::operator () (const std::string& _string)
	{
		uint32_t hashvalue = 208357;

		const char* string = _string.c_str();
		while(int c = *string++)
			hashvalue = ((hashvalue << 5) + (hashvalue << 1) + hashvalue) ^ c; 

		return hashvalue;
	}

} // namespace utils
