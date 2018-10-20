// storage.cpp
#include "storage.h"
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>

Storage::Storage(int fixedChunkContentSize, std::string pathToDownloadFolder)
{
    this->fixedChunkContentSize = fixedChunkContentSize;
    // first 4 bytes for chunk number, 4 bytes for chunk content length, 1 byte for final flag
    this->fixedChunkHeaderSize = 9;
    this->fixedChunkSizeWithHeader = fixedChunkContentSize + this->fixedChunkHeaderSize;
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
    if(((char*)ptrToChunkData)[8] == true){
        finalChunkFound = true;
        finalChunkNumber = chunkNumberToSave;
    }

    char readChunkContent[fixedChunkContentSize];
    char readChunkHeader[fixedChunkHeaderSize];
    std::ifstream is(pathToFileOfDownloading);
    int currChunkNumber;
    while (is.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is.read(readChunkHeader,fixedChunkHeaderSize);
        is.read(readChunkContent,parseInt32(readChunkHeader+4));
        currChunkNumber = parseInt32(readChunkHeader);

        if(currChunkNumber == chunkNumberToSave) {
            // if chunk to save is already downloaded
            return -1;
        }

        if(readChunkHeader[8] == true){
            // if final chunk
            finalChunkFound = true;
            finalChunkNumber = parseInt32(readChunkHeader);
        }

    }
    is.close();

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
        is2.read(readChunkHeader,fixedChunkHeaderSize);
        is2.read(readChunkContent, parseInt32(readChunkHeader+4));
        chunkFlags[parseInt32(readChunkHeader)] = true;
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
        //std::cout << "file completed\n";
        sortAndUpdateFullyDownloadedFile(filename);
    } else {
        //std::cout << "file not completed\n";
    }
 

    //std::string temp = pathToFile.substr(pathToFile.find_last_of(".") + 1);
    //std::cout<<pathToFileOfDownloading;
    //std::cout<<"\n";

    return 1;
}

void Storage::sortAndUpdateFullyDownloadedFile(std::string filename){
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";


    char readChunkContent[fixedChunkContentSize];
    char readChunkHeader[fixedChunkHeaderSize];
    std::ifstream is(pathToFileOfDownloading);
    int count = 1;
    int fileSize = 0;
    while (is.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is.read(readChunkHeader,fixedChunkHeaderSize);
        int chunkContentSize = parseInt32(readChunkHeader+4);
        is.read(readChunkContent,chunkContentSize);
        // int chunkNumber = parseInt32(readChunkHeader);
        fileSize += chunkContentSize;
        count++;

    }
            std::cout<<count;
    std::cout<<"\n";

    is.close();

    std::ofstream outfile;
    outfile.open (pathToFileCompleted);
    std::ifstream is2(pathToFileOfDownloading);
    while (is2.peek() != std::ifstream::traits_type::eof()){ // loop and search
        is2.read(readChunkHeader,fixedChunkHeaderSize);
        int chunkContentSize = parseInt32(readChunkHeader+4);
        is2.read(readChunkContent,chunkContentSize);
        int chunkNumber = parseInt32(readChunkHeader);
        outfile.seekp ((chunkNumber-1) * fixedChunkContentSize);
        outfile.write (readChunkContent,chunkContentSize);

    }
    //std::cout<<"\n";
    is2.close();
    outfile.close();

}

int Storage::getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber, size_t * chunkTotalByteSize)
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
        std::ifstream is(pathToFileCompleted);
        char readChunk[fixedChunkSizeWithHeader];
        char readChunkContent[fixedChunkContentSize];
        char readChunkHeader[fixedChunkHeaderSize];

        int count = 1;

        while (is.peek() != std::ifstream::traits_type::eof()){ // loop getting chunk
            is.read(readChunkContent,fixedChunkContentSize);
            // set chunk number
            serializeInt32(readChunkHeader, count);

            //set chunk content size
            int chunkContentSize = is.gcount();
            serializeInt32(readChunkHeader+4, chunkContentSize);

            // set final flag
            readChunkHeader[8] = false;
            if(count == chunkNumber){

                if(is.eof()){
                    readChunkHeader[8] = true;
                }

                *chunkTotalByteSize = chunkContentSize + fixedChunkHeaderSize;                
                memcpy(readChunk, readChunkHeader, fixedChunkHeaderSize);
                memcpy(readChunk+fixedChunkHeaderSize, readChunkContent, chunkContentSize);
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
    memcpy(buf, &val, 4);
}

int32_t Storage::parseInt32(char * buf)
{
    int32_t val;
    memcpy(&val, buf, 4);
    return val;
}

bool Storage::doesFileExist (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

int main()
{
    Storage *stor = new Storage(1024, "./test");
    char temp[2048];
    // char t2[] = "lol";
    //int i;
    size_t totalChunkSize;
    size_t * chunkSizeRecieved = &totalChunkSize;
    int c = 1;
    while(1){
        //usleep(10000);
        //std::cout<<"\n";
        int i = rand() % 1110 + 1;
        //std::cout<<i;
        int work = stor->getChunk(temp,"test.png",i, chunkSizeRecieved);
        if(work != -1){
            stor->saveChunk(temp, sizeof(char), totalChunkSize, "test.out");
        }
        c++;
    }
        //     std::cout<<"\nok\n";

        // stor->getChunk(temp,"test.t",2, chunkSizeRecieved);
        // stor->saveChunk(temp, sizeof(char), totalChunkSize, "testTO.t");
    

    return 0;
}