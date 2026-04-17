#include <string>
#include <vector>

#include <3nk/EntityPool.hpp>
#include "3nk/Entity.hpp"

EntityPool::EntityPool(uint32_t maxEntities) {
    std::get<std::vector<CTransform> >(m_Pool).resize(maxEntities);
    std::get<std::vector<CRender> >(m_Pool).resize(maxEntities);
    // std::get<std::vector<CBoneRenderInfo> >(mPool).resize(maxEntities);
    m_Active.resize(maxEntities);
    m_Tags.resize(maxEntities);
    m_NumEntities = 0;
}


EntityPool &EntityPool::Instance() {
    static EntityPool pool(10);
    return pool;
}

Entity EntityPool::AddEntity(const std::string &tag) {
    m_Active[m_NumEntities] = true;
    m_Tags[m_NumEntities] = tag;
    Entity e{m_NumEntities++};
    return e;
}
