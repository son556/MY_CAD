#pragma once

#include <fstream>
#include <string>

class ReadSource
{
public:
    static void Read(std::string& src, const std::string& srcPath)
    {
        std::ifstream file(srcPath);
        std::string srcCode((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        src = std::move(srcCode);
    }

private:
    ReadSource() = delete;
    ReadSource(const ReadSource&) = delete;
    ReadSource& operator=(const ReadSource&) = delete;
    ~ReadSource() = delete;
};