#include "framework.h"
#include "CubeMesh.h"
#include <directxmath.h>
#include "helper.h"

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

size_t CubeMesh::vertexSize()
{
    return sizeof(VertexPosColor);
}

void CubeMesh::init(std::shared_ptr<Device> device,
    std::shared_ptr<CommandQueue> commandQueue,
    ComPtr<ID3D12GraphicsCommandList2>)
{
   // mVertexBuffer.
}