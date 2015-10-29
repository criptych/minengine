////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __MODEL_HPP__
#define __MODEL_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////

struct Vertex {
    sf::Vector2f texCoord;
    sf::Vector3f normal;
    sf::Vector3f position;

    Vertex(
    ) {
    }

    explicit Vertex(
        const sf::Vector3f &position
    ): position(position) {
    }

    Vertex(
        const sf::Vector3f &normal,
        const sf::Vector3f &position
    ): normal(normal), position(position) {
    }

    Vertex(
        const sf::Vector2f &texCoord,
        const sf::Vector3f &position
    ): texCoord(texCoord), position(position) {
    }

    Vertex(
        const sf::Vector2f &texCoord,
        const sf::Vector3f &normal,
        const sf::Vector3f &position
    ): texCoord(texCoord), normal(normal), position(position) {
    }

    Vertex(
        float x, float y, float z
    ): position(x, y, z) {
    }

    Vertex(
        float u, float v, float w,
        float x, float y, float z
    ): normal(u, v, w), position(x, y, z) {
    }

    Vertex(
        float s, float t,
        float x, float y, float z
    ): texCoord(s, t), position(x, y, z) {
    }

    Vertex(
        float s, float t,
        float u, float v, float w,
        float x, float y, float z
    ): texCoord(s, t), normal(u, v, w), position(x, y, z) {
    }

};

////////////////////////////////////////////////////////////////////////////////

class Model {
    uint32_t mPrimitive;
    std::vector<Vertex> mVertices;
    std::vector<uint16_t> mIndices;

public:
    Model();
    explicit Model(uint32_t primitive);

    template <typename I>
    Model(
        uint32_t primitive,
        const I &start,
        const I &end
    ): mPrimitive(primitive) {
        addVertices(start, end);
    }

    template <typename A>
    Model(
        uint32_t primitive,
        const A &array
    ): mPrimitive(primitive) {
        addVertices(array);
    }

    uint32_t getPrimitive() const;

    void setPrimitive(uint32_t primitive);

    const std::vector<Vertex> &getVertices() const;
    std::vector<Vertex> &getVertices();

    void clearVertices();
    void reserveVertices(size_t count);
    void addVertex(const Vertex &vertex);
    void addTriangle(const Vertex &a, const Vertex &b, const Vertex &c);
    void addQuad(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d);

    void addVertices(const Vertex *verts, size_t count);

    template <typename I>
    void addVertices(
        const I &start,
        const I &end
    ) {
        mVertices.insert(mVertices.end(), start, end);
    }

    template <typename A>
    void addVertices(
        const A &array
    ) {
        for (const Vertex &v : array) {
            addVertex(v);
        }
    }

    const std::vector<uint16_t> &getIndices() const;
    std::vector<uint16_t> &getIndices();

    void clearIndices();
    void reserveIndices(size_t count);
    void addIndex(uint16_t index);
    void addTriangle(uint16_t a, uint16_t b, uint16_t c);
    void addQuad(uint16_t a, uint16_t b, uint16_t c, uint16_t d);

    void addIndices(const uint16_t *indices, size_t count);

    template <typename I>
    void addIndices(
        const I &start,
        const I &end
    ) {
        mIndices.insert(mIndices.end(), start, end);
    }

    template <typename A>
    void addIndices(
        const A &array
    ) {
        for (uint16_t v : array) {
            addIndex(v);
        }
    }

    void calcNormals(bool smooth = false);
    void calcNormals(size_t start, size_t end, bool smooth = false);

    void addBox(const sf::Vector3f &size, const sf::Vector3f &center);
    void addBox(const sf::Vector3f &size);
    void addBox();

    void makeBox(const sf::Vector3f &size, const sf::Vector3f &center);
    void makeBox(const sf::Vector3f &size);
    void makeBox();

    void addBall(float radius, size_t step, size_t rstep, const sf::Vector3f &center);
    void addBall(float radius, size_t step, size_t rstep);
    void addBall(float radius, size_t step, const sf::Vector3f &center);
    void addBall(float radius, size_t step);

    void makeBall(float radius, size_t step, size_t rstep, const sf::Vector3f &center);
    void makeBall(float radius, size_t step, size_t rstep);
    void makeBall(float radius, size_t step, const sf::Vector3f &center);
    void makeBall(float radius, size_t step);
};

////////////////////////////////////////////////////////////////////////////////

#endif // __MODEL_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

