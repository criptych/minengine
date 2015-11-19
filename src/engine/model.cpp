////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "model.hpp"

#include <SFML/System/VecOps.hpp>

////////////////////////////////////////////////////////////////////////////////

enum GLPrimitive {
    GLPoints,
    GLLines,
    GLLineLoop,
    GLLineStrip,
    GLTriangles,
    GLTriangleStrip,
    GLTriangleFan,
    GLQuads,
    GLQuadStrip,
    GLPolygon,
};

Model::Model(
): mPrimitive(GLPoints) {
}

Model::Model(
    uint32_t primitive
): mPrimitive(primitive) {
}

uint32_t Model::getPrimitive() const {
    return mPrimitive;
}

void Model::setPrimitive(uint32_t primitive) {
    mPrimitive = primitive;
}

const std::vector<Vertex> &Model::getVertices() const {
    return mVertices;
}

std::vector<Vertex> &Model::getVertices() {
    return mVertices;
}

void Model::clearVertices() {
    mVertices.clear();
}

void Model::reserveVertices(size_t count) {
    mVertices.reserve(mVertices.size() + count);
}

void Model::addVertex(const Vertex &vertex) {
    mVertices.push_back(vertex);
}

void Model::addTriangle(const Vertex &a, const Vertex &b, const Vertex &c) {
    addVertex(a);
    addVertex(b);
    addVertex(c);
}

void Model::addQuad(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d) {
    addTriangle(a, b, c);
    addTriangle(c, d, a);
}

void Model::addVertices(const Vertex *verts, size_t count) {
    mVertices.insert(mVertices.end(), verts, verts + count);
}

const std::vector<uint16_t> &Model::getIndices() const {
    return mIndices;
}

std::vector<uint16_t> &Model::getIndices() {
    return mIndices;
}

void Model::clearIndices() {
    mIndices.clear();
}

void Model::reserveIndices(size_t count) {
    mIndices.reserve(mIndices.size() + count);
}

void Model::addIndex(uint16_t vertex) {
    mIndices.push_back(vertex);
}

void Model::addTriangle(uint16_t a, uint16_t b, uint16_t c) {
    addIndex(a);
    addIndex(b);
    addIndex(c);
}

void Model::addQuad(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
    addTriangle(a, b, c);
    addTriangle(c, d, a);
}

void Model::addIndices(const uint16_t *verts, size_t count) {
    mIndices.insert(mIndices.end(), verts, verts + count);
}

void Model::calcNormals(bool smooth) {
    calcNormals(0, mVertices.size(), smooth);
}

