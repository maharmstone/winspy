cmake_minimum_required(VERSION 3.10)

project(winspy)

set(SRC_FILES
	src/BitmapButton.c
    src/CaptureWindow.c
    src/DisplayClassInfo.c
    src/DisplayGeneralInfo.c
    src/DisplayProcessInfo.c
    src/DisplayPropInfo.c
    src/DisplayScrollInfo.c
    src/DisplayStyleInfo.c
    src/DisplayWindowInfo.c
    src/EditSize.c
    src/EditWindow.c
    src/FindTool.c
    src/FindToolTrans.c
    src/FunkyList.c
    src/GetRemoteWindowInfo.c
    src/InjectThread.c
    src/Options.c
    src/RegHelper.c
    src/StaticCtrl.c
    src/StyleEdit.c
    src/TabCtrlUtils.c
    src/Utils.c
    src/WindowFromPointEx.c
    src/WinSpy.c
    src/WinSpyCommand.c
    src/WinSpyDlgs.c
    src/WinSpyTree.c
    src/WinSpyWindow.c
    src/LoadPNG.cpp
    src/resource/WinSpy.rc)

add_executable(winspy ${SRC_FILES})

target_include_directories(winspy PUBLIC src/resource)

target_link_libraries(winspy uxtheme version comctl32 windowscodecs)
