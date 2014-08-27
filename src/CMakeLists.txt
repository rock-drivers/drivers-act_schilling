rock_library(act_schilling
    SOURCES Driver.cpp ActHandler.cpp
    HEADERS Driver.hpp ActHandler.hpp ActTypes.hpp ActRaw.hpp Config.hpp PanTiltTypes.hpp
    DEPS_PKGCONFIG base-types base_schilling)

rock_executable(act_schilling_bin Main.cpp
    DEPS act_schilling)

