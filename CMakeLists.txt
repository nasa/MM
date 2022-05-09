project(CFS_MM C)

include_directories(fsw/src)
include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)

set(APP_SRC_FILES
  fsw/src/mm_mem32.c
  fsw/src/mm_app.c
  fsw/src/mm_dump.c
  fsw/src/mm_mem16.c
  fsw/src/mm_utils.c
  fsw/src/mm_mem8.c
  fsw/src/mm_load.c
)

# Create the app module
add_cfe_app(mm ${APP_SRC_FILES})

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if(ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif()
