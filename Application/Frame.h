#pragma once

class FrameData;

class Frame
{
public:

    Frame();

    virtual ~Frame();

public:
    
    void frameCount(unsigned int value);

    void frameIndex(unsigned int value);
    unsigned int frameIndex();

    
    void init();
    void beginFrame();
    void endFrame();


protected:
    FrameData *mFrameData = nullptr;
    unsigned int mFrameCount = 0;
    unsigned int mFrameIndex = 0;
};