void Model::calcNormals(size_t start, size_t end, bool smooth) {
    if (end > mVertices.size()) {
        end = mVertices.size();
    }
    if (start > end) {
        return;
    }

    switch (mPrimitive) {
        case GLTriangles: {
            for (size_t i = start; i < end; i += 3) {
                glm::vec3 p[3], n[3];

                for (size_t j = 0; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }

                for (size_t j = 0; j < 3; j++) {
                    n[j] = normalize(cross(p[(j+1)%3]-p[(j+0)%3],
                                           p[(j+2)%3]-p[(j+0)%3]));
                }

                for (size_t j = 0; j < 3; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLTriangleFan: {
            for (size_t i = start; i < end; i += 1) {
                glm::vec3 p[3], n[3];

                for (size_t j = 1; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }
                p[0] = mVertices[start].position;

                for (size_t j = 0; j < 3; j++) {
                    n[j] = normalize(cross(p[(j+1)%3]-p[(j+0)%3],
                                           p[(j+2)%3]-p[(j+0)%3]));
                }

                for (size_t j = 0; j < 4; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLTriangleStrip: {
            end -= 2;
            for (size_t i = start; i < end; i += 1) {
                glm::vec3 p[3], n[3];

                for (size_t j = 0; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }

                if ((i - start) % 2 == 0) {
                    for (size_t j = 0; j < 3; j++) {
                        n[j] = normalize(cross(p[(j+1)%3]-p[(j+0)%3],
                                               p[(j+2)%3]-p[(j+0)%3]));
                    }
                } else {
                    for (size_t j = 0; j < 3; j++) {
                        n[j] = normalize(cross(p[(j+2)%3]-p[(j+0)%3],
                                               p[(j+1)%3]-p[(j+0)%3]));
                    }
                }

                for (size_t j = 0; j < 3; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }
    }
}

void Model::addPlane(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec4 &texRect) {
    glm::vec3 normal = normalize(cross(c - b, a - b));
    glm::vec3 d = a + c - b;

    glm::vec2 t0(texRect.x, texRect.y);
    glm::vec2 t1(texRect.x + texRect.z, texRect.y + texRect.w);

    switch (mPrimitive) {
        case GLTriangles: {
            reserveIndices(6);
            size_t n = mVertices.size();
            addQuad(n+0, n+1, n+2, n+3);

            reserveVertices(4);
            addVertex(Vertex(glm::vec2(t1.x,t0.y), normal, c));
            addVertex(Vertex(glm::vec2(t1.x,t1.y), normal, d));
            addVertex(Vertex(glm::vec2(t0.x,t1.y), normal, a));
            addVertex(Vertex(glm::vec2(t0.x,t0.y), normal, b));
            break;
        }
    }
}

void Model::addPlane(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
    addPlane(a, b, c, glm::vec4(0, 0, 1, 1));
}

void Model::makePlane(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, const glm::vec4 &texRect) {
    clearVertices();
    clearIndices();
    setPrimitive(GLTriangles);
    addPlane(a, b, c, texRect);
}

void Model::makePlane(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
    makePlane(a, b, c, glm::vec4(0, 0, 1, 1));
}

void Model::addBox(const glm::vec3 &size, const glm::vec3 &center, const glm::vec4 &texRect) {
    glm::vec3 mx = center + size;
    glm::vec3 mn = center - size;

    glm::vec2 t0(texRect.x, texRect.y);
    glm::vec2 t1(texRect.x + texRect.z, texRect.y + texRect.w);

    glm::vec3 a(mn.x, mn.y, mn.z);
    glm::vec3 b(mx.x, mn.y, mn.z);
    glm::vec3 c(mn.x, mx.y, mn.z);
    glm::vec3 d(mx.x, mx.y, mn.z);
    glm::vec3 e(mn.x, mn.y, mx.z);
    glm::vec3 f(mx.x, mn.y, mx.z);
    glm::vec3 g(mn.x, mx.y, mx.z);
    glm::vec3 h(mx.x, mx.y, mx.z);

    addPlane(h, f, b, texRect);
    addPlane(c, a, e, texRect);
    addPlane(c, g, h, texRect);
    addPlane(e, a, b, texRect);
    addPlane(g, e, f, texRect);
    addPlane(d, b, a, texRect);
}

void Model::addBox(const glm::vec3 &size, const glm::vec3 &center) {
    addBox(size, glm::vec3(), glm::vec4(0, 0, 1, 1));
}

void Model::addBox(const glm::vec3 &size, const glm::vec4 &texRect) {
    addBox(size, glm::vec3(), texRect);
}

void Model::addBox(const glm::vec3 &size) {
    addBox(size, glm::vec3());
}

void Model::makeBox(const glm::vec3 &size, const glm::vec3 &center, const glm::vec4 &texRect) {
    clearVertices();
    clearIndices();
    setPrimitive(GLTriangles);
    addBox(size, center, texRect);
}

void Model::makeBox(const glm::vec3 &size, const glm::vec3 &center) {
    makeBox(size, center, glm::vec4(0, 0, 1, 1));
}

void Model::makeBox(const glm::vec3 &size, const glm::vec4 &texRect) {
    makeBox(size, glm::vec3(), texRect);
}

void Model::makeBox(const glm::vec3 &size) {
    makeBox(size, glm::vec3());
}

void Model::addBall(float radius, size_t step, size_t rstep, const glm::vec3 &center) {
    if (step < 2) {
        step = 2; // step < 2 => straight line
    }
    if (rstep < 3) {
        rstep = 2 * step; // rstep < 3 => flat polygon
    }

    float phi = 0, theta = 0, dPhi = Pi / (step), dTheta = 2.0f * Pi / rstep;

    step += 1;
    rstep += 1;

    reserveVertices((step) * (rstep));
    reserveIndices((step) * (rstep) * 2);

    glm::vec3 n;
    size_t i, j, k = mVertices.size();

    for (i = 0; i < rstep; i++, theta += dTheta) {
        for (j = 0, phi = 0; j < step; j++, phi += dPhi) {

            n.x = std::sin(phi)*std::cos(theta);
            n.y = std::cos(phi);
            n.z = std::sin(phi)*std::sin(theta);
            addVertex(Vertex(n, center+n*radius));

            size_t p = (i + 1) % rstep;
            size_t q = (j + 1) % step;
            addQuad(k+(i*step)+j, k+(p*step)+j, k+(p*step)+q, k+(i*step)+q);
        }
    }
}

void Model::addBall(float radius, size_t step, size_t rstep) {
    addBall(radius, step, rstep, glm::vec3());
}

void Model::addBall(float radius, size_t step, const glm::vec3 &center) {
    addBall(radius, step, 2 * step, center);
}

void Model::addBall(float radius, size_t step) {
    addBall(radius, step, glm::vec3());
}

void Model::makeBall(float radius, size_t step, size_t rstep, const glm::vec3 &center) {
    clearVertices();
    clearIndices();
    setPrimitive(GLTriangles);
    addBall(radius, step, rstep, center);
}

void Model::makeBall(float radius, size_t step, size_t rstep) {
    makeBall(radius, step, rstep, glm::vec3());
}

void Model::makeBall(float radius, size_t step, const glm::vec3 &center) {
    makeBall(radius, step, 2 * step, center);
}

void Model::makeBall(float radius, size_t step) {
    makeBall(radius, step, glm::vec3());
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

