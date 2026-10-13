#include <iostream>
#include "entity_manager.h"

int main() {
    EntityManager<int, float, bool> manager;

    EntityID e = manager.createEntity();

    manager.addComponent(e, 5);

    std::cout << manager.hasComponent<int>(e) << '\n';
    std::cout << manager.hasComponent<float>(e) << '\n';
    std::cout << manager.hasComponent<bool>(e) << '\n';

    std::cout << manager.getComponent<int>(e) << '\n';

    manager.addComponent(e, 3.5f);

    std::cout << manager.getComponent<float>(e) << '\n';
}