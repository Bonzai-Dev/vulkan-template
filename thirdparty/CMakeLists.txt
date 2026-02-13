# SDL
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# spdlog
add_subdirectory(spdlog)

# glm
add_subdirectory(glm)

# Vulkan
find_package(Vulkan REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL3::SDL3 spdlog Vulkan::Vulkan glm::glm)
