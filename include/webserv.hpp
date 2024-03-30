#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>

/**
 * @brief Reads the entire content of a file into a string.
 *
 * This function opens the file specified by `filePath` 
 * and reads its entire content into a single string. 
 * It's designed to handle text files. 
 * If the file cannot be opened, the function outputs an error message to `std::cerr` 
 * and returns an empty string.
 *
 * @param filePath A constant reference to a `std::string` that contains the path to the file to be read.
 * @return A `std::string` containing the contents of the file. 
 * Returns an empty string if the file cannot be opened or is otherwise unreadable.
 *
 * @note This function uses `std::ifstream` to open and read the file. 
 * It assumes the file's contents are text and can be represented as a string. 
 * Binary files or files with data not compatible with `std::string` may not be handled correctly.
 */
std::string readFile(const std::string& filePath);

#endif