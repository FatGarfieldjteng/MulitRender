#include "framework.h"
#include "ShadowPass.h"
#include "Device.h"
#include "RenderTask.h"

ShadowPass::ShadowPass(std::shared_ptr<Device> device)
	:RenderPass(device)

{
}

ShadowPass::~ShadowPass()
{
	
}

void ShadowPass::buildTasks()
{
	// compute renderTask parameters according to meshes in scene

	mRenerTask = new RenderTask(100, 10);
}

void ShadowPass::preprocess()
{

}

void ShadowPass::execute()
{

}

void ShadowPass::postprocess()
{

}