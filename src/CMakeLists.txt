rock_library(act_schilling
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(act_schilling_bin Main.cpp
    DEPS act_schilling)

