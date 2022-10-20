cmake_minimum_required(VERSION 3.16.1)

set(TARGET_NAME MultiRenderApp)

set(HEADER_FILES
	ConfigParameter.h 
	Helper.h
	framework.h
	Adapter.h
	Device.h
	SwapChain.h
	NextGenRender.h
	targetver.h
)

set(SOURCE_FILES 
	Adapter.cpp
	Device.cpp
	SwapChain.cpp
	NextGenRender.cpp
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
)