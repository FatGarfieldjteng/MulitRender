cmake_minimum_required(VERSION 3.16.1)

set(TARGET_NAME MultiRenderApp)

set(HEADER_FILES 
	framework.h
	NextGenRender.h
	targetver.h
)

set(SOURCE_FILES 
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