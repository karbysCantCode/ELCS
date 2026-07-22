#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <cstdint>
#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>

std::vector<uint32_t> openFileToUint32Vector(const std::filesystem::path& path);

void writeUint32VectorToFile(const std::filesystem::path& path, const std::vector<uint32_t>& data);

#endif // FILEHELPER_H
