#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <cstddef>  // size_t
#include <array>
#include <cassert>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "pack.h"  // Pack from prep
#include "registry.h"

using EntityID = size_t;

template <typename... ComponentTypes>
class EntityManager {
private:
    // invalid entity ID is the maximum value of size_t
    static constexpr size_t INVALID_COMPONENT_INDEX = std::numeric_limits<size_t>::max();

    // Entity bookkeeping
    std::vector<bool> mEntityValid;  // tracks if an entity is valid
    std::vector<size_t> mFreeEntities; // tracks free entity IDs for reuse

    // Each entity stores one component index per component type
    std::vector<std::array<size_t, sizeof...(ComponentTypes)>> mEntityComponentIndices;
    // One Registry for each component type
    std::tuple<Registry<ComponentTypes>...> mRegistries;
public:
    // Create new entity
    EntityID createEntity() {
        size_t id; // new entity ID
        if (!mFreeEntities.empty()) { // reuse an ID from the free list
            id = mFreeEntities.back();
            mFreeEntities.pop_back();

            mEntityValid[id] = true;
            // reset all component indices for this entity to invalid
            mEntityComponentIndices[id].fill(INVALID_COMPONENT_INDEX);
        } else {
            // no free IDs, create a new one
            id = mEntityValid.size();

            mEntityValid.push_back(true);
            // initialize all component indices for this entity to invalid
            std::array<size_t, sizeof...(ComponentTypes)> indices;
            indices.fill(INVALID_COMPONENT_INDEX);
            // add the new entity's component indices to the vector
            mEntityComponentIndices.push_back(indices);
        }

        return id;
    }

    // Check if an entity exists and is active
    bool validEntity(EntityID id) const {
        // Valid if the ID is within bounds and the corresponding entry in mEntityValid is true
        return id < mEntityValid.size() && mEntityValid[id];
    }

    // Check if an entity has a component
    template <typename ComponentType>
    bool hasComponent(EntityID id) const {
        // Check that the component type is valid and that the entity ID is valid
        static_assert(
            // Check that the component type is one of the types in ComponentTypes...
            Pack<ComponentTypes...>::template contains<ComponentType>,
            "Invalid component type");
        assert(validEntity(id));
        // Get the index of the component type in the pack
        constexpr auto COMPONENT_TYPE_INDEX = Pack<ComponentTypes...>::template index<ComponentType>;
        // Use the index to look up the entity's component ticket
        size_t index = mEntityComponentIndices[id][COMPONENT_TYPE_INDEX];

        if (index == INVALID_COMPONENT_INDEX) {
            return false;
        }
        // Return whether the component at that index is valid in the corresponding registry
        return std::get<COMPONENT_TYPE_INDEX>(mRegistries).valid(index);
    }

    // Get a reference to a component
    template <typename ComponentType>
    ComponentType& getComponent(EntityID id) {
        static_assert(
            Pack<ComponentTypes...>::template contains<ComponentType>,
            "Invalid component type");
        // Ensures the entity actually has the requested component before we return it
        assert(hasComponent<ComponentType>(id));

        constexpr auto COMPONENT_TYPE_INDEX = Pack<ComponentTypes...>::template index<ComponentType>;
        // Use the index to look up the entity's component ticket
        size_t index = mEntityComponentIndices[id][COMPONENT_TYPE_INDEX];
        // Return a reference to the component in the corresponding registry
        return std::get<COMPONENT_TYPE_INDEX>(mRegistries)[index];
    }

    // Add a component to an entity
    template <typename ComponentType>
    void addComponent(EntityID id, ComponentType&& component) {
        // Because ComponentType&& is a forwarding reference, ComponentType can be deduced as a reference type. 
        // remove_reference_t gives us the underlying component type
        using T = std::remove_reference_t<ComponentType>;

        static_assert(
            Pack<ComponentTypes...>::template contains<T>,
            "Invalid component type");
        // Check if the id is valid and the entity does not already have the component
        assert(validEntity(id));
        assert(!hasComponent<T>(id));
        // Get the index of the component type in the pack
        constexpr auto COMPONENT_TYPE_INDEX = Pack<ComponentTypes...>::template index<T>;
        // Store the component in its corresponding registry
        size_t index = std::get<COMPONENT_TYPE_INDEX>(mRegistries).store(
                                    std::forward<ComponentType>(component));
        // Save the returned ticket in the entity's mapping.
        mEntityComponentIndices[id][COMPONENT_TYPE_INDEX] = index;
    }
};

#endif  // ENTITY_MANAGER_H
