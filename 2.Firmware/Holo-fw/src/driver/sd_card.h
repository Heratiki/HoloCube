/*
 * SD Card Driver
 * 
 * This header defines the interface for SD card operations:
 * - File and directory management
 * - File reading and writing
 * - Binary file operations
 * - File system navigation
 */

#ifndef SD_CARD_H
#define SD_CARD_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

// File system limits
#define DIR_FILE_NUM 10              // Maximum number of files per directory
#define DIR_FILE_NAME_MAX_LEN 20     // Maximum length of file names
#define FILENAME_MAX_LEN 100         // Maximum length of full file path

extern int photo_file_num;
extern char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

/**
 * File Type Enumeration
 * Defines possible types of filesystem entries
 */
enum FILE_TYPE : unsigned char
{
    FILE_TYPE_UNKNOW = 0,    // Unknown file type
    FILE_TYPE_FILE,          // Regular file
    FILE_TYPE_FOLDER         // Directory
};

/**
 * File Information Structure
 * Used to build linked list of directory entries
 */
struct File_Info
{
    char *file_name;          // Name of file or directory
    FILE_TYPE file_type;      // Type (file or directory)
    File_Info *front_node;    // Previous node in linked list
    File_Info *next_node;     // Next node in linked list
};

/**
 * Free memory allocated for file info structure
 * @param info Pointer to file info structure to free
 */
void release_file_info(File_Info *info);

/**
 * Combine two path components
 * @param dst_path Destination buffer for combined path
 * @param pre_path First part of path
 * @param rear_path Second part of path
 */
void join_path(char *dst_path, const char *pre_path, const char *rear_path);

/**
 * SD Card Management Class
 * Provides interface for all SD card operations
 */
class SdCard
{
private:
    char buf[128];  // Internal buffer for file operations

public:
    /**
     * Initialize SD card hardware
     */
    void init();

    /**
     * List directory contents recursively
     * @param dirname Directory to list
     * @param levels Number of subdirectory levels to traverse
     */
    void listDir(const char *dirname, uint8_t levels);

    /**
     * List directory contents as linked list
     * @param dirname Directory to list
     * @return Pointer to first File_Info node
     */
    File_Info *listDir(const char *dirname);

    /**
     * Create new directory
     * @param path Path of directory to create
     */
    void createDir(const char *path);

    /**
     * Remove directory and contents
     * @param path Path of directory to remove
     */
    void removeDir(const char *path);

    /**
     * Read entire file contents
     * @param path Path to file
     */
    void readFile(const char *path);

    /**
     * Read specific line from file
     * @param path Path to file
     * @param num Line number to read (0-based)
     * @return Line contents as String
     */
    String readFileLine(const char *path, int num);

    /**
     * Write data to file (overwrites existing content)
     * @param path Path to file
     * @param message1 Data to write
     */
    void writeFile(const char *path, const char *message1);

    /**
     * Open file with specified mode
     * @param path Path to file
     * @param mode File mode (FILE_READ, FILE_WRITE, etc)
     * @return File object
     */
    File open(const String &path, const char *mode = FILE_READ);

    /**
     * Append data to existing file
     * @param path Path to file
     * @param message Data to append
     */
    void appendFile(const char *path, const char *message);

    /**
     * Rename or move file
     * @param path1 Source path
     * @param path2 Destination path
     */
    void renameFile(const char *path1, const char *path2);

    /**
     * Delete file (C-string path version)
     * @param path Path to file
     * @return true if successful
     */
    boolean deleteFile(const char *path);

    /**
     * Delete file (String path version)
     * @param path Path to file
     * @return true if successful
     */
    boolean deleteFile(const String &path);

    /**
     * Read binary file into buffer
     * @param path Path to file
     * @param buf Buffer to store file contents
     */
    void readBinFromSd(const char *path, uint8_t *buf);

    /**
     * Write binary data to file
     * @param path Path to file
     * @param buf Buffer containing data to write
     */
    void writeBinToSd(const char *path, uint8_t *buf);

    /**
     * Test file I/O operations
     * @param path Path to test file
     */
    void fileIO(const char *path);
};

#endif
