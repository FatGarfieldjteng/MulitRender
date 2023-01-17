#include "framework.h"
#include "BeautyPass.h"
#include "Device.h"

BeautyPass::BeautyPass(std::shared_ptr<Device> device)
	:RenderPass(device)

{
}

BeautyPass::~BeautyPass()
{

}

void BeautyPass::buildTasks()
{

}

void BeautyPass::preprocess()
{
	//
}

void BeautyPass::execute()
{

}

void BeautyPass::postprocess()
{

}