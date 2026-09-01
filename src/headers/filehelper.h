#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <cstdint>
#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>

std::vector<uint32_t> openFileToUint32Vector(const std::filesystem::path& path);

void writeUint32VectorToFile(const std::filesystem::path& path, const std::vector<uint32_t>& data);

bool doesFileExist(const std::filesystem::path& path);
void createFile(const std::filesystem::path& path);
std::vector<std::filesystem::path> getFilesInDirectory(const std::filesystem::path& directory);
#endif 
