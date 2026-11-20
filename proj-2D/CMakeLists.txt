cmake_minimum_required(VERSION 3.16)
project(Interactive2DWorld VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# 1. Dipendenze Autonome:
#    Desktop (Mac, Windows, Linux): Bundled GLM, GLFW, GLAD, ImGui
#    Web (Emscripten / WASM): Bundled GLM, WebGL 2.0, Emscripten GLFW3, ImGui
# -----------------------------------------------------------------------------

# GLM (Bundled Header-Only in external/glm)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glm/glm/glm.hpp")
    add_library(glm_bundled INTERFACE)
    target_include_directories(glm_bundled INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/glm")
    set(GLM_TARGET glm_bundled)
    message(STATUS "Usando GLM incluso nel progetto (external/glm)")
else()
    find_package(glm REQUIRED)
    set(GLM_TARGET glm::glm)
endif()

if(EMSCRIPTEN)
    message(STATUS "Configurazione target WebAssembly / WebGL 2.0 (Emscripten)")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # ImGui per WebGL / Emscripten
    add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/backends
    )
    target_compile_options(imgui PUBLIC -s USE_GLFW=3)

    # File sorgenti del motore di gioco
    file(GLOB_RECURSE SRC_FILES "src/*.cpp")
    add_executable(Interactive2DWorld ${SRC_FILES})

    target_include_directories(Interactive2DWorld PRIVATE
        include
        external
        external/imgui
        external/imgui/backends
    )

    target_link_libraries(Interactive2DWorld PRIVATE
        imgui
        ${GLM_TARGET}
    )

    target_compile_options(Interactive2DWorld PRIVATE -s USE_GLFW=3)
    target_link_options(Interactive2DWorld PRIVATE
        -s USE_GLFW=3
        -s USE_WEBGL2=1
        -s FULL_ES3=1
        -s WASM=1
        -s ALLOW_MEMORY_GROWTH=1
        -s NO_EXIT_RUNTIME=1
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell_minimal.html
    )

else() # COMPILAZIONE DESKTOP (macOS, Windows, Linux)

    # OpenGL
    find_package(OpenGL REQUIRED)

    # GLFW (Bundled in external/glfw con librerie statiche Mac e Windows)
    if(APPLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/macos/libglfw3.a")
        add_library(glfw_bundled STATIC IMPORTED)
        set_target_properties(glfw_bundled PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/macos/libglfw3.a"
        )
        set(GLFW_TARGET glfw_bundled)
        message(STATUS "Usando GLFW per macOS incluso nel progetto (external/glfw/lib/macos/libglfw3.a)")
    elseif(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/windows/glfw3.lib")
        add_library(glfw_bundled STATIC IMPORTED)
        set_target_properties(glfw_bundled PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/windows/glfw3.lib"
        )
        set(GLFW_TARGET glfw_bundled)
        message(STATUS "Usando GLFW per Windows incluso nel progetto (external/glfw/lib/windows/glfw3.lib)")
    else()
        find_package(glfw3 QUIET)
        if(glfw3_FOUND)
            set(GLFW_TARGET glfw)
        else()
            find_path(GLFW_INC GLFW/glfw3.h PATHS /opt/homebrew/include /usr/local/include /usr/include)
            find_library(GLFW_LIB NAMES glfw glfw3 PATHS /opt/homebrew/lib /usr/local/lib /usr/lib)
            add_library(glfw_sys UNKNOWN IMPORTED)
            set_target_properties(glfw_sys PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INC}"
                IMPORTED_LOCATION "${GLFW_LIB}"
            )
            set(GLFW_TARGET glfw_sys)
        endif()
    endif()

    # GLAD (Bundled in external/glad)
    add_library(glad STATIC
        external/glad/src/glad.c
    )
    target_include_directories(glad PUBLIC
        external/glad/include
    )

    # ImGui (Bundled in external/imgui)
    add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        external/imgui
        external/imgui/backends
        external/glad/include
    )
    target_link_libraries(imgui PUBLIC ${GLFW_TARGET} glad)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(imgui PRIVATE -Wno-nontrivial-memcall)
    endif()

    # Target Principale
    file(GLOB_RECURSE SRC_FILES
        "src/*.cpp"
    )

    add_executable(Interactive2DWorld ${SRC_FILES})

    target_include_directories(Interactive2DWorld PRIVATE
        include
        external
        external/glad/include
        external/imgui
        external/imgui/backends
    )

    target_link_libraries(Interactive2DWorld PRIVATE
        glad
        imgui
        ${GLFW_TARGET}
        ${GLM_TARGET}
        ${OPENGL_LIBRARIES}
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(Interactive2DWorld PRIVATE -Wno-nontrivial-memcall)
    endif()

    if(APPLE)
        target_link_libraries(Interactive2DWorld PRIVATE
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework QuartzCore"
        )
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(Interactive2DWorld PRIVATE
            ${CMAKE_DL_LIBS}
            pthread
            m
        )
    endif()

    # Copia automatica di assets/ accanto all'eseguibile dopo il build
    add_custom_command(TARGET Interactive2DWorld POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Interactive2DWorld>/assets
    )

endif()
