#pragma once

#include "Model.hpp"
#include "Components.hpp"

class Entity;

class EntityPool {
    uint32_t m_NumEntities;
    std::tuple<
        std::vector<CTransform>,
        std::vector<CRender>
        // std::vector<CBoneRenderInfo>
    > m_Pool;
    std::vector<bool> m_Active;
    std::vector<std::string> m_Tags;

    EntityPool(uint32_t maxEntities);

public:
    static EntityPool & Instance();

    Entity AddEntity(const std::string &tag);

    template<typename T>
    T& GetComponent(uint32_t entityId);

    template<typename T>
    bool HasComponent(uint32_t entityId);

    template<typename T>
    T& AddComponent(uint32_t entityId);

    template<typename T, typename... Args>
    T& AddComponent(uint32_t entityId, Args&&... args);
};

template<typename T>
T &EntityPool::GetComponent(uint32_t entityId) {
    return std::get<std::vector<T>>(m_Pool)[entityId];
}

template<typename T>
bool EntityPool::HasComponent(uint32_t entityId) {
    return std::get<std::vector<T>>(m_Pool)[entityId].mActive;
}

template<typename T>
T &EntityPool::AddComponent(const uint32_t entityId) {
    auto &component = GetComponent<T>(entityId);
    // component = T{};
    component.mActive = true;
    return component;
}

template<typename T, typename... Args>
T &EntityPool::AddComponent(const uint32_t entityId, Args &&... args) {
    auto &component = GetComponent<T>(entityId);
    component = T(std::forward<Args>(args)...);
    component.mActive = true;
    return component;
}
