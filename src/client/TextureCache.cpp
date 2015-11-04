////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "TextureCache.hpp"

#include <SFML/System/Err.hpp>

////////////////////////////////////////////////////////////////////////////////

TextureCache::TextureCache(
    size_t maxCount
): mMaxCount(maxCount) {
}

TextureCache::~TextureCache() {
    releaseAll();
    flush();
}

void TextureCache::releaseAll() {
    for (auto &entry : mTextures) {
        entry.second->references = 0;
    }
}

void TextureCache::flush() {
    for (auto &item : mTextures) {
        CacheEntry *entry = item.second;
        if (entry->references == 0) {
            mTextures.erase(entry->name);
            mReverse.erase(entry->texture);
            delete entry->texture;
            delete entry;
        }
    }
}

sf::Texture *TextureCache::acquire(const std::string &name, bool reload) {
    CacheEntry *entry = nullptr;
    auto found = mTextures.find(name);
    if (found == mTextures.end()) {
        sf::Texture *texture = new sf::Texture();
        sf::err() << "Loading texture \"" << name << "\"... ";
        if (!texture->loadFromFile(name)) {
            sf::err() << "error!\n";
            delete texture;
            return nullptr;
        }
        sf::err() << "OK\n";
        entry = new CacheEntry();
        entry->name = name;
        entry->texture = texture;
        entry->references = 0;
        mTextures[name] = entry;
        mReverse[texture] = entry;
    } else {
        entry = found->second;
        if (reload) {
            sf::err() << "Reloading texture \"" << name << "\"... ";
            if (!entry->texture->loadFromFile(entry->name)) {
                sf::err() << "error!\n";
                mTextures.erase(entry->name);
                mReverse.erase(entry->texture);
                delete entry->texture;
                delete entry;
                return nullptr;
            }
            sf::err() << "OK\n";
        } else {
            sf::err() << "Already loaded texture \"" << name << "\"\n";
        }
    }
    ++entry->references;
    return entry->texture;
}

void TextureCache::release(const sf::Texture *texture) {
    auto found = mReverse.find(texture);
    if (found != mReverse.end()) {
        CacheEntry *entry = found->second;
        if (entry->references > 0) {
            --entry->references;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

