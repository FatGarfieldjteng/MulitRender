#pragma once
#include "Mesh.h"
#include <d3d12.h>

class CubeMesh : public Mesh
{
public:

    CubeMesh();

    ~CubeMesh();

public:
    virtual void init();
};
