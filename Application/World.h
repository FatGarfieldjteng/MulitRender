#pragma once

class Scene;
class Camera;

class World
{
public:

    World();

    ~World();

public:
    void SetScene(Scene* scene)
    {
        mScene = scene;
    }

    Scene* GetScene()
    {
        return mScene;
    }

    void SetCamera(Camera* camera)
    {
        mCamera = camera;
    }

    Camera* GetCamera()
    {
        return mCamera;
    }

protected:
    Scene* mScene = nullptr;
    Camera* mCamera = nullptr;
};
