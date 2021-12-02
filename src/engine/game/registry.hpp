#pragma once
#include <unordered_map>
#include <vector>

struct Entity {
    uint32_t id;
};

struct EntityRef {
    Entity entity;
    uint32_t generation;
};

template <typename T>
class Registry {
   public:
    Entity create() {
        uint32_t i;
        for (i = 0; i < flags.size(); i++) {
            if (!flags[i]) {
                flags[i] = true;
                generations[i]++;
                return {i};
            }
        }
        flags.push_back(true);
        generations.push_back(1);
        return {i + 1};
    };

    void erase(Entity _ent) {
        flags[_ent.id] = false;
        data.erase(_ent.id);
    };

    EntityRef getRef(Entity _ent) const {
        return {_ent, generations[_ent.id]};
    };

    std::optional<Entity> getEntity(EntityRef _ent) const {
        if (_ent.generation != generations[_ent.entity.id]) {
            return {};
        } else {
            return _ent.entity;
        }
    };

    void setData(Entity _ent, const T& _value) {
        data[_ent.id] = _value;
    };

    T& getData(Entity _ent) {
        return data[_ent.id];
    };

    template <typename FN, typename... Args>
    void execute(FN _fn, Args&... args) {
        for (auto& element : data) {
            _fn(element.second, args...);
        }
    };

   private:
    std::unordered_map<int, T> data;
    std::vector<bool> flags;
    std::vector<uint32_t> generations;
};