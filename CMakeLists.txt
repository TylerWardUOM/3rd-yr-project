cmake_minimum_required(VERSION 3.20)
project(Rewrite LANGUAGES C CXX)

# --------------------------------------------------
# C++ standard
# --------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------
# Main executable
# --------------------------------------------------
add_executable(app
    # third-party sources
    third_party/glad/src/gl.c
    
    src/main.cpp

    # core

    # data

    #platform
    src/platform/Window.cpp

    # geometry
    src/geometry/GeometryDatabase.cpp
    src/geometry/GeometryFactory.cpp
    src/geometry/sdf/PlaneSDF.cpp

    # world
    src/world/WorldManager.cpp

    # Render
    src/render/RenderMeshRegistry.cpp
    src/render/gpu/MeshGPU.cpp
    src/render/GlSceneRenderer.cpp
    src/render/shader/Shader.cpp
    src/render/UI/ViewportController.cpp
    src/render/UI/ImGuiLayer.cpp
    src/render/UI/Ui.cpp

    #Hardware
    src/hardware/SerialLink.cpp
    src/hardware/DeviceAdapter.cpp
    #haptic
    src/engines/HapticEngine.cpp

    #Physics
    src/engines/PhysicsEnginePhysX.cpp
)

# --------------------------------------------------
# Serial test executable
# --------------------------------------------------
add_executable(serial_tests
    src/main_serial_tests.cpp
    src/hardware/SerialLink.cpp
)

target_include_directories(serial_tests PRIVATE
    include
)

if (WIN32)
    target_link_libraries(serial_tests PRIVATE
        setupapi
    )
endif()

# --------------------------------------------------
# Copy shaders next to the executable
# --------------------------------------------------

set(SHADER_SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/include/render/shader
)

file(GLOB SHADER_FILES
    ${SHADER_SOURCE_DIR}/*.vert
    ${SHADER_SOURCE_DIR}/*.frag
)

add_custom_command(
    TARGET app
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:app>/shaders

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SHADER_FILES}
            $<TARGET_FILE_DIR:app>/shaders

    COMMENT "Copying shaders to output directory"
)


# --------------------------------------------------
# Project include paths
# --------------------------------------------------
target_include_directories(app PRIVATE
    include
    third_party/glad/include
)


set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

target_sources(app PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(app PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)


# --------------------------------------------------
# GLFW (vendored)
# --------------------------------------------------
add_subdirectory(third_party/glfw)

target_link_libraries(app PRIVATE glfw)

target_include_directories(app PRIVATE
    third_party/glfw/include
)

# --------------------------------------------------
# PhysX (vcpkg, manual linkage – corrected)
# --------------------------------------------------

set(VCPKG_INSTALLED_DIR "C:/Users/tman0/vcpkg/installed/x64-windows")

# Headers (top-level include!)
target_include_directories(app PRIVATE
    ${VCPKG_INSTALLED_DIR}/include/physx
)

# Libraries (handle Debug vs Release properly)
target_link_libraries(app PRIVATE
    $<$<CONFIG:Debug>:${VCPKG_INSTALLED_DIR}/debug/lib/PhysX_64.lib>
    $<$<CONFIG:Debug>:${VCPKG_INSTALLED_DIR}/debug/lib/PhysXCommon_64.lib>
    $<$<CONFIG:Debug>:${VCPKG_INSTALLED_DIR}/debug/lib/PhysXFoundation_64.lib>
    $<$<CONFIG:Debug>:${VCPKG_INSTALLED_DIR}/debug/lib/PhysXExtensions_static_64.lib>

    $<$<CONFIG:Release>:${VCPKG_INSTALLED_DIR}/lib/PhysX_64.lib>
    $<$<CONFIG:Release>:${VCPKG_INSTALLED_DIR}/lib/PhysXCommon_64.lib>
    $<$<CONFIG:Release>:${VCPKG_INSTALLED_DIR}/lib/PhysXFoundation_64.lib>
    $<$<CONFIG:Release>:${VCPKG_INSTALLED_DIR}/lib/PhysXExtensions_static_64.lib>
)

# --------------------------------------------------
# System libraries (Windows)
# --------------------------------------------------
if (WIN32)
    target_link_libraries(app PRIVATE
        opengl32
    )
endif()

# --------------------------------------------------
# Copy PhysX DLLs next to executable (runtime fix)
# --------------------------------------------------
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${VCPKG_INSTALLED_DIR}/bin/PhysX_64.dll
        $<TARGET_FILE_DIR:app>
)

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${VCPKG_INSTALLED_DIR}/bin/PhysXCommon_64.dll
        $<TARGET_FILE_DIR:app>
)

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${VCPKG_INSTALLED_DIR}/bin/PhysXFoundation_64.dll
        $<TARGET_FILE_DIR:app>
)

# --------------------------------------------------


# --------------------------------------------------

# --------------------------------------------------
# GLM (vendored, header-only)
# --------------------------------------------------

target_include_directories(app PRIVATE
    third_party/glm
)
# --------------------------------------------------