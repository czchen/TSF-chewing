cmake_minimum_required(VERSION 2.8.8)

project(TSF-chewing)
add_subdirectory(libchewing)
include_directories(libchewing/include)

# http://www.utf8everywhere.org/
add_definitions(/D_UNICODE=1)

set(SRC ${PROJECT_SOURCE_DIR}/src)

add_library(ChewingService SHARED
        ${SRC}/CandidateList.h
        ${SRC}/CandidateWindow.h
        ${SRC}/DisplayAttributeInfo.h
        ${SRC}/EditSession.h
        ${SRC}/EnumDisplayAttributeInfo.h
        ${SRC}/Globals.h
        ${SRC}/Resource.h
        ${SRC}/TextService.h
        ${SRC}/Util.h
        ${SRC}/CandidateList.cpp
        ${SRC}/CandidateWindow.cpp
        ${SRC}/Compartment.cpp
        ${SRC}/Composition.cpp
        ${SRC}/DisplayAttribute.cpp
        ${SRC}/DisplayAttributeInfo.cpp
        ${SRC}/DisplayAttributeProvider.cpp
        ${SRC}/DllMain.cpp
        ${SRC}/EndComposition.cpp
        ${SRC}/EnumDisplayAttributeInfo.cpp
        ${SRC}/Globals.cpp
        ${SRC}/KeyEventSink.cpp
        ${SRC}/KeyHandler.cpp
        ${SRC}/LanguageBar.cpp
        ${SRC}/PreComp.cpp
        ${SRC}/Register.cpp
        ${SRC}/Server.cpp
        ${SRC}/StartComposition.cpp
        ${SRC}/TextEditSink.cpp
        ${SRC}/TextService.cpp
        ${SRC}/ThreadMgrEventSink.cpp
        ${SRC}/TextService.def
        ${SRC}/TextService.ico
        ${SRC}/TextService.rc
        ${SRC}/Util.cpp
)
# How to link to target in subproject?
target_link_libraries(ChewingService chewing_static)
