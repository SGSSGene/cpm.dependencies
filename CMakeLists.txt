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
            read_entry(${ITEM} NAME "" NAME)
            if ("${NAME}" EQUAL "")
                message(FATAL_ERROR "CPM dependency entry must hav a \"NAME\" or \"name\" field")
            endif()
            read_entry(${ITEM} FORCE "FORCE" FORCE)
            read_entry(${ITEM} VERSION "VERSION" VERSION)
            read_entry(${ITEM} GIT_TAG "GIT_TAG" GIT_TAG)
            read_entry(${ITEM} DOWNLOAD_ONLY "DOWNLOAD_ONLY" DOWNLOAD_ONLY)
            read_entry(${ITEM} PATCHES "PATCHES" PATCHES)
            read_entry(${ITEM} GITHUB_REPOSITORY "GITHUB_REPOSITORY" GITHUB_REPOSITORY)
            read_entry(${ITEM} BITBUCKET_REPOSITORY "BITBUCKET_REPOSITORY" BITBUCKET_REPOSITORY)
            read_entry(${ITEM} GITLAB_REPOSITORY "GITLAB_REPOSITORY" GITLAB_REPOSITORY)
            read_entry(${ITEM} GIT_REPOSITORY "GIT_REPOSITORY" GIT_REPOSITORY)
            read_entry(${ITEM} SOURCE_DIR "SOURCE_DIR" SOURCE_DIR)
            read_entry(${ITEM} FIND_PACKAGE_ARGUMENTS "FIND_PACKAGE_ARGUMENTS" FIND_PACKAGE_ARGUMENTS)
            read_entry(${ITEM} NO_CACHE "NO_CACHE" NO_CACHE)
            read_entry(${ITEM} SYSTEM "SYSTEM" SYSTEM)
            read_entry(${ITEM} GIT_SHALLOW "GIT_SHALLOW" GIT_SHALLOW)
            read_entry(${ITEM} EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL" EXCLUDE_FROM_ALL)
            read_entry(${ITEM} SOURCE_SUBDIR "SOURCE_SUBDIR" SOURCE_SUBDIR)
            read_entry(${ITEM} CUSTOM_CACHE_KEY "CUSTOM_CACHE_KEY" CUSTOM_CACHE_KEY)
            read_entry(${ITEM} URL "URL" URL)
            read_entry(${ITEM} OPTIONS "OPTIONS" OPTIONS)
            read_entry(${ITEM} DOWNLOAD_COMMAND "DOWNLOAD_COMMAND" DOWNLOAD_COMMAND)
            CPMDeclarePackage(${NAME}
                "NAME ${NAME}"
                "${FORCE}"
                "${VERSION}"
                "${GIT_TAG}"
                "${DOWNLOAD_ONLY}"
                "${PATCHES}"
                "${GITHUB_REPOSITORY}"
                "${BITBUCKET_REPOSITORY}"
                "${GITLAB_REPOSITORY}"
                "${GIT_REPOSITORY}"
                "${SOURCE_DIR}"
                "${FIND_PACKAGE_ARGUMENTS}"
                "${NO_CACHE}"
                "${SYSTEM}"
                "${GIT_SHALLOW}"
                "${EXCLUDE_FROM_ALL}"
                "${SOURCE_SUBDIR}"
                "${CUSTOM_CACHE_KEY}"
                "${URL}"
                "${OPTIONS}"
                "${DOWNLOAD_COMMAND}"
            )
            list(APPEND PackageNames ${NAME})
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
    foreach (Package ${PackageNames})
        CPMGetPackage(${Package})
    endforeach()
endmacro()
