#pragma once

#include <cstdint>
#include <utility>

#include "EntityPool.hpp"

class EntityManager;
class EntityPool;

class Entity {
    friend EntityPool;
    friend EntityManager;
    uint32_t m_Id;
    // Entity() = delete;
    // ~Entity() = delete;
    Entity(const uint32_t id);
public:
    template <typename T>
    T& Get();

    template <typename T>
    bool Has() const;

    template <typename T>
    T& Add();

    template <typename T, typename... Args>
    T& Add(Args&&... args);

    void OnInspector();
};

template <typename T>
T& Entity::Get() {
    return EntityPool::Instance().GetComponent<T>(m_Id);
}

template <typename T>
bool Entity::Has() const {
    return EntityPool::Instance().HasComponent<T>(m_Id);
}

template <typename T>
T& Entity::Add() {
    return EntityPool::Instance().AddComponent<T>(m_Id);
}

template <typename T, typename... Args>
T& Entity::Add(Args&&... args) {
    return EntityPool::Instance().AddComponent<T>(m_Id, std::forward<Args>(args)...);
}
