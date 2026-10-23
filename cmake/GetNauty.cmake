cmake_minimum_required(VERSION 3.7)

project(nauty-download NONE)

include(ExternalProject)

set(EXTRA_CFLAGS "'-O3 -fPIC'")
set(NCPU 4)

ExternalProject_Add(nauty_traces
  URL
    https://pallini.di.uniroma1.it/nauty2_9_3.tar.gz
    https://users.cecs.anu.edu.au/~bdm/nauty/nauty2_9_3.tar.gz
  URL_HASH          SHA256=9fc4edae04f88a0f5883985be3b39cf7f898fd6cc96e96b9ee25452743cc1b5b
  TLS_VERIFY        OFF
  TIMEOUT           120
  SOURCE_DIR        "${NAUTY_WORK_DIR}"
  BINARY_DIR        "${NAUTY_WORK_DIR}"
  CONFIGURE_COMMAND "${NAUTY_WORK_DIR}/configure" "CFLAGS=${EXTRA_CFLAGS}"
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
