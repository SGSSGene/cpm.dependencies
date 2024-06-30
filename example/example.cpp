// SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


int main(int argc, char** argv) {
    cxxopts::Options options("Example", "Example program to demonstrate a cpm.dependencies");
    options.add_options()
        ("h,help", "Help page")
    ;
    auto result = options.parse(argc, argv);

    if (result["help"].as<bool>()) {
        fmt::print("Printing a useless help page\n");
    } else {
        json data;
        data["comment"] = "hello world";

        fmt::print("{}\n", data.dump());
    }
    return 0;
}
