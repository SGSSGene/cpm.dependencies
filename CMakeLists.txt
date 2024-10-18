# SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

function(read_entry ITEM FIELD PREFIX VAR_OUT WITH_QUOTES AS_LIST)
    set(RET_VALUE)
    string(JSON TYPE_VALUE ERROR_VARIABLE JSON_ERROR TYPE ${ITEM} ${FIELD})

    set(QUOTES "")
    if (WITH_QUOTES)
        set(QUOTES "\"")
    endif()

    # If not found, check if lower case can be found
    if (NOT ${JSON_ERROR} MATCHES "^NOTFOUND$")
        string(TOLOWER ${FIELD} FIELD)
        string(JSON TYPE_VALUE ERROR_VARIABLE JSON_ERROR TYPE ${ITEM} ${FIELD})
    endif()

    if (NOT ${JSON_ERROR} MATCHES "^NOTFOUND$")
        # nothing to do
    elseif (${TYPE_VALUE} MATCHES "^STRING$")
        string(JSON VALUE GET ${ITEM} ${FIELD})
        if (NOT ${VALUE} MATCHES "^$")
            set(RET_VALUE "${PREFIX}${QUOTES}${VALUE}${QUOTES}")
        endif()
    elseif (${TYPE_VALUE} MATCHES "^BOOLEAN$")
        string(JSON VALUE GET ${ITEM} ${FIELD})
        if (${VALUE} MATCHES "ON")
            set(RET_VALUE "${PREFIX}ON")
        else()
            set(RET_VALUE "${PREFIX}OFF")
        endif()

    elseif (${TYPE_VALUE} MATCHES "^ARRAY$")
        string(JSON ARRAY_LENGTH LENGTH ${ITEM} ${FIELD})
        if (${ARRAY_LENGTH} EQUAL 0)
            return()
        endif()
        MATH(EXPR ARRAY_LENGTH "${ARRAY_LENGTH}-1")

        set(RET_VALUE ${PREFIX})
        set(SEP " ")
        if (AS_LIST)
            set(SEP ";")
        endif()

        foreach (IDX RANGE ${ARRAY_LENGTH})
            string(JSON VALUE GET ${ITEM} ${FIELD} ${IDX})
            set(RET_VALUE "${RET_VALUE}${SEP}${QUOTES}${VALUE}${QUOTES}")
        endforeach()
    else()
        message(FATAL_ERROR "Unexpected JSON type ${TYPE_VALUE} at position ${IDX} ${FIELD}")
    endif()

    set(${VAR_OUT} "${RET_VALUE}" PARENT_SCOPE)
endfunction()

