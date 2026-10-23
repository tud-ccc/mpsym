cmake_minimum_required(VERSION 3.18)

project(nlohmann_json-download NONE)

include(ExternalProject)

ExternalProject_Add(nlohmann_json
  URL              "https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz"
  URL_HASH         "SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa"
  SOURCE_DIR        "${NLOHMANN_JSON_SRC_DIR}"
  BINARY_DIR        "${NLOHMANN_JSON_BIN_DIR}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
