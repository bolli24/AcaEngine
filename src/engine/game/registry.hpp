#pragma once
#include <any>
#include <cstdint>
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

        if (sparse.size() <= _ent.id)
            sparse.resize(_ent.id + 1, -1);  // Expand sparse array to fit entity id

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
        if (_ent.id >= sparse.size() || sparse[_ent.id] == -1) return nullptr;
        return reinterpret_cast<const Component*>(buffer.data()) + sparse[_ent.id];
    }

    // BONUS:
    Component& operator[](Entity _ent) {
        return *at(_ent);
    }

    bool hasEntity(Entity _ent) {
        return !(_ent.id >= sparse.size() || sparse[_ent.id] == -1);
    }

    // Remove a component from an existing entity.
    // Does not check whether it exists.                ?? Was ist it ??
    void erase(Entity _ent) {
        if (_ent.id >= sparse.size() || sparse[_ent.id] == -1) return;

        Component& toRemove = *at(_ent);                        // TODO: fix move
        toRemove = *at(entities[entities.size() - 1]);

        int position = sparse[_ent.id];
        sparse[entities[entities.size() - 1].id] = position;    // TODO: This line throws a VS overflow warning !?
        sparse[_ent.id] = -1;

        entities[position] = entities[entities.size() - 1];
        entities.pop_back();

        size_t newSize = componentSize * entities.size();
        buffer.resize(newSize);
    }

    std::vector<Entity> getEntities() const {
        return entities;
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

        const int test = 2;

        for (auto& element : componentsMap) {
            ComponentAccess<char>& componentAccess = reinterpret_cast<ComponentAccess<char>&>(element.second);
            componentAccess.erase(_ent);
        }

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
            return reinterpret_cast<ComponentAccess<Component>&>(componentsMap[std::type_index(typeid(Component))]);
        }

        ComponentAccess<Component> componentAccess;
        componentsMap[std::type_index(typeid(Component))] = *reinterpret_cast<ComponentAccess<char>*>(&componentAccess);
        return reinterpret_cast<ComponentAccess<Component>&>(componentsMap[std::type_index(typeid(Component))]);
    };

    template <component_type Component>
    const ComponentAccess<Component>& getComponents() const {
        return reinterpret_cast<const ComponentAccess<Component>&>(componentsMap.find(std::type_index(typeid(Component)))->second);
    };

    // Execute an Action on all entities having the components
    // expected by Action::operator(component_type&...).
    // In addition, the entity itself is provided if
    // the first parameter is of type Entity.
    template <typename... Components, typename Action>
    void execute(const Action& _action) {
        std::vector<std::type_index> componentTypes = {std::type_index(typeid(Components))...};

        int startIndex = 0;

        if (componentTypes[0] == std::type_index(typeid(Entity))) {
            startIndex = 1;
        }

        if (!componentsMap.contains(componentTypes[startIndex])) return;
        std::vector<Entity> entities = componentsMap[componentTypes[startIndex]].getEntities();

        for (int i = startIndex + 1; i < componentTypes.size(); i++) {
            if (!componentsMap.contains(componentTypes[i])) return;
            auto it = std::remove_if(entities.begin(), entities.end(), [&](Entity ent) {
                return !componentsMap.at(componentTypes[i]).hasEntity(ent);
            });
            entities.erase(it, entities.end());
        }

        for (Entity entity : entities) {
            helper<Components...>(entity, _action, std::tie());
        }
    };

    template <typename Component, typename... Rest, typename Action, typename Args>
    void helper(Entity entity, const Action& action, Args args) {
        if constexpr (std::is_same<Entity, Component>::value) {
            auto allArgs = std::tuple_cat(args, std::tie(entity));
            if constexpr (sizeof...(Rest) > 0)
                helper<Rest...>(entity, action, allArgs);
            else
                std::apply(action, allArgs);

        } else {
            Component& component = getComponents<Component>()[entity];
            auto allArgs = std::tuple_cat(args, std::tie(component));
            if constexpr (sizeof...(Rest) > 0)
                helper<Rest...>(entity, action, allArgs);
            else
                std::apply(action, allArgs);
        }
    }

   private:
    std::unordered_map<std::type_index, ComponentAccess<char>> componentsMap;

    std::vector<bool> flags;
    std::vector<uint32_t> unusedIds;
    std::vector<uint32_t> generations;
};