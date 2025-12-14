#pragma once

#include "Resource.h"
#include "Texture.h"

class Cubemap : public Resource
{
public:
    Cubemap(const std::vector<std::shared_ptr<Texture>> &faceTextures);
    ~Cubemap();

    void Bind(GLuint slot = 0) const;
    void Unbind() const;

    GLuint GetID() const { return mID; }

private:
    void LoadCubemap();

private:
    GLuint mID;
    std::vector<std::shared_ptr<Texture>> mFaceTextures;
};