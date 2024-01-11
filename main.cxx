#include <filesystem>
#include <format>
#include <fstream>

#include "stdio.h"

template <typename T>
void byteswap(T& value)
{
    T result;

    size_t n = sizeof(T);
    for (int i = 0; i < n; ++i)
    {
        ((uint8_t*)&result)[i] = ((uint8_t*)&value)[n - i - 1];
    }

    value = result;
}

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./main.exe <input path> <output path>");
        return 0;
    }

    std::ifstream istream(argv[1], std::ifstream::binary);

    int depth = 0;

    std::filesystem::path path(argv[2]);

    size_t container    = 0;
    std::streampos from = 0;

    while (!istream.eof())
    {
        char type[8];
        istream.get(type, sizeof(type), '\0');

        istream.seekg(1, std::ios_base::cur);

        char name[64];
        istream.get(name, sizeof(name), '\0');

        istream.seekg(1, std::ios_base::cur);

        uint32_t length;
        istream.read((char*)&length, sizeof(length));

        byteswap(length);

        std::string label = "";

        if (!path.empty())
        {
            try
            {
                std::filesystem::create_directories(path);
            }
            catch(const std::exception& e)
            {
                printf("%s\n", e.what());
            }
        }

        for (int i = 0; i < depth; ++i)
        {
            label += "|";
        }

        label += std::format("+ {} ({} bytes)", name, length);

        printf("%s\n", label.c_str());

        if (strcmp(type, "Leaf") == 0)
        {
            std::ofstream ostream(path.string() + "/" + name, std::fstream::binary);

            char* buf = (char*)malloc(length);
            istream.read(buf, length);
            ostream.write(buf, length);

            free(buf);
        }
        else if (strcmp(type, "Branch") == 0)
        {
            path.append(name);

            container = length;
            from      = istream.tellg();
            ++depth;
        }
        else if (strlen(type) == 0)
        {
            break;
        }

        if (depth > 0)
        {
            if (istream.tellg() - from >= container)
            {
                path = path.parent_path();

                --depth;
            }
        }
    }

    return 0;
}