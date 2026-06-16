cmake_minimum_required(VERSION 3.16)
project(SwarmSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- External: GLFW ----------------------------------------------------------
add_subdirectory(external/glfw)
find_package(OpenGL REQUIRED)

# ---- ImGui sources -----------------------------------------------------------
set(IMGUI_DIR      ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp
)

# ---- ImPlot sources ----------------------------------------------------------
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/external/implot)
set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    ${IMPLOT_DIR}/implot_demo.cpp
)

# ---- Project sources (explicit list avoids picking up broken stubs) ----------
set(PROJECT_SOURCES
    src/main.cpp

    src/core/Agent.cpp
    src/core/Simulation.cpp
    src/core/XlsxWriter.cpp
    src/core/ReportManager.cpp

    src/algorithms/ParticleSwarmOptimization.cpp
    src/algorithms/AntColonyOptimization.cpp
    src/algorithms/ArtificialBeeColony.cpp
    src/algorithms/FishSchoolSearch.cpp

    src/scenarios/PathPlanning.cpp
    src/scenarios/Reconnaissance.cpp
    src/scenarios/ObstacleAvoidance.cpp
    src/scenarios/TargetSearch.cpp

    src/gui/GUI.cpp
)

# ---- Executable --------------------------------------------------------------
add_executable(SwarmSim
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)

target_include_directories(SwarmSim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scenarios
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/external/implot
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
)

target_link_libraries(SwarmSim PRIVATE
    glfw
    OpenGL::GL
)

# ---- MSVC-specific flags -----------------------------------------------------
if (MSVC)
    target_compile_options(SwarmSim PRIVATE /W3 /MP)
    # Suppress a few noisy warnings from third-party headers
    target_compile_definitions(SwarmSim PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================
#  CPack – produce a self-contained Windows installer / zip
# ============================================================
set(CPACK_PACKAGE_NAME    "SwarmSimFramework")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR  "SwarmSim")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Swarm Intelligence Simulation Framework")

# Copy the exe plus any runtime DLLs into install/bin
install(TARGETS SwarmSim
        RUNTIME DESTINATION bin)

# Also bundle the Visual C++ redistributable (if present)
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

# Generator: NSIS (installer) or ZIP
set(CPACK_GENERATOR "ZIP;NSIS")
set(CPACK_NSIS_DISPLAY_NAME "Swarm Intelligence Framework")
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut \\\"$DESKTOP\\\\SwarmSim.lnk\\\" \\\"$INSTDIR\\\\bin\\\\SwarmSim.exe\\\"")

include(CPack)