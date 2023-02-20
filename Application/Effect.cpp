#include "framework.h"
#include "Effect.h"
#include "helper.h"

Effect::Effect()

{

}

Effect::~Effect()
{

}

void Effect::setVS(ComPtr<ID3DBlob> VSBlob)
{
	mVertexShaderBlob = VSBlob;
}

void Effect::setPS(ComPtr<ID3DBlob> PSBlob)
{
	mPixelShaderBlob = PSBlob;
}

ComPtr<ID3DBlob> Effect::getVS()
{
	return mVertexShaderBlob;
}

ComPtr<ID3DBlob> Effect::getPS()
{
	return mPixelShaderBlob;
}

void Effect::build(std::shared_ptr<Device> device)
{
	loadShader();
}
