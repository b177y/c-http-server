#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <string.h>

int getNextString(char fromstring[], char getstring[], int pointer, int max){
    /* Function to get the next word from a string */
    int gs_pointer = 0;
    char currChar = fromstring[pointer];
    while (currChar != 32 && currChar != EOF && pointer < max) { // go through string adding to new string until space or EOF reached.
        getstring[gs_pointer] = currChar;
        pointer++; gs_pointer++;
        currChar = fromstring[pointer];
    }
    return pointer;
}

int getPath(char path[], char FileToGet[]){
    /* Function to get the full path from the filename by appending it to the current working directory */
    char cwd[128] = {0};
    if (getcwd(cwd, 128) == NULL){ // Assume that path is max 128 bytes (chars) long
        perror("Failed to get CWD (path may be too long).\n");
        return -1;
    }
    strcpy(path, cwd); // copy cwd to path
    strcat(path, FileToGet); // concatenate filename to path
    return 0;
}

void getFileToSend(char headers[], char path[]){
    char req[5] = {0};
    int pointer = 0;
    pointer = getNextString(headers, req, pointer, 5); // Get request type (this is so in theory further functionality could be added in the future, but this program only responds to get requests)
    printf("Request type: %s \n", req);
    char FileToGet[128] = {0}; // Assumes that filename is max 128 bytes (chars) long. If this is not the case you need to sort your file naming habits out...
    pointer = getNextString(headers, FileToGet, pointer+1, 128); // Get filename
    printf("Requested file: %s \n", FileToGet);
    if (strcmp(FileToGet, "/") == 0){
        strcpy(FileToGet, "/index.html"); // If no file is specified (request is '/') then serve index.html
    }
    getPath(path, FileToGet); // get full path of file.
    printf("Assumed Full Path: %s\n", path);
}
