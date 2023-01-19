#pragma once
#include <d3d12.h>
#include <vector>

class Node;

class Scene
{
public:

    Scene();

    ~Scene();

public:

    virtual void addNode(Node* node) = 0;

    virtual size_t nodeCount() = 0;

    virtual Node* node(size_t i) = 0;
};
