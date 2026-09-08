cmake_minimum_required(VERSION 3.6)

project(nauty-download NONE)

include(ExternalProject)

set(EXTRA_CFLAGS "'-O3 -fPIC'")
set(NCPU 4)

ExternalProject_Add(nauty_traces
  URL               https://users.cecs.anu.edu.au/~bdm/nauty/nauty26r10.tar.gz
  URL_HASH          SHA256=065f8d08f2c3f301f47b472a26ec8e2d7d8750d671722248afe61b03209ded9a
  SOURCE_DIR        "${NAUTY_WORK_DIR}"
  BINARY_DIR        "${NAUTY_WORK_DIR}"
  CONFIGURE_COMMAND "${NAUTY_WORK_DIR}/configure" "CFLAGS=${EXTRA_CFLAGS}"
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
