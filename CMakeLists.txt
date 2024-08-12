cmake_minimum_required(VERSION 3.5)

project(VRBaseStation LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Also link to VTK
find_package( VTK REQUIRED )
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(VRBaseStation
        main.cpp
        mainwindow.cpp
        mainwindow.h
	ModelPart.cpp
	ModelPart.h
	ModelPartList.cpp
	ModelPartList.h
	optionDialog.cpp
	optionDialog.h
        VRRenderThread.cpp
        VRRenderThread.h
        mainwindow.ui
	optiondialog.ui
	icons.qrc
)

target_link_libraries(VRBaseStation PRIVATE Qt6::Widgets )
target_link_libraries(VRBaseStation PRIVATE ${VTK_LIBRARIES})


set(OpenVR_INCLUDE_DIR "C:/OpenVR/headers")
target_include_directories(VRBaseStation PRIVATE C:/OpenVR/headers)
set(OpenVR_LIBRARY "C:/OpenVR/lib/win64/openvr_api.lib")
#target_include_directories(VRBaseStation PRIVATE ${C:/OpenVR/lib/win64/openvr_api.lib})


# Copy across Open VR bindings that map controllers
# The program will expect to find these in the build dir when it runs
add_custom_target(	VRBindings )
add_custom_command(	TARGET VRBindings PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E
		copy_directory ${CMAKE_SOURCE_DIR}/vrbindings ${CMAKE_BINARY_DIR}/ )


# Specify that VRBaseStation requires VRBindings to be built
add_dependencies( VRBaseStation VRBindings )


include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "VRBaseStation ${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_GENERATOR "NSIS")
set(CPACK_SOURCE_DIRECTORY "C:/Users/Izzam/EEEE2076-master")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSS "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_NAME "VRBaseStation")
set(CPACK_NSIS_PACKAGE_NAME "VRBaseStationInstaller")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VR Base Station Application Installer")
set(CPAC_PACKAGE_GROUP "University of Nottingham")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_NSIS_MODIFY_PATH ON)
#set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")


include(CPack)

install (TARGETS VRBaseStation DESTINATION bin)

set(CPACK_PACKAGE_EXECUTABLES "VRBaseStation" "renderer")
install (TARGETS VRBaseStation RUNTIME DESTINATION bin COMPONENT DEV)
install(DIRECTORY "C:/Program Files (x86)/VTK/bin" DESTINATION bin COMPONENT VTK FILES_MATCHING PATTERN "*.dll") 
install(DIRECTORY "C:/Qt/6.6.2/msvc2019_64/bin" DESTINATION bin COMPONENT Qt FILES_MATCHING PATTERN "*.dll") 
install(DIRECTORY "C:/OpenVR/bin/win64" DESTINATION bin COMPONENT OpenVR FILES_MATCHING PATTERN "*.dll") 

#install(DIRECTORY "C:/Users/Izzam/EEEE2076-master/Icons" DESTINATION bin COMPONENT Icons FILES_MATCHING PATTERN "*.png") 
#install(DIRECTORY "C:/Users/Izzam/EEEE2076-master/vrbindings" DESTINATION bin COMPONENT VRBINDINGS FILES_MATCHING_PATTERN "*.json") 