# SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

function(read_entry ITEM FIELD PREFIX VAR_OUT)
    set(${VAR_OUT} PARENT_SCOPE)
    string(JSON TYPE_VALUE ERROR_VARIABLE JSON_ERROR TYPE ${ITEM} ${FIELD})

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
            set(${VAR_OUT} ${PREFIX} ${VALUE} PARENT_SCOPE)
        endif()
    elseif (${TYPE_VALUE} MATCHES "^BOOLEAN$")
        string(JSON VALUE GET ${ITEM} ${FIELD})
        if (${VALUE} MATCHES "ON")
            set(${VAR_OUT} ${PREFIX} ON PARENT_SCOPE)
        else()
            set(${VAR_OUT} ${PREFIX} OFF PARENT_SCOPE)
        endif()

    elseif (${TYPE_VALUE} MATCHES "^ARRAY$")
        string(JSON ARRAY_LENGTH LENGTH ${ITEM} ${FIELD})
        if (${ARRAY_LENGTH} EQUAL 0)
            return()
        endif()
        MATH(EXPR ARRAY_LENGTH "${ARRAY_LENGTH}-1")

        set(output ${PREFIX})
        foreach (IDX RANGE ${ARRAY_LENGTH})
            string(JSON VALUE GET ${ITEM} ${FIELD} ${IDX})
            list(APPEND output ${VALUE})
        endforeach()
        set(${VAR_OUT} "${output}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unexpected JSON type ${TYPE_VALUE} at position ${IDX} ${FIELD}")
    endif()
endfunction()

macro(read_packages_section FILE_AS_STRING)
    # Iterate over it and declare all dependencies
    string(JSON number_of_objects LENGTH ${FILE_AS_STRING})
    MATH(EXPR number_of_objects "${number_of_objects}-1")
    foreach (IDX RANGE ${number_of_objects})
        string(JSON TYPE_VALUE TYPE ${FILE_AS_STRING} ${IDX})
        string(JSON ITEM GET ${FILE_AS_STRING} ${IDX})
        if (${TYPE_VALUE} MATCHES "^OBJECT$")
            #!TODO Many options missing
            read_entry(${ITEM} NAME "" DEP_NAME)
            if ("${DEP_NAME}" EQUAL "")
                message(FATAL_ERROR "CPM dependency entry must hav a \"NAME\" or \"name\" field")
            endif()
            read_entry(${ITEM} FORCE "FORCE" DEP_FORCE)
            read_entry(${ITEM} VERSION "VERSION" DEP_VERSION)
            read_entry(${ITEM} GIT_TAG "GIT_TAG" DEP_GIT_TAG)
            read_entry(${ITEM} DOWNLOAD_ONLY "DOWNLOAD_ONLY" DEP_DOWNLOAD_ONLY)
            read_entry(${ITEM} PATCHES "PATCHES" DEP_PATCHES)
            read_entry(${ITEM} GITHUB_REPOSITORY "GITHUB_REPOSITORY" DEP_GITHUB_REPOSITORY)
            read_entry(${ITEM} BITBUCKET_REPOSITORY "BITBUCKET_REPOSITORY" DEP_BITBUCKET_REPOSITORY)
            read_entry(${ITEM} GITLAB_REPOSITORY "GITLAB_REPOSITORY" DEP_GITLAB_REPOSITORY)
            read_entry(${ITEM} GIT_REPOSITORY "GIT_REPOSITORY" DEP_GIT_REPOSITORY)
            read_entry(${ITEM} SOURCE_DIR "SOURCE_DIR" DEP_SOURCE_DIR)
            read_entry(${ITEM} FIND_PACKAGE_ARGUMENTS "FIND_PACKAGE_ARGUMENTS" DEP_FIND_PACKAGE_ARGUMENTS)
            read_entry(${ITEM} NO_CACHE "NO_CACHE" DEP_NO_CACHE)
            read_entry(${ITEM} SYSTEM "SYSTEM" DEP_SYSTEM)
            read_entry(${ITEM} GIT_SHALLOW "GIT_SHALLOW" DEP_GIT_SHALLOW)
            read_entry(${ITEM} EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL" DEP_EXCLUDE_FROM_ALL)
            read_entry(${ITEM} SOURCE_SUBDIR "SOURCE_SUBDIR" DEP_SOURCE_SUBDIR)
            read_entry(${ITEM} CUSTOM_CACHE_KEY "CUSTOM_CACHE_KEY" DEP_CUSTOM_CACHE_KEY)
            read_entry(${ITEM} URL "URL" DEP_URL)
            read_entry(${ITEM} OPTIONS "OPTIONS" DEP_OPTIONS)
            read_entry(${ITEM} DOWNLOAD_COMMAND "DOWNLOAD_COMMAND" DEP_DOWNLOAD_COMMAND)
            if (NOT DEP_SYSTEM)
                set(DEP_SYSTEM "SYSTEM" "YES")
            endif()
            CPMDeclarePackage(${DEP_NAME}
                "NAME ${DEP_NAME}"
                "${DEP_FORCE}"
                "${DEP_VERSION}"
                "${DEP_GIT_TAG}"
                "${DEP_DOWNLOAD_ONLY}"
                "${DEP_PATCHES}"
                "${DEP_GITHUB_REPOSITORY}"
                "${DEP_BITBUCKET_REPOSITORY}"
                "${DEP_GITLAB_REPOSITORY}"
                "${DEP_GIT_REPOSITORY}"
                "${DEP_SOURCE_DIR}"
                "${DEP_FIND_PACKAGE_ARGUMENTS}"
                "${DEP_NO_CACHE}"
                "${DEP_SYSTEM}"
                "${DEP_GIT_SHALLOW}"
                "${DEP_EXCLUDE_FROM_ALL}"
                "${DEP_SOURCE_SUBDIR}"
                "${DEP_CUSTOM_CACHE_KEY}"
                "${DEP_URL}"
                "${DEP_OPTIONS}"
                "${DEP_DOWNLOAD_COMMAND}"
            )

            list(APPEND PackageNames ${DEP_NAME})
        else()
            message(FATAL_ERROR "Unexpected JSON type ${TYPE_VALUE} at position ${IDX}")
        endif()
    endforeach()
endmacro()

macro(parseDependencyFile file)
    message("Parsing ${file}")
    file(READ ${file} FILE_AS_STRING)

    # Read format_version
    string(JSON FORMAT_VERSION ERROR_VARIABLE JSON_ERROR GET ${FILE_AS_STRING} "format_version")
    if (NOT ${JSON_ERROR} MATCHES "^NOTFOUND$")
        message(FATAL_ERROR "field \"format_version\" is missing. Recommended to set to \"format_version\": \"0.0.1\"")
    endif()

    if ("${FORMAT_VERSION}" MATCHES "1")
        # Read package section
        string(JSON ITEM ERROR_VARIABLE JSON_ERROR GET ${FILE_AS_STRING} "packages")
        if (${JSON_ERROR} MATCHES "^NOTFOUND$")
            read_packages_section(${ITEM})
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
            endforeach()
        endif()
    else()
        message(FATAL_ERROR "unkonwn \"format_version\" ${FORMAT_VERSION}")
    endif()
endmacro()

macro(CPMLoadDependenciesFile file)
    set(PackageNames)
    parseDependencyFile(${file})
    message("CPM.dependencies:")
    foreach (Package ${PackageNames})
        message("- ${Package}")
    endforeach()

    foreach (Package ${PackageNames})
        CPMGetPackage(${Package})
    endforeach()
endmacro()
