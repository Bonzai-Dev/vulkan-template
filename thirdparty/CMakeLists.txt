# SDL
add_subdirectory(SDL EXCLUDE_FROM_ALL)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC SDL3::SDL3)

# spdlog
add_subdirectory(spdlog)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC spdlog)

# glm
add_subdirectory(glm)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC glm::glm)

# Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan Vulkan::Headers)