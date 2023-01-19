#include "framework.h"
#include "SimpleScene.h"
#include "CubeMesh.h"
#include "helper.h"

SimpleScene::SimpleScene()
{

}

SimpleScene::~SimpleScene()
{

}

void SimpleScene::addNode(Node* node)
{
	mNodes.push_back(node);
}

//void SimpleScene::build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList)
//{
//	// create a CubeMesh, and add it to the SimpleScene
//	CubeMesh *mesh = new CubeMesh();
//	mesh->build(GS, commandList);
//	
//	mMeshes.push_back(mesh);
//}

size_t SimpleScene::nodeCount()
{
	return mNodes.size();
}

Node* SimpleScene::node(size_t i)
{
	return mNodes[i];
}