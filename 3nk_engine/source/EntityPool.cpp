#include <string>
#include <vector>

#include <3nk/EntityPool.hpp>
#include "3nk/Entity.hpp"

EntityPool::EntityPool(uint32_t maxEntities) {
    std::get<std::vector<CTransform> >(mPool).resize(maxEntities);
    std::get<std::vector<CRender> >(mPool).resize(maxEntities);
    // std::get<std::vector<CBoneRenderInfo> >(mPool).resize(maxEntities);
    mActive.resize(maxEntities);
    mTags.resize(maxEntities);
    mNumEntities = 0;
}


EntityPool &EntityPool::Instance() {
    static EntityPool pool(10);
    return pool;
}

Entity EntityPool::AddEntity(const std::string &tag) {
    mActive[mNumEntities] = true;
    mTags[mNumEntities] = tag;
    Entity e{mNumEntities++};
    return e;
}