function(read_packages_section FILE_AS_STRING)
    # Iterate over it and declare all dependencies
    string(JSON number_of_objects LENGTH ${FILE_AS_STRING})
    MATH(EXPR number_of_objects "${number_of_objects}-1")
    foreach (IDX RANGE ${number_of_objects})
        string(JSON TYPE_VALUE TYPE ${FILE_AS_STRING} ${IDX})
        string(JSON ITEM GET ${FILE_AS_STRING} ${IDX})
        if (${TYPE_VALUE} MATCHES "^OBJECT$")
            #!TODO Many options missing
            read_entry(${ITEM} IF "" DEP_IF NO NO)
            if ("${DEP_IF}" MATCHES ".")
                set(TEST_IF "
                    if (${DEP_IF})
                        set(MUST_LOAD_THIS_PACKAGE ON)
                    else()
                        set(MUST_LOAD_THIS_PACKAGE OFF)
                    endif()"
                )
                cmake_language(EVAL CODE "${TEST_IF}")
                if (NOT ${MUST_LOAD_THIS_PACKAGE})
                    continue()
                endif()
            endif ()

            read_entry(${ITEM} NAME "" DEP_NAME NO NO)
            if ("${DEP_NAME}" EQUAL "")
                message(FATAL_ERROR "CPM dependency entry must hav a \"NAME\" or \"name\" field")
            endif()
            read_entry(${ITEM} FORCE "FORCE " DEP_FORCE NO NO)
            read_entry(${ITEM} VERSION "VERSION " DEP_VERSION YES NO)
            read_entry(${ITEM} VERSION "" DEP_RAW_VERSION NO NO)
            read_entry(${ITEM} GIT_TAG "GIT_TAG " DEP_GIT_TAG YES NO)
            read_entry(${ITEM} DOWNLOAD_ONLY "DOWNLOAD_ONLY " DEP_DOWNLOAD_ONLY NO NO)
            read_entry(${ITEM} PATCHES "PATCHES " DEP_PATCHES YES NO)
            read_entry(${ITEM} GITHUB_REPOSITORY "GITHUB_REPOSITORY " DEP_GITHUB_REPOSITORY NO NO)
            read_entry(${ITEM} BITBUCKET_REPOSITORY "BITBUCKET_REPOSITORY " DEP_BITBUCKET_REPOSITORY NO NO)
            read_entry(${ITEM} GITLAB_REPOSITORY "GITLAB_REPOSITORY " DEP_GITLAB_REPOSITORY NO NO)
            read_entry(${ITEM} GIT_REPOSITORY "GIT_REPOSITORY " DEP_GIT_REPOSITORY NO NO)
            read_entry(${ITEM} SOURCE_DIR "SOURCE_DIR " DEP_SOURCE_DIR YES NO)
            read_entry(${ITEM} FIND_PACKAGE_ARGUMENTS "FIND_PACKAGE_ARGUMENTS " DEP_FIND_PACKAGE_ARGUMENTS YES NO)
            read_entry(${ITEM} NO_CACHE "NO_CACHE " DEP_NO_CACHE NO NO)
            read_entry(${ITEM} SYSTEM "SYSTEM " DEP_SYSTEM NO NO)
            read_entry(${ITEM} GIT_SHALLOW "GIT_SHALLOW " DEP_GIT_SHALLOW NO NO)
            read_entry(${ITEM} EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL " DEP_EXCLUDE_FROM_ALL NO NO)
            read_entry(${ITEM} SOURCE_SUBDIR "SOURCE_SUBDIR " DEP_SOURCE_SUBDIR YES NO)
            read_entry(${ITEM} CUSTOM_CACHE_KEY "CUSTOM_CACHE_KEY " DEP_CUSTOM_CACHE_KEY YES NO)
            read_entry(${ITEM} URL "URL " DEP_URL YES NO)
            read_entry(${ITEM} OPTIONS "OPTIONS " DEP_OPTIONS YES NO)
            read_entry(${ITEM} DOWNLOAD_COMMAND "DOWNLOAD_COMMAND " DEP_DOWNLOAD_COMMAND YES NO)
            read_entry(${ITEM} CPMADDPACKAGE_COMMAND "" DEP_${DEP_NAME}_CPMADDPACKAGE_COMMAND NO NO)
            read_entry(${ITEM} CMAKE_COMMANDS "" DEP_${DEP_NAME}_CMAKE_COMMANDS NO YES)

            string(REPLACE "{VERSION}" "${DEP_RAW_VERSION}" DEP_GIT_TAG "${DEP_GIT_TAG}")

            if (NOT DEP_SYSTEM)
                set(DEP_SYSTEM "SYSTEM YES")
            endif()
            if (NOT DEP_EXCLUDE_FROM_ALL)
                set(DEP_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL YES")
            endif()

            set(${DEP_NAME}_declare_command
                "${DEP_NAME}
                NAME ${DEP_NAME}
                ${DEP_FORCE}
                ${DEP_VERSION}
                ${DEP_GIT_TAG}
                ${DEP_DOWNLOAD_ONLY}
                ${DEP_PATCHES}
                ${DEP_GITHUB_REPOSITORY}
                ${DEP_BITBUCKET_REPOSITORY}
                ${DEP_GITLAB_REPOSITORY}
                ${DEP_GIT_REPOSITORY}
                ${DEP_SOURCE_DIR}
                ${DEP_FIND_PACKAGE_ARGUMENTS}
                ${DEP_NO_CACHE}
                ${DEP_SYSTEM}
                ${DEP_GIT_SHALLOW}
                ${DEP_EXCLUDE_FROM_ALL}
                ${DEP_SOURCE_SUBDIR}
                ${DEP_CUSTOM_CACHE_KEY}
                ${DEP_URL}
                ${DEP_OPTIONS}
                ${DEP_DOWNLOAD_COMMAND}"
                PARENT_SCOPE
            )
            set(DEP_${DEP_NAME}_CMAKE_COMMANDS "${DEP_${DEP_NAME}_CMAKE_COMMANDS}" PARENT_SCOPE)
            list(APPEND PackageNames ${DEP_NAME})
        else()
            message(FATAL_ERROR "Unexpected JSON type ${TYPE_VALUE} at position ${IDX}")
        endif()
    endforeach()
    set(PackageNames "${PackageNames}" PARENT_SCOPE)
endfunction()

function(parseDependencyFile file)
    file(READ ${file} FILE_AS_STRING)

    # Read format_version
    string(JSON FORMAT_VERSION ERROR_VARIABLE JSON_ERROR GET ${FILE_AS_STRING} "format_version")
    if (NOT ${JSON_ERROR} MATCHES "^NOTFOUND$")
        message(FATAL_ERROR "JSON Parsing Error: ${JSON_ERROR}")
    endif()

    if ("${FORMAT_VERSION}" MATCHES "1")
        # Read package section
        string(JSON ITEM ERROR_VARIABLE JSON_ERROR GET ${FILE_AS_STRING} "packages")
        if (${JSON_ERROR} MATCHES "^NOTFOUND$")
            read_packages_section(${ITEM})
            foreach (DEP_NAME ${PackageNames})
                set(${DEP_NAME}_declare_command "${${DEP_NAME}_declare_command}" PARENT_SCOPE)
                set(DEP_${DEP_NAME}_CMAKE_COMMANDS "${DEP_${DEP_NAME}_CMAKE_COMMANDS}" PARENT_SCOPE)
            endforeach()
            set(PackageNames "${PackageNames}" PARENT_SCOPE)
        endif()

        # Read other dependency files
        string(JSON ITEM ERROR_VARIABLE JSON_ERROR GET ${FILE_AS_STRING} "dependency_files")
        if (${JSON_ERROR} MATCHES "^NOTFOUND$")
            string(JSON number_of_objects LENGTH ${ITEM})
            MATH(EXPR number_of_objects "${number_of_objects}-1")
            foreach (IDX RANGE ${number_of_objects})
                string(JSON FILE GET ${ITEM} ${IDX})

                cmake_path(IS_RELATIVE FILE is_relative)
                if (${is_relative})
                    set(path ${file})
                    cmake_path(HAS_PARENT_PATH path has_parent)

                    cmake_path(GET path PARENT_PATH parent_path)
                    set(FILE "${parent_path}/${FILE}")
                endif()
                parseDependencyFile(${FILE})
                foreach (DEP_NAME ${PackageNames})
                    set(${DEP_NAME}_declare_command "${${DEP_NAME}_declare_command}" PARENT_SCOPE)
                    set(DEP_${DEP_NAME}_CMAKE_COMMANDS "${DEP_${DEP_NAME}_CMAKE_COMMANDS}" PARENT_SCOPE)
                endforeach()
                set(PackageNames "${PackageNames}" PARENT_SCOPE)
            endforeach()
        endif()
    else()
        message(FATAL_ERROR "unknown \"format_version\" ${FORMAT_VERSION}")
    endif()
endfunction()

macro(CPMLoadDependenciesFile file)
    set(PackageNames)
    parseDependencyFile(${file})
    message("CPM.dependencies:")
    foreach (DEP_NAME ${PackageNames})
        message("- ${DEP_NAME}")
        string(REGEX REPLACE "[\n\r \t]+" " " decl "${${DEP_NAME}_declare_command}")
#        message(${decl})
        cmake_language(EVAL CODE CPMDeclarePackage(${decl}))
    endforeach()

    foreach (DEP_NAME ${PackageNames})
        if (NOT DEP_${DEP_NAME}_CPMADDPACKAGE_COMMAND)
            CPMGetPackage("${DEP_NAME}")
        else()
            cmake_language(CALL ${DEP_${DEP_NAME}_CPMADDPACKAGE_COMMAND} ${declaration})
        endif()
        if (${DEP_NAME}_ADDED)
            foreach (cmd ${DEP_${DEP_NAME}_CMAKE_COMMANDS})
                set(CPM_CURRENT_SOURCE_DIR "${${DEP_NAME}_SOURCE_DIR}")
                cmake_language(EVAL CODE "${cmd}")
            endforeach()
        endif()

    endforeach()
endmacro()
