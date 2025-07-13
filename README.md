# CPM.dependencies

Move dependency declaration from CMakeLists.txt or a cpm-package.lock to a JSON file (cpm.dependencies).

## About

This allows to declare dependencies into a single or multiple JSON files.
Having dependencies in a JSON file allows for other tools to easily parse and inspect or update dependency information.

It has the same advantages as a `cpm-package.lock` file, but also integrates the `GetPackage()` call. Making the usage even easier.

## How to integrate

Must use [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

```cmake
include(cmake/CPM.cmake)
CPMAddPackage("gh:SGSSGene/cpm.dependencies@1.0.1")

# Loads and adds all packages from the JSON file
CPMLoadDependenciesFile("cpm.dependencies")
```

## JSON file layout

An example layout:
```
{
    "format_version": "1",
    "packages": [
        {
            "name": "nlohmann_json",
            "version": "3.11.3",
            "github_repository": "nlohmann/json",
            "options": [
                "json_buildtests off"
            ]
        },
        {
            "name": "fmt",
            "version": "10.2.1",
            "git_tag": "{VERSION}",
            "github_repository": "fmtlib/fmt"
        }
    ],
    "dependency_files": [
        "secondfile.dependencies"
    ]
}
```
`format_version`: Must be value "1", allows for later backwards compatibility.
`packages`: A list of objects. Each objects represents a `CPMAddPackage`/`CPMDeclarePackage` call.
`dependency_files`: A list of files that should be additionally loaded.

Each package objects has the following layout, all keys must be complete lower-case or upper-case.
Only the `name` field is mandatory.
```
    {
        "if": "", # some condition with cmake variables
        "name": "name, mandetory",
        "force": false,
        "version": "",
        "git_tag": "",
        "download_only": "",
        "patches": "",
        "github_repository": "",
        "bitbucket_repository": "",
        "gitlab_repository": "",
        "git_repository": "",
        "source_dir": "",
        "find_package_arguments": "",
        "no_cache": false,
        "system": true,
        "git_shallow": false,
        "exclude_from_all": true,
        "source_subdir": "",
        "custom_cache_key": "",
        "url": [],
        "options": [],
        "download_command": [],
        "cpmaddpackage_command": "cpmaddpackage",
        "cmake_commands": []
    }
```

## Helper Variable
Field `git_tag` supports the variable `{VERSION}` which will be replaced by the content of the `version` field.
This allows to write more sophisticated tags like:
```
    {
        "name": "pcre2",
        "version": "10.43",
        "git_tag": "pcre2-{VERSION}"
        ...
    }
```
