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
	Node.h
	CubeMesh.h
    Camera.h
	Light.h
	GraphicsResource.h
    BufferResource.h
	VertexBuffer.h
	IndexBuffer.h
	Texture.h
	TextureResource.h
	Scene.h
	World.h
	SimpleScene.h
	Effect.h
	Frame.h
	FrameData.h
	SimpleEffect.h
	ShadowEffect.h
	UploadBuffer.h
	CPUDescriptorAllocator.h
	CPUDescriptorPage.h
	CPUDescriptorAllocation.h
	ViewManager.h
	RootSignature.h
	RootSignatureManager.h
	PipelineState.h
	PipelineStateManager.h
	CommandList.h
	CommandRecorder.h
	ThreadSafeQueue.h
	ResourceStateTracker.h
	Managers.h
	ShaderManager.h
	EffectManager.h
	RootSignatureManager.h
	PipelineStateManager.h
	TextureManager.h
	LightManager.h
	CameraManager.h
	HeapManager.h
	RenderPass.h
	RenderGraph.h
	ShadowPass.h
	BeautyPass.h
	RenderTask.h
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
	Node.cpp
	CubeMesh.cpp
	Camera.cpp
	Light.cpp
	GraphicsResource.cpp
	BufferResource.cpp
	VertexBuffer.cpp
	IndexBuffer.cpp
	Texture.cpp
	TextureResource.cpp
	Scene.cpp
	World.cpp
	SimpleScene.cpp
	Effect.cpp
	Frame.cpp
	FrameData.cpp
	SimpleEffect.cpp
	ShadowEffect.cpp
	UploadBuffer.cpp
	CPUDescriptorAllocator.cpp
	CPUDescriptorPage.cpp
	CPUDescriptorAllocation.cpp
	ViewManager.cpp
	RootSignature.cpp
	RootSignatureManager.cpp
	PipelineState.cpp
	PipelineStateManager.cpp
	CommandList.cpp
	CommandRecorder.cpp
	ResourceStateTracker.cpp
	Managers.cpp
	ShaderManager.cpp
	EffectManager.cpp
	RootSignatureManager.cpp
	PipelineStateManager.cpp
	TextureManager.cpp
	LightManager.cpp
	CameraManager.cpp
	HeapManager.cpp
	RenderPass.cpp
	ShadowPass.cpp
	BeautyPass.cpp
	RenderGraph.cpp
	RenderTask.cpp
	../ThirdParty/enkiTS/src/TaskScheduler.cpp
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

target_include_directories(${TARGET_NAME} PRIVATE ../ThirdParty/enkiTS/src)

target_link_libraries( MultiRenderApp
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC d3dcompiler.lib
    PUBLIC DirectXTex
)

target_compile_features( MultiRenderApp 
	PUBLIC cxx_std_17
)

add_subdirectory(Shader)