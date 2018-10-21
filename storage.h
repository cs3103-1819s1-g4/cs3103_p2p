// STORAGE.H
#ifndef STORAGE_H
#define STORAGE_H
#include <iostream>
#include <fstream>

const int FIXED_CHUNK_SIZE =  2058;
const int FIXED_CHUNK_HEADER_SIZE = 10;
const int FIXED_CHUNK_CONTENT_SIZE = 2048;

class Storage
{
    static const int fixedChunkContentSize = FIXED_CHUNK_CONTENT_SIZE;
    static const int fixedChunkHeaderSize = FIXED_CHUNK_HEADER_SIZE;
    static const int fixedChunkSizeWithHeader = FIXED_CHUNK_SIZE;
    std::string pathToDownloadFolder;
    bool doesFileExist (const std::string& name);
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

class Chunk{

    // first 4 bytes for chunk number, 4 bytes for chunk content length, 1 byte for final flag, 1 extra
    char completeChunk[FIXED_CHUNK_SIZE];
    static const int chunkHeaderSize = FIXED_CHUNK_HEADER_SIZE;

    char * chunkHeader;
    char * chunkContent;

    std::string filename;

    int completeChunkSize;
    int chunkContentSize;
    int chunkNumber;
    bool finalFlag;

  public:

    /**
     * Create a chunk object from data stream, chunk max size should be 2048+10
     * ptrToChunkData - Chunk to be saved to storage
     * size - Size in bytes of each element to be written.
     * count - Number of elements, each one with a size of size bytes.
     * filename - Name of file with extension
     */
    Chunk(void *ptrToChunkData, size_t size, size_t count, std::string filename);

    Chunk(void *ptrToChunkContent, size_t contentSize, size_t contentCount, int chunkNumber, bool finalFlag, std::string filename);

    int getCompleteChunkSize();
    int getChunkNumber();
    int getChunkContentSize();
    bool isFinalChunk(); 

};

void serializeInt32(char * buf, int32_t val);
int32_t parseInt32(char * buf);

#endif