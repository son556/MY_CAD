#pragma once

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>

class Cam
{
public:
    Cam(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& up, float near, float far, float fov, float aspect);
    ~Cam() = default;
    Cam(const Cam&) = default;
    Cam& operator=(const Cam&) = default;

    void SetPos(const glm::vec3& pos);
    void SetDir(const glm::vec3& dir);
    const glm::mat4& GetViewMat() const { return _view; }
    const glm::mat4& GetProjMat() const { return _projection; }

private:
    Cam() = delete;

    glm::vec3 _pos;
    glm::vec3 _dir;
    glm::vec3 _up;
    
    glm::mat4 _view;
    glm::mat4 _projection;
};

