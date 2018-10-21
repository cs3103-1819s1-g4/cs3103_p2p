// STORAGE.H
#ifndef STORAGE_H
#define STORAGE_H
#include <iostream>
#include <fstream>

class Storage
{
    int fixedChunkContentSize;
    int fixedChunkSizeWithHeader;
    int fixedChunkHeaderSize;
    std::string pathToDownloadFolder;
    bool doesFileExist (const std::string& name);
    void serializeInt32(char * buf, int32_t val);
    int32_t parseInt32(char * buf);
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
    int getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber, size_t * chunkByteSize);
};

#endif