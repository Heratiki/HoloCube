/*
 * Flash Filesystem Driver
 * 
 * This header defines the interface for flash memory filesystem operations:
 * - Basic file operations (read, write, append, delete)
 * - Directory listing
 * - File management utilities
 */

#include <Arduino.h>
#include "FS.h"

/**
 * Flash Filesystem Management Class
 * 
 * Provides an interface for managing files in flash memory,
 * supporting basic file operations and directory management.
 */
class FlashFS
{
private:
    fs::FS *m_fs;  // Filesystem instance pointer

public:
    FlashFS();     // Constructor
    ~FlashFS();    // Destructor

    /**
     * List directory contents
     * @param dirname Directory path to list
     * @param levels Number of subdirectory levels to traverse
     */
    void listDir(const char *dirname, uint8_t levels);

    /* Directory operations (currently disabled)
    void createDir(const char *path);
    void removeDir(const char *path);
    */

    /**
     * Read file contents
     * @param path Path to the file
     * @param info Buffer to store file contents
     * @return Number of bytes read
     */
    uint16_t readFile(const char *path, uint8_t *info);

    /**
     * Write data to a file (overwrites existing content)
     * @param path Path to the file
     * @param message Data to write
     */
    void writeFile(const char *path, const char *message);

    /**
     * Append data to a file
     * @param path Path to the file
     * @param message Data to append
     */
    void appendFile(const char *path, const char *message);

    /**
     * Rename or move a file
     * @param src Source file path
     * @param dst Destination file path
     */
    void renameFile(const char *src, const char *dst);

    /**
     * Delete a file
     * @param path Path to the file to delete
     */
    void deleteFile(const char *path);

    /* Recursive file operations (currently disabled)
    void writeFile2(const char *path, const char *message);  // Write to nested path
    void deleteFile2(const char *path);                      // Delete nested file
    */

private:
    /**
     * Test file I/O operations
     * @param path Path to test file
     */
    void testFileIO(const char *path);
};

/**
 * Parse and analyze command parameters
 * @param info Input string to parse
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return true if parsing successful, false otherwise
 */
bool analyseParam(char *info, int argc, char **argv);
