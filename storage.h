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
    void Storage::sortAndUpdateFullyDownloadedFile(std::string filename);

  public:
    Storage(int fixedChunkSize, std::string pathToDownloadFolder);
    int saveChunk(void *ptrToChunkData, size_t size, size_t count, std::string filename);
    int getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber, size_t * chunkByteSize);
};

#endif