#pragma once

class Scene;
class Camera;

class World
{
public:

    World();

    ~World();

public:
    void setScene(Scene* scene)
    {
        mScene = scene;
    }

    Scene* getScene()
    {
        return mScene;
    }

    void setCamera(Camera* camera)
    {
        mCamera = camera;
    }

    Camera* getCamera()
    {
        return mCamera;
    }

protected:
    Scene* mScene = nullptr;
    Camera* mCamera = nullptr;
};
