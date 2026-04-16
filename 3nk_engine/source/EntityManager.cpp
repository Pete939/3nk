
#include <3nk/Entity.hpp>
#include <3nk/EntityPool.hpp>
#include <3nk/EntityManager.hpp>


Entity EntityManager::Add(const std::string &tag) {
    Entity e = EntityPool::Instance().AddEntity(tag);
    m_Entities[m_NumEntities++] = e;
    return e;
}

EntityManager::EntityManager(uint32_t maxEntities) {
    m_Entities.resize(maxEntities, 0);
}

EntityManager &EntityManager::Instance() {
    static EntityManager manager(10);
    return manager;
}

std::vector<Entity> & EntityManager::GetEntities() {
    return m_Entities;
}
