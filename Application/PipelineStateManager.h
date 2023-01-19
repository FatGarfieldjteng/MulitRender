#pragma once

#include <map>
#include <string>

class PipelineState;

class PipelineStateManager
{
public:

    PipelineStateManager();

    ~PipelineStateManager();

public:
    void addPipelineState(const std::string& ID, PipelineState* pipelineState);

    PipelineState* pipelineState(const std::string& ID);

private:
    std::map<std::string, PipelineState*> mStringToPipelineState;
};
