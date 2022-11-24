#include "framework.h"
#include "Effect.h"
#include "helper.h"

Effect::Effect(std::shared_ptr<Device> device)
	:mDevice(device)
{

}

Effect::~Effect()
{

}

void Effect::init()
{
	loadShader();
	createInputLayout();
}
