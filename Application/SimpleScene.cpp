#include "framework.h"
#include "SimpleScene.h"
#include "CubeMesh.h"
#include "helper.h"

SimpleScene::SimpleScene(GraphicsSystem* GS)
	:Scene(GS)
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

void SimpleScene::init()
{
	// create a CubeMesh, and add it to the SimpleScene
	CubeMesh *mesh = new CubeMesh(mGS);
	mesh->init();

	mMeshes.push_back(mesh);
}