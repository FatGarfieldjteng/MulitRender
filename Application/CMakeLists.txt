cmake_minimum_required(VERSION 3.16.1)

set(TARGET_NAME MultiRenderApp)

set(HEADER_FILES
	ConfigParameter.h 
	Helper.h
	framework.h
	Adapter.h
	Device.h
	SwapChain.h
	Graphicssystem.h
	NextGenRender.h
	CommandQueue.h
	Mesh.h
	CubeMesh.h
    Camera.h
	GraphicsResource.h
    BufferResource.h
	VertexBuffer.h
	IndexBuffer.h
	Scene.h
	SimpleScene.h
	Effect.h
	SimpleEffect.h
	UploadBuffer.h
	CPUDescriptorAllocator.h
	CPUDescriptorPage.h
	CPUDescriptorAllocation.h
	ViewManager.h
	RootSignature.h
	CommandList.h
	ResourceStateTracker.h
	d3dx12.h
	targetver.h
)

set(SOURCE_FILES 
	Adapter.cpp
	Device.cpp
	SwapChain.cpp
	GraphicsSystem.cpp
	NextGenRender.cpp
	CommandQueue.cpp
	Mesh.cpp
	CubeMesh.cpp
	Camera.cpp
	GraphicsResource.cpp
	BufferResource.cpp
	VertexBuffer.cpp
	IndexBuffer.cpp
	Scene.cpp
	SimpleScene.cpp
	Effect.cpp
	SimpleEffect.cpp
	UploadBuffer.cpp
	CPUDescriptorAllocator.cpp
	CPUDescriptorPage.cpp
	CPUDescriptorAllocation.cpp
	ViewManager.cpp
	RootSignature.cpp
	CommandList.cpp
	ResourceStateTracker.cpp
)

set(RESOURCE_FILES 
    NextGenRender.rc
    NextGenRender.ico
	small.ico
)

add_executable( ${TARGET_NAME} WIN32
    ${HEADER_FILES} 
    ${SOURCE_FILES}
	${RESOURCE_FILES}
)

target_link_libraries( MultiRenderApp 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC d3dcompiler.lib
    PUBLIC DirectXTex
)

add_subdirectory(Shader)