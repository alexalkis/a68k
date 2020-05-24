cmake_minimum_required(VERSION 3.13)
project(a68k C)




if (CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")
    add_compile_options(-mcrt=nix13 -fbaserel -fomit-frame-pointer -msmall-code -Wall)
    set(CMAKE_EXE_LINKER_FLAGS "-s -mcrt=nix13 -fbaserel -msmall-code" CACHE INTERNAL "" FORCE)
    FIND_PROGRAM(CRANKER cranker_generic PATHS "/usr/local/in")
    IF(CRANKER)
        SET(CRANKER_COMMAND ${CRANKER} -f ${CMAKE_CURRENT_BINARY_DIR}/a68k -o ${CMAKE_CURRENT_BINARY_DIR}/a68kc)
    ENDIF(CRANKER)
else()
    #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    add_compile_options(
            "-Wall" "-Wpedantic" "-Wextra"
#            "$<$<CONFIG:DEBUG>:-fsanitize=address;-Wshadow;>"
    )
    add_link_options(
#            "-s"
#            "$<$<CONFIG:DEBUG>:-fsanitize=address>"
    )
endif(CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(a68k
        A68kdef.h
        A68kglb.h
        A68kmain.c
        A68kmisc.c
        Adirect.c
        Codegen.c
        embedlvos.c
        Opcodes.c
        Operands.c
        protos.h
        Symtab.c
        wb_parse.c)

#set(CMAKE_VERBOSE_MAKEFILE ON)

add_custom_command(TARGET a68k
        POST_BUILD
        COMMAND ${CRANKER_COMMAND}
        COMMAND ls -l a68k
        )