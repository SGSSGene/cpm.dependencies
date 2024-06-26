# SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project("hallo")


function(read_entry ITEM FIELD PREFIX VAR_OUT)
    string(JSON TYPE_VALUE TYPE ${ITEM} ${FIELD})
    if (${TYPE_VALUE} MATCHES "^STRING$")
        string(JSON VALUE GET ${ITEM} ${FIELD})
        if (NOT ${VALUE} MATCHES "^$")
            set(${VAR_OUT} "${PREFIX} ${VALUE}" PARENT_SCOPE)
        endif()
    elseif (${TYPE_VALUE} MATCHES "^BOOLEAN$")
        string(JSON VALUE GET ${ITEM} ${FIELD})
        if (${VALUE} MATCHES "ON")
            set(${VAR_OUT} "${PREFIX} ON" PARENT_SCOPE)
        else()
            set(${VAR_OUT} "${PREFIX} OFF" PARENT_SCOPE)
        endif()

    elseif (${TYPE_VALUE} MATCHES "^ARRAY$")
        string(JSON ARRAY_LENGTH LENGTH ${ITEM} ${FIELD})
        if (${ARRAY_LENGTH} EQUAL 0)
            return()
        endif()
        MATH(EXPR ARRAY_LENGTH "${ARRAY_LENGTH}-1")

        set(output "")
        foreach (IDX RANGE ${ARRAY_LENGTH})
            string(JSON VALUE GET ${ITEM} ${FIELD} ${IDX})
            set(output "${output} ${VALUE}")
        endforeach()
        set(${VAR_OUT} "${PREFIX} ${output}" PARENT_SCOPE)
    elseif(${TYPE_VALUE} MATCHES "^NULL$")
        # nothing to do
    else()
        message(FATAL_ERROR "Unexpected JSON type ${TYPE_VALUE} at position ${IDX} ${FIELD}")
    endif()
endfunction()

function(CPMAddDependencies file)
    file(READ ${file} FILE_AS_STRING)

    # Iterate over it and declare all dependencies
    string(JSON number_of_objects LENGTH ${FILE_AS_STRING})
    MATH(EXPR number_of_objects "${number_of_objects}-1")
    foreach (IDX RANGE ${number_of_objects})
        string(JSON TYPE_VALUE TYPE ${FILE_AS_STRING} ${IDX})
        string(JSON ITEM GET ${FILE_AS_STRING} ${IDX})
        if (${TYPE_VALUE} MATCHES "^OBJECT$")
            read_entry(${ITEM} NAME "NAME" NAME)
            read_entry(${ITEM} VERSION "VERSION" VERSION)
            read_entry(${ITEM} PATCHES "PATCHES" PATCHES)
            read_entry(${ITEM} OPTIONS "OPTIONS" OPTIONS)
            read_entry(${ITEM} DOWNLOAD_ONLY "DOWNLOAD_ONLY" DOWNLOAD_ONLY)
            read_entry(${ITEM} forward_flags "" FORWARD_FLAGS)
            CPMAddPackage(${NAME}
                ${VERSION}
                ${PATCHES}
                ${OPTIONS}
                ${DOWNLOAD_ONLY}
                ${FORWARD_FLAGS}
            )
        elseif(${TYPE_VALUE} MATCHES "^STRING$")
            string(JSON VALUE GET ${FILE_AS_STRING} ${IDX})
            CPMAddPackage("${VALUE}")
        else()
            message(FATAL_ERROR "Unexpected JSON type ${TYPE_VALUE} at position ${IDX}")
        endif()
    endforeach()
endfunction()

CPMAddDependencies(test.json)
