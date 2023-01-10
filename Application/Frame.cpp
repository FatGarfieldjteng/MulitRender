#include "framework.h"
#include "Frame.h"
#include "FrameData.h"

Frame::Frame()
{
	mFrameData = new FrameData();
}

Frame::~Frame()
{
	delete mFrameData;
}

void Frame::frameCount(unsigned int value)
{
	mFrameCount = value;
}

void Frame::frameIndex(unsigned int value)
{
	mFrameIndex = value;
}

unsigned int Frame::frameIndex()
{
	return mFrameIndex;
}

void Frame::init()
{

}

void Frame::beginFrame()
{

}

void Frame::endFrame()
{

}