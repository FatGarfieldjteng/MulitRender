#include "framework.h"
#include "CubeMesh.h"
#include "GraphicsSystem.h"
#include "helper.h"

#include <directxmath.h>

typedef DirectX::XMFLOAT3 DXfloat3;

// data private to CubeMesh
namespace{
    
    struct VertexPosColor
    {
        DXfloat3 Position;
        DXfloat3 Color;
    };

    static VertexPosColor vertices[8] = {
        { DXfloat3(-1.0f, -1.0f, -1.0f), DXfloat3(0.0f, 0.0f, 0.0f) },  // 0
        { DXfloat3(-1.0f,  1.0f, -1.0f), DXfloat3(0.0f, 1.0f, 0.0f) },  // 1
        { DXfloat3(1.0f,  1.0f, -1.0f), DXfloat3(1.0f, 1.0f, 0.0f) },   // 2
        { DXfloat3(1.0f, -1.0f, -1.0f), DXfloat3(1.0f, 0.0f, 0.0f) },   // 3
        { DXfloat3(-1.0f, -1.0f,  1.0f), DXfloat3(0.0f, 0.0f, 1.0f) },  // 4
        { DXfloat3(-1.0f,  1.0f,  1.0f), DXfloat3(0.0f, 1.0f, 1.0f) },  // 5
        { DXfloat3(1.0f,  1.0f,  1.0f), DXfloat3(1.0f, 1.0f, 1.0f) },   // 6
        { DXfloat3(1.0f, -1.0f,  1.0f), DXfloat3(1.0f, 0.0f, 1.0f) }    // 7
    };

    static WORD indicies[36] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };
}

CubeMesh::CubeMesh()
{

}

CubeMesh::~CubeMesh()
{

}

void CubeMesh::build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    // create vertex buffer
    // upload vertex buffer data.
    GS->updateBufferResource(commandList,
        &mVertexBuffer.mBuffer, &mIntermediateVertexBuffer,
        _countof(vertices), sizeof(VertexPosColor), vertices);

    // setup vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW& vbView = mVertexBuffer.mVertexBufferView;
    vbView.BufferLocation = mVertexBuffer.mBuffer->GetGPUVirtualAddress();
    vbView.SizeInBytes = sizeof(vertices);
    vbView.StrideInBytes = sizeof(VertexPosColor);

    // create index buffer
    // upload index buffer data.
    GS->updateBufferResource(commandList,
        &mIndexBuffer.mBuffer, &mIntermediateIndexBuffer,
        _countof(indicies), sizeof(WORD), indicies);
    
    D3D12_INDEX_BUFFER_VIEW& ibView = mIndexBuffer.mIndexBufferView;
    ibView.BufferLocation = mIndexBuffer.mBuffer->GetGPUVirtualAddress();
    ibView.Format = DXGI_FORMAT_R16_UINT;
    ibView.SizeInBytes = sizeof(indicies);
       
}