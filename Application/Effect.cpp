#include "framework.h"
#include "Effect.h"
#include "helper.h"

Effect::Effect()

{

}

Effect::~Effect()
{

}

void Effect::build(std::shared_ptr<Device> device)
{
	loadShader();
	createRootSignature(device);
	createPipelineStateObject(device);
}
