#pragma once
#include <any>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <vector>

struct Entity {
    uint32_t id;
};

struct EntityRef {
    Entity entity;
    uint32_t generation;
};

template <class T>
concept component_type = std::movable<T> && std::is_trivially_destructible_v<T>;

template <component_type Component>
class ComponentAccess {
   public:
    ComponentAccess() : componentSize(sizeof(Component)) {}

    // Add a new component to an existing entity. No changes are done if Component
    // if _ent already has a component of this type.
    // @return A reference to the new component or the already existing component.
    Component& insert(Entity _ent, const Component& _comp) {
        if (sparse.size() > _ent.id && sparse[_ent.id] != -1) {
            return *at(_ent);
        }

        while (_ent.id >= sparse.size()) {  // Expand sparse array to fit entity id
            sparse.push_back(-1);
        }

        sparse[_ent.id] = entities.size();
        entities.push_back(_ent);
        buffer.resize(componentSize * entities.size());
        Component* component = at(_ent);
        *component = _comp;

        return *component;
    }

    // Retrieve a component of _ent.
    // @return A pointer to the associated component or nullptr if it does not exist.
    Component* at(Entity _ent) {
        if (_ent.id >= sparse.size() || sparse[_ent.id] == -1) return nullptr;
        return reinterpret_cast<Component*>(buffer.data()) + sparse[_ent.id];
    }

    const Component* at(Entity _ent) const {
        return at(_ent);
    }

    // BONUS:
    Component& operator[](Entity _ent) {
        return *at(_ent);
    }

    // Remove a component from an existing entity.
    // Does not check whether it exists.                ?? Was ist it ??
    void erase(Entity _ent) {
        if (_ent.id >= sparse.size() || sparse[_ent.id] == -1) return;

        Component* toRemove = at(_ent.id);
        Component* last = at(entities.size() - 1);

        int position = sparse[_ent.id];
        sparse[entities[entities.size() - 1]] = position;
        sparse[_ent.id] = -1;

        entities[position] = entities[entities.size() - 1];
        entities.pop_back();

        *toRemove = *last;
        buffer.resize(componentSize * (entities.size() - 1) * 4);
    }

   private:
    size_t componentSize;
    std::vector<uint32_t> sparse;
    std::vector<Entity> entities;
    std::vector<char> buffer;
};

class Registry {
   public:
    Entity create() {
        uint32_t i;

        if (unusedIds.size() > 0) {
            i = unusedIds.back();
            unusedIds.pop_back();
            flags[i] = true;
            generations[i]++;
            return {i};
        }
        flags.push_back(true);
        generations.push_back(1);
        return {(uint32_t)flags.size() - 1};
    };

    void erase(Entity _ent) {
        flags[_ent.id] = false;
        // data.erase(_ent.id);
        unusedIds.push_back(_ent.id);
    };

    EntityRef getRef(Entity _ent) const {
        return {_ent, generations[_ent.id]};
    };

    std::optional<Entity> getEntity(EntityRef _ent) const {
        if (!flags[_ent.entity.id]) {
            return {};
        } else if (_ent.generation != generations[_ent.entity.id]) {
            return {};
        } else {
            return _ent.entity;
        }
    };

    template <component_type Component>
    ComponentAccess<Component>& getComponents() {
        if (componentsMap.contains(std::type_index(typeid(Component)))) {
            return std::any_cast<ComponentAccess<Component>&>(componentsMap[std::type_index(typeid(Component))]);
        }

        ComponentAccess<Component> componentAccess;
        componentsMap[std::type_index(typeid(Component))] = componentAccess;
        return std::any_cast<ComponentAccess<Component>&>(componentsMap[std::type_index(typeid(Component))]);
    };

    template <component_type Component>
    const std::optional<ComponentAccess<Component>&> getComponents() const {
        return getComponents<Component>();
    };

    // Execute an Action on all entities having the components
    // expected by Action::operator(component_type&...).
    // In addition, the entity itself is provided if
    // the first parameter is of type Entity.
    template <typename Action>
    void execute(const Action& _action){};

   private:
    std::unordered_map<std::type_index, std::any> componentsMap;

    std::vector<bool> flags;
    std::vector<uint32_t> unusedIds;
    std::vector<uint32_t> generations;
};