#pragma once
#include <string>
#include <vector>

class Entity;
#include <cstdint>

class EntityManager {
    std::vector<Entity> m_Entities;
    uint32_t m_NumEntities = 0;
    EntityManager(uint32_t maxEntities);
public:
    Entity Add(const std::string &tag);
    static EntityManager &Instance();
    std::vector<Entity> &GetEntities();
    uint32_t GetNumEntities() const { return m_NumEntities; }
};
