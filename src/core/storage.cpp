// storage.cpp
#include "storage.h"

Storage::Storage(std::string pathToDownloadFolder) {
    this->pathToDownloadFolder = pathToDownloadFolder;
    // create download directory
    if (!doesFileExist(pathToDownloadFolder)) {
        if (CreateDirectory(this->pathToDownloadFolder.c_str(), NULL)) {
            // Directory created
        } else if (ERROR_ALREADY_EXISTS == GetLastError()) {
            // Directory already exists
        } else {
            // Failed for some other reason
            lastError = "Failed to create new directory";
        }
    }
}

int Storage::saveChunk(void *ptrToChunkData, size_t size, size_t count, std::string filename) {
    std::lock_guard<std::mutex> guard(myMutex);

    int numOfCharInChunk = (count * size) / sizeof(char);
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";
    // chunks being saved should be incomplete files

    // file is complete
    if (doesFileExist(pathToFileCompleted)) {
        // should not be saving a chunk to a completed file
        lastError = "File already fully downloaded";
        return -1;
    }

    //if file does not exist create new .p2pdownloading file
    if (!doesFileExist(pathToFileOfDownloading)) {
        std::ofstream outfile(pathToFileOfDownloading);
        outfile.close();
    }


    bool finalChunkFound = false;
    int finalChunkNumber = -1;

    int chunkNumberToSave = parseInt32((char *) ptrToChunkData);
    // if saving chunk is final chunk
    if ((bool) ((char *) ptrToChunkData)[8] == true) {
        finalChunkFound = true;
        finalChunkNumber = chunkNumberToSave;
    }

    char readChunkContent[fixedChunkContentSize];
    char readChunkHeader[fixedChunkHeaderSize];
    std::ifstream is(pathToFileOfDownloading, std::ios::binary);
    int currChunkNumber;
    while (is.peek() != std::ifstream::traits_type::eof()) { // loop and search
        is.read(readChunkHeader, fixedChunkHeaderSize);
        is.read(readChunkContent, parseInt32(readChunkHeader + 4));
        currChunkNumber = parseInt32(readChunkHeader);

        if (currChunkNumber == chunkNumberToSave) {
            // if chunk to save is already downloaded
            lastError = "Chunk number for file already saved";
            return -1;
        }

        if ((bool) readChunkHeader[8] == true) {
            // if final chunk
            finalChunkFound = true;
            finalChunkNumber = parseInt32(readChunkHeader);
        }

    }
    is.close();

    // append chunk to end of file
    std::ofstream myfile;
    myfile.open(pathToFileOfDownloading, std::ios::binary | std::ios::app);
    myfile.write((char *) ptrToChunkData, numOfCharInChunk);
    myfile.close();

    if (!finalChunkFound) {
        return 1;
    }

    // check if all chunks are downloaded
    bool *chunkFlags = (bool *) malloc(finalChunkNumber + 1);
    int i;
    for (i = 1; i <= finalChunkNumber; i++) {
        chunkFlags[i] = false;
    }

    std::ifstream is2(pathToFileOfDownloading, std::ios::binary);
    while (is2.peek() != std::ifstream::traits_type::eof()) {
        is2.read(readChunkHeader, fixedChunkHeaderSize);
        is2.read(readChunkContent, parseInt32(readChunkHeader + 4));
        chunkFlags[parseInt32(readChunkHeader)] = true;
    }
    is2.close();

    bool fileCompleted = true;
    for (i = 1; i <= finalChunkNumber; i++) {
        if (chunkFlags[i] == false) {
            //std::cout<<i;
            fileCompleted = false;
            break;
        }
    }

    free(chunkFlags);

    if (fileCompleted) {
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

void Storage::sortAndUpdateFullyDownloadedFile(std::string filename) {
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";

    char readChunkContent[fixedChunkContentSize];
    char readChunkHeader[fixedChunkHeaderSize];
    std::ifstream is(pathToFileOfDownloading, std::ios::binary);
    int fileSize = 0;
    while (is.peek() != std::ifstream::traits_type::eof()) { // loop and search
        is.read(readChunkHeader, fixedChunkHeaderSize);
        int chunkContentSize = parseInt32(readChunkHeader + 4);
        is.read(readChunkContent, chunkContentSize);
        // int chunkNumber = parseInt32(readChunkHeader);
        fileSize += chunkContentSize;
    }
    is.close();

    std::ofstream outfile;
    outfile.open(pathToFileCompleted, std::ios::binary);
    std::ifstream is2(pathToFileOfDownloading, std::ios::binary);
    while (is2.peek() != std::ifstream::traits_type::eof()) { // loop and search
        is2.read(readChunkHeader, fixedChunkHeaderSize);
        int chunkContentSize = parseInt32(readChunkHeader + 4);
        is2.read(readChunkContent, chunkContentSize);
        int chunkNumber = parseInt32(readChunkHeader);
        outfile.seekp((chunkNumber - 1) * fixedChunkContentSize);
        outfile.write(readChunkContent, chunkContentSize);
    }
    is2.close();
    outfile.close();

    DeleteFile(pathToFileOfDownloading.c_str());

}

int Storage::getChunk(void *ptrToFillWithChunkData, std::string filename, int chunkNumber,
                      size_t *chunkTotalByteSize) {
    std::lock_guard<std::mutex> guard(myMutex);

    if (chunkNumber <= 0) {
        // chunk number should be more than 0
        lastError = "Chunk number should be more than 0";
        return -1;
    }
    std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
    std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";
    // file does not exist
    if (!doesFileExist(pathToFileCompleted) && !doesFileExist(pathToFileOfDownloading)) {
        lastError = "Downloaded file nor downloading file does not exist";
        return -1;
    }

    if (doesFileExist(pathToFileCompleted)) {
        // get from completed file
        std::ifstream is(pathToFileCompleted, std::ios::binary);
        char readChunk[fixedChunkSizeWithHeader];
        char readChunkContent[fixedChunkContentSize];
        char readChunkHeader[fixedChunkHeaderSize];
        int count = 1;
        while (is.peek() != std::ifstream::traits_type::eof()) { // loop getting chunk
            is.read(readChunkContent, fixedChunkContentSize);
            // set chunk number
            serializeInt32(readChunkHeader, count);
            //set chunk content size
            int chunkContentSize = (int) is.gcount();
            serializeInt32(readChunkHeader + 4, chunkContentSize);
            // set final flag
            readChunkHeader[8] = false;
            if (count == chunkNumber) {
                is.peek();
                if (is.eof()) {
                    readChunkHeader[8] = true;
                }

                *chunkTotalByteSize = chunkContentSize + fixedChunkHeaderSize;
                memcpy(readChunk, readChunkHeader, fixedChunkHeaderSize);
                memcpy(readChunk + fixedChunkHeaderSize, readChunkContent, chunkContentSize);
                memcpy(ptrToFillWithChunkData, readChunk, fixedChunkSizeWithHeader);

                is.close();
                return 1;
            }
            count++;
        }
        is.close();
        lastError = "Chunk number cannot be found from completed file";
        return -1;

    } else if (doesFileExist(pathToFileOfDownloading)) {
        // get from incomplete file
        char readChunk[fixedChunkSizeWithHeader];
        char readChunkContent[fixedChunkContentSize];
        char readChunkHeader[fixedChunkHeaderSize];
        std::ifstream is(pathToFileOfDownloading, std::ios::binary);
        while (is.peek() != std::ifstream::traits_type::eof()) { // loop and search
            is.read(readChunkHeader, fixedChunkHeaderSize);
            int chunkContentSize = parseInt32(readChunkHeader + 4);
            is.read(readChunkContent, chunkContentSize);
            int savedChunkNumber = parseInt32(readChunkHeader);
            if (chunkNumber == savedChunkNumber) {
                *chunkTotalByteSize = chunkContentSize + fixedChunkHeaderSize;
                memcpy(readChunk, readChunkHeader, fixedChunkHeaderSize);
                memcpy(readChunk + fixedChunkHeaderSize, readChunkContent, chunkContentSize);
                memcpy(ptrToFillWithChunkData, readChunk, fixedChunkSizeWithHeader);
                is.close();
                return 1;
            }
        }
        is.close();
        lastError = "Chunk number cannot be found from downloading file";
        return -1;
    }

    lastError = "end of getChunk, this line should not be reached";
    return -1;
}

void serializeInt32(char *buf, int32_t val) {
    uint32_t uval = val;
    buf[0] = uval;
    buf[1] = uval >> 8;
    buf[2] = uval >> 16;
    buf[3] = uval >> 24;
}

int32_t parseInt32(char *buf) {
    uint32_t u0 = (unsigned char) buf[0], u1 = (unsigned char) buf[1], u2 = (unsigned char) buf[2], u3 = (unsigned char) buf[3];
    uint32_t uval = u0 | (u1 << 8) | (u2 << 16) | (u3 << 24);
    return uval;
}

bool Storage::doesFileExist(const std::string &name) {
    std::ifstream f(name.c_str());
    return f.good();
}

bool Storage::addFileToDownloadFolder(std::string pathToFile, std::string fileName) {
    std::lock_guard<std::mutex> guard(myMutex);

    std::string pathToFileInDownloadFolder = pathToDownloadFolder + "\\" + fileName;
    return CopyFile(pathToFile.c_str(), pathToFileInDownloadFolder.c_str(), FALSE);
}

int Storage::getFinalChunkNumber(std::string fileName) {
    std::lock_guard<std::mutex> guard(myMutex);

    std::string pathToFileCompleted = pathToDownloadFolder + "/" + fileName;
    if (!doesFileExist(pathToFileCompleted)) {
        lastError = "Fully Downloaded file does not exist";
        return -1;
    }
    std::ifstream is(pathToFileCompleted, std::ios::binary);
    char readChunkContent[fixedChunkContentSize];
    int count = 1;
    while (is.peek() != std::ifstream::traits_type::eof()) { // loop getting chunk
        is.read(readChunkContent, fixedChunkContentSize);
        is.peek();
        if (is.eof()) {
            is.close();
            return count;
        }
        count++;
    }
    is.close();

    return count;
}

 int Storage::getArrOfChunkNumbers(int * buf, size_t maxElements, std::string filename){
     std::lock_guard<std::mutex> guard(myMutex);

     std::string pathToFileCompleted = pathToDownloadFolder + "/" + filename;
     std::string pathToFileOfDownloading = pathToDownloadFolder + "/" + filename + ".p2pdownloading";

     if (!doesFileExist(pathToFileCompleted) && !doesFileExist(pathToFileOfDownloading)) {
         lastError = "Downloaded file nor downloading file does not exist";
         return -1;
     }

     char readChunkContent[fixedChunkContentSize];
     char readChunkHeader[fixedChunkHeaderSize];
     int count = 0;

     if (doesFileExist(pathToFileCompleted)) {

         std::ifstream is(pathToFileCompleted, std::ios::binary);
         while (is.peek() != std::ifstream::traits_type::eof()) { // loop and search
             if(count>maxElements-1){
                 lastError = "Not enough int array buffer allocated for getting arr of chunk numbers";
                 return -1;
             }
             is.read(readChunkContent, fixedChunkContentSize);
             buf[count] = count + 1;
             count ++;
         }
         is.close();

     } else if(doesFileExist(pathToFileOfDownloading)) {

         std::ifstream is(pathToFileOfDownloading, std::ios::binary);
         while (is.peek() != std::ifstream::traits_type::eof()) { // loop and search
             if(count>maxElements-1){
                 lastError = "Not enough int array buffer allocated for getting arr of chunk numbers";
                 return -1;
             }
             is.read(readChunkHeader, fixedChunkHeaderSize);
             int chunkContentSize = parseInt32(readChunkHeader + 4);
             is.read(readChunkContent, chunkContentSize);
             int chunkNumber = parseInt32(readChunkHeader);
             buf[count] = chunkNumber;
             count ++;
         }
         is.close();
     }

     return count;
 }

 std::string Storage::getLastError(){
    return lastError;
}


// This is an example of how to use the storage class
int main() {
    Storage *stor = new Storage("./downloads");

    //stor->addFileToDownloadFolder("C:\\Users\\Jonathan Weng\\CLionProjects\\cs3103_p2p\\cmake-build-debug\\downloads\\mygit.exe", "mygi");
    //std::cout<<stor->getFinalChunkNumber("2.txt");

    int finNo = stor->getFinalChunkNumber("test2.out");
    if(finNo>0){
        std::cout<< "fino: " << finNo << std::endl;
    } else {
        std::cout<<stor->getLastError() << std::endl;
    }
    int myarr[1000];
    int chunNo = stor->getArrOfChunkNumbers(myarr, 1000, "test2.out");
    if(chunNo>0){
        std::cout<< "chunno: "<< chunNo << std::endl;
    //        int j;
    //        for(j=0;j<chunNo;j++){
    //            std::cout << myarr[j] << std::endl;
    //        }
    } else {
        std::cout<< "lasterror: " << stor->getLastError() << std::endl;
    }



    char temp[2058]; // temp to be used to get and pass chunk data has to be maximum chunk size
    // the chunk content size can be from 1-2048 to be used when saving chunk

    size_t totalChunkSize; // to be used when getting chunk, the chunk size which includes chunk content and header size
    // will be wrtten into this variable so that you know the size of the chunk out of the 2058 bytes temp arr.
    size_t *chunkSizeRecieved = &totalChunkSize;

    int i;
    while (1) {
        i = 1 + rand() % 1000;
        int work = stor->getChunk(temp, "test2.out", i, chunkSizeRecieved);
        if (work != -1) {
            // total chunk size includes the size of the chunk header + size of chunk content
            stor->saveChunk(temp, sizeof(char), totalChunkSize, "test8.out");
        }
    }
    return 0;
}