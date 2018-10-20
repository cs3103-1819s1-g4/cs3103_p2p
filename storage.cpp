// storage.cpp
#include "storage.h"
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>

Storage::Storage(int fixedChunkSize, std::string pathToDownloadFolder)
{
    this->fixedChunkSize = fixedChunkSize;
    // first 4 bytes for chunk number, 1 byte for final flag
    this->fixedChunkSizeWithHeader = fixedChunkSize + 5;
    this->pathToDownloadFolder = pathToDownloadFolder;

    // create download directory
    if(!doesFileExist(pathToDownloadFolder)){
        // TODO CHANGE TO WINDOWS
        if (mkdir(pathToDownloadFolder.c_str(), 0777) == -1) {
            std::cerr << "Error :  " << strerror(errno) << std::endl; 
        } else {
            std::cout << "Directory created"; 
        }
    }
}


int Storage::saveChunk(void *ptrToChunkData, size_t size, size_t count, std::string filename)
{
    int numOfCharInChunk = (count*size) / sizeof(char);
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";
    // chunks being saved should be incomplete files

    // file is complete
    if(doesFileExist(pathToFileCompleted)) {
        // should not be saving a chunk to a completed file
        return -1;
    }

    //if file does not exist create new .p2pdownloading file
    if(!doesFileExist(pathToFileOfDownloading)){
        // TODO CHANGE TO WINDOWS
        std::ofstream outfile (pathToFileOfDownloading);
        outfile.close();
    }


    bool finalChunkFound = false;
    int finalChunkNumber = -1;

    int chunkNumberToSave = parseInt32((char*)ptrToChunkData);
    // if saving chunk is final chunk
    if(((char*)ptrToChunkData)[4] == true){
        finalChunkFound = true;
        finalChunkNumber = chunkNumberToSave;
    }

    char readChunk[fixedChunkSizeWithHeader];
    std::ifstream is(pathToFileOfDownloading);
    int currChunkNumber;
    while (is.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is.read(readChunk,fixedChunkSizeWithHeader);
        currChunkNumber = parseInt32(readChunk);

        if(currChunkNumber == chunkNumberToSave) {
            // if chunk to save is already downloaded
            return -1;
        }

        if(readChunk[4] == true){
            // if final chunk
            finalChunkFound = true;
            finalChunkNumber = parseInt32(readChunk);
        }

    }
    is.close();
    //     std::cout<<chunkNumberToSave;
    //     std::cout<<"-";
    //     std::cout<<parseInt32((char*)ptrToChunkData);
    //             std::cout<<"-";
    //     std::cout<<(((char*)ptrToChunkData)[4] == true);   
    //                  std::cout<<"\ntemptemp";

    //     std::cout<<"\n";
    //             std::cout<<"\ntemptemp---";
    // sleep(1);
    //                 std::cout<<numOfCharInChunk;

    // append chunk to end of file
    std::ofstream myfile;
    myfile.open (pathToFileOfDownloading, std::ios::app);
    myfile.write( (char*)ptrToChunkData, numOfCharInChunk );    
    myfile.close();

    if(!finalChunkFound) {
        return 1;
    }

    // check if all chunks are downloaded
    bool * chunkFlags = (bool*) malloc(finalChunkNumber + 1);
    int i;
    for(i=1; i<=finalChunkNumber; i++){
        chunkFlags[i] = false;
    }

    std::ifstream is2(pathToFileOfDownloading);
    while (is2.peek() != std::ifstream::traits_type::eof()){ 
        is2.read(readChunk,fixedChunkSizeWithHeader);
        chunkFlags[parseInt32(readChunk)] = true;
    }
    is2.close();

    bool fileCompleted = true;
    for(i=1; i<=finalChunkNumber; i++){
        if(chunkFlags[i] == false) {
            //std::cout<<i;
            fileCompleted = false;
            break;
        }
    }

    free (chunkFlags);

    if(fileCompleted) {
        std::cout << "file completed\n";
    } else {
        std::cout << "file not completed\n";
    }
 

    //std::string temp = pathToFile.substr(pathToFile.find_last_of(".") + 1);
    //std::cout<<pathToFileOfDownloading;
    //std::cout<<"\n";

    return 1;
}

int Storage::getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber, size_t * chunkByteSize)
{
    if(chunkNumber <= 0){
        // chunk number should be more than 0
        return -1;
    }
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";
    // file does not exist
    if(!doesFileExist(pathToFileCompleted) && !doesFileExist(pathToFileOfDownloading)) {
        // should not be saving a chunk to a completed file
        return -1;
    }


    if(doesFileExist(pathToFileCompleted)){
        // get from completed file
        std::ifstream is(pathToFileCompleted, std::ifstream::binary);
        char readChunk[fixedChunkSizeWithHeader];
        int count = 1;
        while (is.peek() != std::ifstream::traits_type::eof()){ // loop getting chunk
            is.read(readChunk + 5,fixedChunkSize);
            serializeInt32(readChunk, count);

            readChunk[4] = false;
            if(count == chunkNumber){
                if(is.eof()){
                    readChunk[4] = true;
                }
                *chunkByteSize = is.gcount() + 5;
                memcpy(ptrToFillWithChunkData, readChunk, fixedChunkSizeWithHeader);
                is.close();
                return 1;
            }
            count++;
        }
        is.close();
        return -1;

    } else if(doesFileExist(pathToFileOfDownloading)) {
        // get from incomplete file


    }

    return 1;
}

void Storage::serializeInt32(char * buf, int32_t val)
{
    uint32_t uval = val;
    buf[0] = uval;
    buf[1] = uval >> 8;
    buf[2] = uval >> 16;
    buf[3] = uval >> 24;
}

int32_t Storage::parseInt32(char * buf)
{
    // This prevents buf[i] from being promoted to a signed int.
    uint32_t u0 = buf[0], u1 = buf[1], u2 = buf[2], u3 = buf[3];
    uint32_t uval = u0 | (u1 << 8) | (u2 << 16) | (u3 << 24);
    return uval;
}

bool Storage::doesFileExist (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

int main()
{
    Storage *stor = new Storage(8, "./test");
    char temp[13];
    // char t2[] = "lol";
    int i;
    size_t temp1;
    size_t * mysize = &temp1;
    // while(1){
    //     sleep(1);
    //     std::cout<<"\n";
    //     int i = rand() % 6 + 1;
    //     std::cout<<i;
    //     int work = stor->getChunk(temp,"test.t",i, mysize);
    //     if(work != -1){
    //         stor->saveChunk(temp, sizeof(char), temp1, "testTO.t");
    //     }
    // }
        stor->getChunk(temp,"test.t",2, mysize);
        stor->saveChunk(temp, sizeof(char), temp1, "testTO.t");
        stor->getChunk(temp,"test.t",3, mysize);
        stor->saveChunk(temp, sizeof(char), temp1, "testTO.t");
        stor->getChunk(temp,"test.t",5, mysize);
        stor->saveChunk(temp, sizeof(char), temp1, "testTO.t");
        stor->getChunk(temp,"test.t",1, mysize);
        stor->saveChunk(temp, sizeof(char), temp1, "testTO.t");
    


    return 0;
}