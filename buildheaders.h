#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h> 
#include <unistd.h>

int cmpSuffixes(char path[], char output[]){
    /* Function to get known http Content-Types using file extensions.
    As files on the server are placed by the server admin, not uploaded by users, we assume file extensions are legitimate. */
    int plen = strlen(path);
    path = path + plen; // Path now points to end of path, so subtracted n from it will give the last n chars of it.
    if (strncmp(path-5, ".html", 5) == 0){
        strcpy(output, "text/html; charset=UTF-8"); // if path ends in .html, set html Content-Type
        return 1;
    } else if (strncmp(path-4, ".css", 4) == 0){
        strcpy(output, "text/css"); /* if path ends in .css, set css content type. Using file -i to get the mime type of a css file returns
        text/plain as there is no file signature specifically for css files. Not all browsers will process css when text/plain is given as content type. */
        return 1;
    }
    return 0;
}

void magicMime(char path[], char output[]){
    /* Function to use the linux file command to get the mime type of a file by looking at the magic bytes.
    This is useful as the server can serve files that it has not been set up to serve with the correct http headers.*/
    char command[248] = {0};
    sprintf(command, "file -i \"%s\"", path); // format the command string with the file path, ready to be executed
    FILE *p;
    p = popen(command, "r"); // Execute the command so that it can be read using file pointer p
    int cmd_ptr = 0;
    char currChar = fgetc(p);
    while (currChar != EOF && currChar != 10){ // Read the command using the file pointer until EOF or newline is found.
        output[cmd_ptr] = currChar;
        cmd_ptr++;
        currChar = fgetc(p);
    }
    output[cmd_ptr] = '\0';
    int pathlen = strlen(path) + 2; /* output is in format "path: http-content-type" where we only want the 
    http-content-type so we must take everything from two beyond the length of the path to the end of the string */
    int outlen = strlen(output);
    strncpy(output, output + pathlen, outlen - pathlen); // copy output to itself using new bounds
    output[outlen-pathlen] = '\0';
}

void getFileType(char path[], char output[]){
    /* Function to get the http Content-Type of a file. First it will attempt to compare file extensions,
    and if no match is found, it will use the linux file command to attempt to identify the file type*/
    if (cmpSuffixes(path, output) != 1){
        magicMime(path, output);
    }
}

void buildHeaders(char buffer[], char path[]){
    /* Function to build HTTP headers for a response */
    printf("Building HTTP headers.\n");
    int fd = open(path, O_RDONLY); /* If the file exists the file descriptor will be larger than 2
    (if its negative there was an error, cannot be 0, 1 or 2 as those are stdin, stdout and stderr) */
    if (fd > 2) { // If file exists
        strcat(buffer, "HTTP/1.1 200 OK\n"); // Set HTTP status code to OK as file has been found.
        char output[100] = {0};
        getFileType(path, output); // Work out filetype
        printf("\nContent type is : %s\n", output);
        strcat(buffer, "Content-Type: "); strcat(buffer, output); strcat(buffer, "\n"); // Set HTTP Content-Type header
    }
    else { // If file does not exist
        strcat(buffer, "HTTP/1.1 404 Not Found\n"); // Set HTTP status code to 404 to show that the resource cannot be found
        strcat(buffer, "Referrer-Policy: no-referrer\n");
        strcat(buffer, "Content-Type: text/html; charset=UTF-8\n"); // Set content type to html - as we will return a simple error 404 html page.
    }
    // Set Universal Headers (headers will be the same whether resource is found or not)
    time_t t;
    time(&t); // get current time
    strcat(buffer, "Date: "); strcat(buffer, ctime(&t)); // Set HTTP Date header
    strcat(buffer, "Server: Billy's Very Simple HTTP Server\n"); // Set HTTP Server Header
    strcat(buffer, "Last-Modified: Fri Dec 20 12:27:14 GMT 2019\n"); // Set HTTP Last-Modified Header 
    strcat(buffer, "\n"); // Newline needed to seperate HTTP response content from HTTP response headers
    printf("Built HTTP headers.");
    if (fd < 3) {
        // If file wasn't found, add error 404 html page to response.
        strcat(buffer, "<html><head><title>Error 404</title></head><body><h1>Error 404</h1><p>Sorry, the requested file could not be found.</p></body></html>");
    }
}