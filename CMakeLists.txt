CU_SET_PATH("CMAKE_Individual-XP_DIR" "${CMAKE_CURRENT_LIST_DIR}")

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/Individual-XP.cpp")

AC_ADD_SCRIPT_LOADER("Individual-XP"      "${CMAKE_CURRENT_LIST_DIR}/src/loader.h")

CU_ADD_HOOK(AFTER_WORLDSERVER_CMAKE "${CMAKE_CURRENT_LIST_DIR}/src/cmake/after_ws_install.cmake")

message("-- Individual-XP CONFIGURED --")
