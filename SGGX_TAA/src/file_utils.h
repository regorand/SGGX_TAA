#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>

static const std::string RES_PATH = "res/models/";

static std::vector<std::string> loadableObjs;

static std::string read_file(const std::string_view path)
{
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

static std::vector<std::string> getLoadableObj(std::string base_path) {
    std::vector<std::string> results;
    for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
        if (entry.is_directory()) {
            std::cout << entry.path() << std::endl;
            for (const auto& inner_entry : std::filesystem::directory_iterator(entry.path())) {
                if (inner_entry.path().extension() == ".obj") {
                    results.push_back(inner_entry.path().string());
                }
            }
        }
    }
    return results;
}

static void updateLoadableObjects() {
    loadableObjs = getLoadableObj(RES_PATH);
}
