// STORAGE.H
#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <windows.h>
#include <mutex>

// first 4 bytes for chunk number, 4 bytes for chunk content length, 1 byte for final flag, 1 extra
const int FIXED_CHUNK_HEADER_SIZE = 10;
const int FIXED_CHUNK_CONTENT_SIZE = 2048;
const int FIXED_CHUNK_SIZE = FIXED_CHUNK_CONTENT_SIZE + FIXED_CHUNK_HEADER_SIZE; // 2058

class Storage {
    static const int fixedChunkContentSize = FIXED_CHUNK_CONTENT_SIZE;
    static const int fixedChunkHeaderSize = FIXED_CHUNK_HEADER_SIZE;
    static const int fixedChunkSizeWithHeader = FIXED_CHUNK_SIZE;
    std::string pathToDownloadFolder;
    std::string lastError = "no last error";

    std::mutex myMutex;

    bool doesFileExist(const std::string &name);

    void sortAndUpdateFullyDownloadedFile(std::string filename);

public:

    // pathToDownloadFolder - string of path to download folder e.g "./download"
    Storage(std::string pathToDownloadFolder);

    /**
     * Save chunk to storage chunk chunk max size should be 2048+10
     * ptrToChunkData - Chunk to be saved to storage
     * size - Size in bytes of each element to be written.
     * count - Number of elements, each one with a size of size bytes.
     * filename - Name of file with extension
     */
    int saveChunk(void *ptrToChunkData, size_t size, size_t count, std::string filename);

    /**
     * Get chunk from storage
     * ptrToFillWithChunkData - Pointer to the array of elements to be written
     * filename - Name of file with extension
     * chunkNumber - chunkNumber to retreive
     * chunkByteSize - Pointer to a size_t to be written, which indicates the total size of the chunk retrieved
     */
    int getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber, size_t *chunkByteSize);

    /**
     * adds file to donwload folder to be able to get chunk
     * @param pathToFile - absolute path to file
     * @param fileName - filename to save as
     * @return - true if success, false if false
     */
    bool addFileToDownloadFolder(std::string pathToFile, std::string fileName);

    // gets the final chunk number / total number of chunks in file
    // only fully downloaded file
    int getFinalChunkNumber(std::string fileName);

    // paramter buf - array of integers of chunk numbers that you have
    // count - Number of elements that can be used in buffer
    // returns the total number of current chunks of file (the array size of buf)
    // if unsuccessful if -1
    int getArrOfChunkNumbers(int * buf, size_t count, std::string filename);

    std::string getLastError();
};

void serializeInt32(char *buf, int32_t val);

int32_t parseInt32(char *buf);

#endif