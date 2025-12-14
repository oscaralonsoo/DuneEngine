#include "Cubemap.h"
#include "Globals.h"

#include <glad/glad.h>
#include <IL/il.h>

Cubemap::Cubemap(const std::vector<std::shared_ptr<Texture>> &faceTextures) : Resource(ResourceType::Cubemap), mFaceTextures(faceTextures), mID(0)
{
    LoadCubemap();
}

Cubemap::~Cubemap()
{
    if (mID != 0)
    {
        glDeleteTextures(1, &mID);
    }
}

void Cubemap::Bind(GLuint slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
}

void Cubemap::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::LoadCubemap()
{
    glGenTextures(1, &mID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mID);

    for (GLuint i = 0; i < mFaceTextures.size(); i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFaceTextures[i]->GetImageInternalFormat(),
                     mFaceTextures[i]->GetWidth(), mFaceTextures[i]->GetHeight(), 0,
                     mFaceTextures[i]->GetImageFormat(), GL_UNSIGNED_BYTE, mFaceTextures[i]->GetData());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}