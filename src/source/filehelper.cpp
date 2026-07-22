#include "filehelper.h"

std::vector<uint32_t> openFileToUint32Vector(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file)
        std::cout << "couldnt open file " << path.string() << " \n";

    file.seekg(0, std::ios::end);
    std::size_t bytes  = file.tellg();
    file.seekg(0);

    if (bytes  % sizeof(uint32_t) != 0)
        std::cout << "File size isn't a multiple of uint32_t\n";

    std::vector<uint32_t> data(bytes  / sizeof(uint32_t));

    file.read(reinterpret_cast<char*>(data.data()), bytes );

    return data;
}

void writeUint32VectorToFile(const std::filesystem::path& path, const std::vector<uint32_t>& data) {
    std::ofstream file(path, std::ios::binary);

    if (!file) {
        std::cout << "Couldn't open file " << path.string() << '\n';
        return;
    }

    file.write(
        reinterpret_cast<const char*>(data.data()),
        data.size() * sizeof(uint32_t)
        );

    if (!file) {
        std::cout << "Failed writing to file " << path.string() << '\n';
    }
}