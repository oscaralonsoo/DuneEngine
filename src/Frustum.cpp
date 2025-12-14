#include "Frustum.h"
#include <cmath>

void Frustum::Update(const glm::mat4& view, const glm::mat4& projection)
{
    glm::mat4 vp = projection * view;

    mPlanes[Left].normal.x = vp[0][3] + vp[0][0];
    mPlanes[Left].normal.y = vp[1][3] + vp[1][0];
    mPlanes[Left].normal.z = vp[2][3] + vp[2][0];
    mPlanes[Left].d        = vp[3][3] + vp[3][0];

    mPlanes[Right].normal.x = vp[0][3] - vp[0][0];
    mPlanes[Right].normal.y = vp[1][3] - vp[1][0];
    mPlanes[Right].normal.z = vp[2][3] - vp[2][0];
    mPlanes[Right].d        = vp[3][3] - vp[3][0];

    mPlanes[Bottom].normal.x = vp[0][3] + vp[0][1];
    mPlanes[Bottom].normal.y = vp[1][3] + vp[1][1];
    mPlanes[Bottom].normal.z = vp[2][3] + vp[2][1];
    mPlanes[Bottom].d        = vp[3][3] + vp[3][1];

    mPlanes[Top].normal.x = vp[0][3] - vp[0][1];
    mPlanes[Top].normal.y = vp[1][3] - vp[1][1];
    mPlanes[Top].normal.z = vp[2][3] - vp[2][1];
    mPlanes[Top].d        = vp[3][3] - vp[3][1];

    mPlanes[Near].normal.x = vp[0][3] + vp[0][2];
    mPlanes[Near].normal.y = vp[1][3] + vp[1][2];
    mPlanes[Near].normal.z = vp[2][3] + vp[2][2];
    mPlanes[Near].d        = vp[3][3] + vp[3][2];

    mPlanes[Far].normal.x = vp[0][3] - vp[0][2];
    mPlanes[Far].normal.y = vp[1][3] - vp[1][2];
    mPlanes[Far].normal.z = vp[2][3] - vp[2][2];
    mPlanes[Far].d        = vp[3][3] - vp[3][2];

    for (int i = 0; i < Count; ++i)
    {
        float len = glm::length(mPlanes[i].normal);
        if (len > 0.0f)
        {
            mPlanes[i].normal /= len;
            mPlanes[i].d      /= len;
        }
    }
}

bool Frustum::ContainsAABB(const AABB& box) const
{
    for (int p = 0; p < Count; ++p)
    {
        const Plane& plane = mPlanes[p];

        glm::vec3 positiveVertex;
        positiveVertex.x = (plane.normal.x >= 0.0f) ? box.max.x : box.min.x;
        positiveVertex.y = (plane.normal.y >= 0.0f) ? box.max.y : box.min.y;
        positiveVertex.z = (plane.normal.z >= 0.0f) ? box.max.z : box.min.z;

        float dist = plane.Distance(positiveVertex);
        if (dist < 0.0f)
        {
            return false;
        }
    }
    return true;
}
