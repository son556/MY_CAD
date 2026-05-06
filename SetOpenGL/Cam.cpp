#include "Cam.h"

Cam::Cam(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, float near, float far, float fov, float aspect)
    : _pos(pos), _dir(dir), _up(up)
{
    _view = glm::lookAtLH(_pos, _pos + _dir, _up);
    _projection = glm::perspectiveLH(fov, aspect, near, far);
}

void Cam::SetPos(const glm::vec3& pos)
{
    _pos = pos;
    _view = glm::lookAtLH(_pos, _pos + _dir, _up);
}

void Cam::SetDir(const glm::vec3& dir)
{
    if (glm::dot(dir, _up) < 0.00001)
        return;
    _dir = dir;
    _view = glm::lookAtLH(_pos, _pos + _dir, _up);
}
