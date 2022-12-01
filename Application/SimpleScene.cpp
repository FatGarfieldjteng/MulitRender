#include "framework.h"
#include "SimpleScene.h"
#include "CubeMesh.h"
#include "helper.h"

SimpleScene::SimpleScene()
{

}

SimpleScene::~SimpleScene()
{
	for (auto &mesh : mMeshes)
	{
		if (mesh)
		{
			delete mesh;
		}
	}
}

void SimpleScene::build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	// create a CubeMesh, and add it to the SimpleScene
	CubeMesh *mesh = new CubeMesh();
	mesh->build(GS, commandList);
	
	mMeshes.push_back(mesh);
}

void SimpleScene::endBuild()
{
	for (auto& mesh : mMeshes)
	{
		mesh->endBuild();
	}
}

size_t SimpleScene::mesheCount()
{
	return mMeshes.size();
}

Mesh* SimpleScene::mesh(size_t i)
{
	return mMeshes[i];
}