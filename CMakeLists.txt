cmake_minimum_required(VERSION 3.14)
project(cpp-project CXX)

# Znalezienie SDL2
find_package(SDL2 REQUIRED)

find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Automatyczne odnalezienie plik�w �r�d�owych w katalogu src
file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Dodanie pliku wykonywalnego
add_executable(cpp-project ${SRC_FILES})

# Dodanie katalog�w z nag��wkami (w tym w�asnych, SDL2 i SDL2_mixer)
target_include_directories(cpp-project PUBLIC 
    "${CMAKE_SOURCE_DIR}/include"  # Twoje nag��wki
    ${SDL2_INCLUDE_DIRS}           # Nag��wki SDL2
    ${SDL2_MIXER_INCLUDE_DIRS}     # Nag��wki SDL2_mixer
    ${SDL2_TTF_INCLUDE_DIRS} # Nag��wki SDL2_tff
)

# Linkowanie bibliotek SDL2 i SDL2_mixer
target_link_libraries(cpp-project PUBLIC 
    SDL2::SDL2                     # G��wna biblioteka SDL2
    SDL2::SDL2main                 # Funkcja g��wna SDL2
    SDL2_mixer::SDL2_mixer               # Biblioteka SDL2_mixer
    SDL2_ttf::SDL2_ttf  
)


# Ustawienie standardu C++
set_target_properties(cpp-project PROPERTIES CXX_STANDARD 23)
