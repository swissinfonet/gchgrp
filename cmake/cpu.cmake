if (NOT CMAKE_SYSTEM_PROCESSOR)
    message(WARNING "CMAKE_SYSTEM_PROCESSOR not defined")
endif()


if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    add_definitions(/DRAPIDJSON_SSE2)
endif()


if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64)$")
    set(gchgrp_ARM     ON)
    set(gchgrp_ARMv8   ON)
    set(WITH_LIBCPUID OFF)

    add_definitions(/Dgchgrp_ARM)
    add_definitions(/Dgchgrp_ARMv8)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv7|armv7f|armv7s|armv7k|armv7-a|armv7l)$")
    set(gchgrp_ARM     ON)
    set(gchgrp_ARMv7   ON)
    set(WITH_LIBCPUID OFF)

    add_definitions(/Dgchgrp_ARM)
    add_definitions(/Dgchgrp_ARMv7)
endif()
