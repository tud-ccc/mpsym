file(READ "${RESOURCE}" DATA HEX)
string(REGEX REPLACE "(..)" "0x\\1," DATA "${DATA}")

file(WRITE "${OUTFILE}"
  "#include <stddef.h>\n"
  "char const ${SYMBOL}[] = {${DATA}0};\n"
  "size_t const ${SYMBOL}_len = sizeof(${SYMBOL}) - 1;\n"
)
