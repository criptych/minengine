////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __RESOURCECACHE_HPP__
#define __RESOURCECACHE_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////////

template <typename Resource>
class ResourceCache {
    struct CacheEntry {
        std::string name;
        Resource *resource;
        size_t references;
    };
    std::map<std::string, CacheEntry*> mResources;
    std::map<const Resource*, CacheEntry*> mReverseMap;

    size_t mMaxCount;

public:
    ResourceCache(size_t maxCount = 1024);
    ~ResourceCache();

    Resource *acquire(const std::string &name, bool reload = false);
    void release(const Resource *resource);

    void reloadAll();

    void flush();

protected:
    void releaseAll();

    virtual Resource *load(Resource *resource, const std::string &name) = 0;

    CacheEntry *insert(const std::string &name, Resource *resource);
    void remove(CacheEntry *entry);
};

////////////////////////////////////////////////////////////////////////////////

#include <SFML/System/Err.hpp>

////////////////////////////////////////////////////////////////////////////////

template <typename Resource>
ResourceCache<Resource>::ResourceCache(
    size_t maxCount
): mMaxCount(maxCount) {
}

template <typename Resource>
ResourceCache<Resource>::~ResourceCache() {
    releaseAll();
    flush();
}

template <typename Resource>
void ResourceCache<Resource>::releaseAll() {
    for (auto &item : mResources) {
        item.second->references = 0;
    }
}

template <typename Resource>
void ResourceCache<Resource>::flush() {
    for (auto &item : mResources) {
        CacheEntry *entry = item.second;
        if (entry->references == 0) {
            remove(entry);
        }
    }
}

template <typename Resource>
void ResourceCache<Resource>::reloadAll() {
    for (auto &item : mResources) {
        release(acquire(item.second->name, true));
    }
}

template <typename Resource>
Resource *ResourceCache<Resource>::acquire(const std::string &name, bool reload) {
    CacheEntry *entry = nullptr;
    auto found = mResources.find(name);
    if (found == mResources.end()) {
        sf::err() << "Loading resource \"" << name << "\"... ";
        Resource *resource = new Resource;
        if (!load(resource, name)) {
            sf::err() << "error!\n";
            delete resource;
            return nullptr;
        }
        sf::err() << "OK\n";
        entry = insert(name, resource);
    } else {
        entry = found->second;
        if (reload) {
            sf::err() << "Reloading resource \"" << name << "\"... ";
            if (!load(entry->resource, name)) {
                sf::err() << "error!\n";
                return nullptr;
            }
            sf::err() << "OK\n";
        } else {
            sf::err() << "Already loaded resource \"" << name << "\"\n";
        }
    }
    ++entry->references;
    return entry->resource;
}

template <typename Resource>
void ResourceCache<Resource>::release(const Resource *resource) {
    auto found = mReverseMap.find(resource);
    if (found != mReverseMap.end()) {
        CacheEntry *entry = found->second;
        if (entry->references > 0) {
            --entry->references;
        }
    }
}

template <typename Resource>
typename ResourceCache<Resource>::CacheEntry *ResourceCache<Resource>::insert(
    const std::string &name,
    Resource *resource
) {
    CacheEntry *entry = new CacheEntry();
    entry->name = name;
    entry->resource = resource;
    entry->references = 0;
    mResources[name] = entry;
    mReverseMap[resource] = entry;
    return entry;
}

template <typename Resource>
void ResourceCache<Resource>::remove(CacheEntry *entry) {
    mResources.erase(entry->name);
    mReverseMap.erase(entry->resource);
    delete entry->resource;
    delete entry;
}

////////////////////////////////////////////////////////////////////////////////

#endif // __RESOURCECACHE_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

