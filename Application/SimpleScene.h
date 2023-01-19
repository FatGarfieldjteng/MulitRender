#pragma once
#include "Scene.h"
#include <d3d12.h>
#include <vector>

class Node;

class SimpleScene : public Scene
{
public:

    SimpleScene();

    ~SimpleScene();

public:

    virtual void addNode(Node* ndoe) override;

    virtual size_t nodeCount() override;

    virtual Node* node(size_t i) override;
    
private:
    std::vector <Node*> mNodes;
};
