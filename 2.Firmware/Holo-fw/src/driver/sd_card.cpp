/*
 * SD Card Driver Implementation
 * 
 * This file implements the SD card functionality:
 * - Hardware initialization and configuration
 * - File and directory operations
 * - Binary file handling
 * - File system navigation and management
 */

#include "sd_card.h"
#include "SD_MMC.h"
#include <string.h>

// Global variables for file management
int photo_file_num = 0;
char file_name_list[DIR_FILE_NUM][DIR_FILE_NAME_MAX_LEN];

// Virtual filesystem pointer
static fs::FS *tf_vfs;

/**
 * Free memory allocated for file info linked list
 * Traverses list and frees all nodes
 */
void release_file_info(File_Info *info)
{
    File_Info *cur_node = NULL;
    if (NULL == info)
    {
        return;
    }
    for (cur_node = info->next_node; NULL != cur_node;)
    {
        // Check for circular reference
        if (info->next_node == cur_node)
        {
            break;
        }
        File_Info *tmp = cur_node;
        cur_node = cur_node->next_node;
        free(tmp);
    }
    free(info);
}

/**
 * Join two path components safely
 * Handles path separators appropriately
 */
void join_path(char *dst_path, const char *pre_path, const char *rear_path)
{
    // Copy first path component
    while (*pre_path != 0)
    {
        *dst_path = *pre_path;
        ++dst_path;
        ++pre_path;
    }
    // Add separator if needed
    if (*(pre_path - 1) != '/')
    {
        *dst_path = '/';
        ++dst_path;
    }

    // Skip leading separator in second component
    if (*rear_path == '/')
    {
        ++rear_path;
    }
    // Copy second path component
    while (*rear_path != 0)
    {
        *dst_path = *rear_path;
        ++dst_path;
        ++rear_path;
    }
    *dst_path = 0;
}

/**
 * Get basename from full path
 * Returns pointer to last component of path
 */
static const char *get_file_basename(const char *path)
{
    const char *ret = path;
    for (const char *cur = path; *cur != 0; ++cur)
    {
        if (*cur == '/')
        {
            ret = cur + 1;
        }
    }
    return ret;
}

/**
 * Initialize SD card hardware
 * Configures SPI interface and mounts filesystem
 */
void SdCard::init()
{
    // Initialize SPI interface for SD card
    SPIClass *sd_spi = new SPIClass(HSPI);
    sd_spi->begin(14, 26, 13, 15);        // Custom HSPI pins
    if (!SD.begin(15, *sd_spi, 80000000)) // SD-Card SS pin is 15
    {
        Serial.println("Card Mount Failed");
        return;
    }
    tf_vfs = &SD;
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    // Print card information
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

/**
 * List directory contents recursively
 * Stores file names in global file_name_list
 */
void SdCard::listDir(const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);
    photo_file_num = 0;

    File root = tf_vfs->open(dirname);
    if (!root || !root.isDirectory())
    {
        Serial.println("Failed to open directory");
        return;
    }

    int dir_len = strlen(dirname) + 1;
    File file = root.openNextFile();
    
    while (file && photo_file_num < DIR_FILE_NUM)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            // Store file name without directory path and extension
            strncpy(file_name_list[photo_file_num], file.name() + dir_len, DIR_FILE_NAME_MAX_LEN - 1);
            file_name_list[photo_file_num][strlen(file_name_list[photo_file_num]) - 4] = 0;

            char file_name[FILENAME_MAX_LEN] = {0};
            sprintf(file_name, "%s/%s.bin", dirname, file_name_list[photo_file_num]);
            Serial.print(file_name);
            ++photo_file_num;
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    Serial.println(photo_file_num);
}

/**
 * List directory contents as linked list
 * Creates circular doubly-linked list of File_Info nodes
 */
File_Info *SdCard::listDir(const char *dirname)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = tf_vfs->open(dirname);
    if (!root || !root.isDirectory())
    {
        Serial.println("Failed to open directory");
        return NULL;
    }

    int dir_len = strlen(dirname) + 1;

    // Create head node for directory
    File_Info *head_file = (File_Info *)malloc(sizeof(File_Info));
    head_file->file_type = FILE_TYPE_FOLDER;
    head_file->file_name = (char *)malloc(dir_len);
    strncpy(head_file->file_name, dirname, dir_len - 1);
    head_file->file_name[dir_len - 1] = 0;
    head_file->front_node = NULL;
    head_file->next_node = NULL;

    File_Info *file_node = head_file;
    File file = root.openNextFile();

    // Add node for each directory entry
    while (file)
    {
        const char *fn = get_file_basename(file.name());
        int filename_len = strlen(fn);
        if (filename_len > FILENAME_MAX_LEN - 10)
        {
            Serial.println("Filename is too long.");
        }

        // Create and link new node
        file_node->next_node = (File_Info *)malloc(sizeof(File_Info));
        file_node->next_node->front_node = file_node;
        file_node = file_node->next_node;

        // Set node data
        file_node->file_name = (char *)malloc(filename_len);
        strncpy(file_node->file_name, fn, filename_len);
        file_node->file_name[filename_len] = 0;
        file_node->next_node = NULL;

        char tmp_file_name[FILENAME_MAX_LEN] = {0};
        join_path(tmp_file_name, dirname, file_node->file_name);
        
        // Set node type and print info
        if (file.isDirectory())
        {
            file_node->file_type = FILE_TYPE_FOLDER;
            Serial.print("  DIR : ");
            Serial.println(tmp_file_name);
        }
        else
        {
            file_node->file_type = FILE_TYPE_FILE;
            Serial.print("  FILE: ");
            Serial.print(tmp_file_name);
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }

        file = root.openNextFile();
    }

    // Make list circular if it has entries
    if (NULL != head_file->next_node)
    {
        file_node->next_node = head_file->next_node;
        head_file->next_node->front_node = file_node;
    }
    return head_file;
}

// File and directory operation implementations...
// (Rest of the implementation remains the same, already well structured)
