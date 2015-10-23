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
    mVertices.reserve(count);
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
    mIndices.reserve(count);
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

void Model::setColor(const sf::Color &color) {
    for (Vertex &v : mVertices) {
        v.color = color;
    }
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

    //~ sf::err() << "start == " << start << ", end == " << end << '\n';

    switch (mPrimitive) {
        case GLTriangles: {
            for (size_t i = start; i < end; i += 3) {
                sf::Vector3f p[3], n[3];

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
                sf::Vector3f p[3], n[3];

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
                sf::Vector3f p[3], n[3];

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

        case GLQuads: {
            for (size_t i = start; i < end; i += 4) {
                sf::Vector3f p[4], n[4];

                for (size_t j = 0; j < 4; j++) {
                    p[j] = mVertices[i+j].position;
                }

                for (size_t j = 0; j < 4; j++) {
                    n[j] = normalize(cross(p[(j+1)%4]-p[(j+0)%4],
                                           p[(j+3)%4]-p[(j-0)%4]));
                }

                for (size_t j = 0; j < 4; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLQuadStrip: {
            end -= 2;
            for (size_t i = start; i < end; i += 2) {
                sf::Vector3f p[4], n[4];

                for (size_t j = 0; j < 4; j++) {
                    p[j] = mVertices[i+j].position;
                }

                for (size_t j = 0; j < 4; j++) {
                    n[j] = normalize(cross(p[(j+1)%4]-p[(j+0)%4],
                                           p[(j+3)%4]-p[(j-0)%4]));
                }

                for (size_t j = 0; j < 4; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }
    }

    //~ sf::err() << std::flush;
}

void Model::addBox(const sf::Vector3f &size, const sf::Vector3f &center) {
    sf::Vector3f mx = center + size;
    sf::Vector3f mn = center - size;

    switch (mPrimitive) {
        case GLTriangles: {
            reserveIndices(mIndices.size() + 36);

            size_t n = mVertices.size();
            addQuad(n+ 0, n+ 1, n+ 2, n+ 3);
            addQuad(n+ 4, n+ 5, n+ 6, n+ 7);
            addQuad(n+ 8, n+ 9, n+10, n+11);
            addQuad(n+12, n+13, n+14, n+15);
            addQuad(n+16, n+17, n+18, n+19);
            addQuad(n+20, n+21, n+22, n+23);

            // continue
        }

        case GLQuads: {
            reserveVertices(mVertices.size() + 24);

            addVertex(Vertex(0x7fff,0x0000,  1, 0, 0, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff,  1, 0, 0, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff,  1, 0, 0, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x0000,  1, 0, 0, mx.x,mn.y,mn.z));

            addVertex(Vertex(0x0000,0x0000, -1, 0, 0, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, -1, 0, 0, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, -1, 0, 0, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, -1, 0, 0, mn.x,mx.y,mn.z));

            addVertex(Vertex(0x0000,0x0000,  0, 1, 0, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff,  0, 1, 0, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff,  0, 1, 0, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000,  0, 1, 0, mx.x,mx.y,mn.z));

            addVertex(Vertex(0x7fff,0x0000,  0,-1, 0, mx.x,mn.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff,  0,-1, 0, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff,  0,-1, 0, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x0000,  0,-1, 0, mn.x,mn.y,mn.z));

            addVertex(Vertex(0x7fff,0x0000,  0, 0, 1, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff,  0, 0, 1, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff,  0, 0, 1, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x0000,  0, 0, 1, mn.x,mn.y,mx.z));

            addVertex(Vertex(0x0000,0x0000,  0, 0,-1, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff,  0, 0,-1, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff,  0, 0,-1, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x0000,  0, 0,-1, mx.x,mn.y,mn.z));

            break;
        }
    }
}

void Model::addBox(const sf::Vector3f &size) {
    addBox(size, sf::Vector3f());
}

void Model::addBox() {
    addBox(sf::Vector3f(1,1,1));
}

void Model::makeBox(const sf::Vector3f &size, const sf::Vector3f &center) {
    clearVertices();
    clearIndices();
    setPrimitive(GLTriangles);
    addBox(size, center);
}

void Model::makeBox(const sf::Vector3f &size) {
    makeBox(size, sf::Vector3f());
}

void Model::makeBox() {
    makeBox(sf::Vector3f(1,1,1));
}

void Model::addBall(float radius, size_t step, size_t rstep, const sf::Vector3f &center) {
    if (step < 2) {
        step = 2; // step < 2 => straight line
    }
    if (rstep < 3) {
        rstep = 2 * step; // rstep < 3 => flat polygon
    }

    float phi = 0, theta = 0, dPhi = Pi / (step), dTheta = 2.0f * Pi / rstep;

    step += 1;
    rstep += 1;

    reserveVertices(mVertices.size() + (step) * (rstep));
    reserveIndices(mIndices.size() + (step) * (rstep) * 2);

    sf::Vector3f n;
    size_t i, j, k = mVertices.size();

    for (i = 0; i < rstep; i++, theta += dTheta) {
        for (j = 0, phi = 0; j < step; j++, phi += dPhi) {

            n.x = std::sin(phi)*std::cos(theta);
            n.y = std::cos(phi);
            n.z = std::sin(phi)*std::sin(theta);
            addVertex(Vertex(n, n*radius));

            size_t p = (i + 1) % rstep;
            size_t q = (j + 1) % step;
            addQuad(k+(i*step)+j, k+(p*step)+j, k+(p*step)+q, k+(i*step)+q);
        }
    }
}

void Model::addBall(float radius, size_t step, size_t rstep) {
    addBall(radius, step, rstep, sf::Vector3f());
}

void Model::addBall(float radius, size_t step, const sf::Vector3f &center) {
    addBall(radius, step, 2 * step, center);
}

void Model::addBall(float radius, size_t step) {
    addBall(radius, step, sf::Vector3f());
}

void Model::makeBall(float radius, size_t step, size_t rstep, const sf::Vector3f &center) {
    clearVertices();
    clearIndices();
    setPrimitive(GLTriangles);
    addBall(radius, step, rstep, center);
}

void Model::makeBall(float radius, size_t step, size_t rstep) {
    makeBall(radius, step, rstep, sf::Vector3f());
}

void Model::makeBall(float radius, size_t step, const sf::Vector3f &center) {
    makeBall(radius, step, 2 * step, center);
}

void Model::makeBall(float radius, size_t step) {
    makeBall(radius, step, sf::Vector3f());
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////
