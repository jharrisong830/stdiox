/**
 * NAME:    John Graham
 * PLEDGE:  I pledge my honor that I have abided by the Stevens Honor System.
 * 
 * FILE: stdiox.c
*/


#include "stdiox.h"


/* HELPERS */

/**Converts num from an int to a string, and writes it to the file specified by fd*/
int itoa(int fd, int num) {
    char charnum;
    char strnum[PATH_MAX]=""; //creating a string representation of num
    if(num==0) { //if num is 0...
        if(write(fd, "0", 1) != 1) { //...write 0 and return (no other work needed)
            errno=EIO;
            return -1;
        }
        return 0;
    }
    else if(num<0) { //if num is negative...
        num=-num; //..reverse the sign...
        if(write(fd, "-", 1) != 1) { //...and write a '-', then write the rest of the number as normal
            errno=EIO;
            return -1;
        }
    }
    int i=0; //counter
    while(num!=0) { //while num is not 0...
        charnum=(48+(num%10)); //extract rightmost digit, add 48 for ascii conversion
        num=num/10; //discard rightmost digit
        strnum[i]=charnum; //add char digit to string!
        i++; //increment counter
    }
    for(int i=strlen(strnum)-1; i>=0; i--) { //iterate backwards over the string to write (added in reverse order)
        if(write(fd, &strnum[i], 1) !=1) { //writing one byte at a time to the file
            errno=EIO; //exit with error if needed
            return -1;
        }
    }
}

/**Converts num from a float to a string, and writes it to the file specified by fd*/
int ftoa(int fd, float num) {
    char strnum[PATH_MAX]="";
    if(num==0) { //if num is 0...
        if(write(fd, "0", 1) != 1) { //...write 0 and return (no other work needed)
            errno=EIO;
            return -1;
        }
        return 0;
    }
    else if(num<0) { //if num is negative...
        num=-num; //..reverse the sign...
        if(write(fd, "-", 1) != 1) { //...and write a '-', then write the rest of the number as normal
            errno=EIO;
            return -1;
        }
    }

    int intnum=(int)num; //integer portion of the number (before the .)
    float decimals=num-intnum; //decimal portion of the number (after the .)
    char charnum;
    int i=0;
    itoa(fd, intnum); //call itoa on the integer portion of this number
    if(decimals>0) {
        char point='.';
        if(write(fd, &point, 1) !=1) { //writing one byte at a time to the file
            errno=EIO; //exit with error if needed
            return -1;
        }

        while(decimals>0) { //while there is still value after the decimal point...
            int next_digit=(int)(decimals*10); //leftmost digit after the decimal point
            char next_char=next_digit+48; //character representation of that digit
            decimals=decimals*10; //multiply the decimals by 10...
            decimals=decimals-next_digit; //...then subtract the next digit (cutting off that digit)
            if(write(fd, &next_char, 1) !=1) { //writing one byte at a time to the file
                errno=EIO; //exit with error if needed
                return -1;
            }
        }
    }
    return 0;
}


/* MAIN FUNCTIONS */

/**Prints data of the specified format to the file specified by filename*/
int fprintfx(char* filename, char format, void* data) {
    int fd=1; //set initial fd as 1 (stdout, the terminal)
    if(strcmp(filename, "")!=0) { //if filename not empty, then open a file
        if((fd=open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP)) == -1) { //opens a file to write/append to (creates new file if it doesn't already exist, with rw-r----- perms)
            errno=EIO;
            return -1;
        } //remove this if block & uncomment dirent (at bottom of this file) to keep doing cancel characters
        char* newline="\n"; //for if we need to write a newline
        off_t seek_check;
        if((seek_check=lseek(fd, 0, SEEK_END)) == -1) { //get our current position in the file, store in seek_check
            errno=EIO;
            return -1;
        }
        else if(seek_check!=0) { //if seek_check!=0, then there's content in the file...
            write(fd, newline, 1); //...so we should start on a newline
        } //otherwise, don't write a newline, since we are at the start of a file (and don't want a newline immediately)
    }
    if(data==NULL || (format!='d' && format!='s' && format!='f')) { //if we're given invalid input...
        errno=EIO; //...set error code...
        return -1; //...and return -1 :(
    }
    if(format=='d') { //integer numbers
        int num=*(int*)data; //dereference the data
        itoa(fd, num);
    }
    else if(format=='f') {
        float num=*((float*)data);
        ftoa(fd, num);
    }
    else if(format=='s') { //strings
        int i=0;
        char curr_char=*((char*)data); //getting the first char from data
        while(curr_char!='\0') { //iterate over all of data
            if(write(fd, &curr_char, 1) != 1) { //write one byte at a time from data to the file
                errno=EIO; //exit with error if needed
                return -1;
            }
            i++; //increment counter
            curr_char=*((char*)data+i); //move to the next char!
        }
    }

    if(fd==1) { //if writing to stdout...
        char* newline="\n";
        write(fd, newline, 1); //...always write a newline character at the end
    }
    return 0;
}

/**Scans one line from the file specified by filename of the specified format to the memory of dst*/
int fscanfx(char* filename, char format, void* dst) {
    int fd=0; //set initial fd as 0 (stdin, the keyboard)
    if(strcmp(filename, "")!=0) { //if filename not empty, then open a file
        DIR* fd_table=opendir("/proc/self/fd"); //open the fd table
        struct dirent* current_fd;
        struct stat main_file; //for reading inodes
        struct stat dirent_file;
        stat(filename, &main_file); //get the inode of the file we're trying to open
        while((current_fd=readdir(fd_table)) != NULL) { //for each entry in the fd table
            fstat(atoi(current_fd->d_name), &dirent_file); //get the stat struct for that fd
            if(main_file.st_ino == dirent_file.st_ino) { //if inode numbers are the same...
                fd=atoi(current_fd->d_name); //...then manually set the fd...
                break; //...and break!
            }
        }
        free(fd_table); //free the fd table directory
        if(fd==0) { //if we haven't changed the fd yet, then make a call to open
            if((fd=open(filename, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP)) == -1) { //opens a file to write/append to (creates new file if it doesn't already exist, with rw-r----- perms)
                errno=ENOENT; //error if file doesn't exist
                return -1;
            }
        }
    }
    int i=0; //counter of total bytes read
    char* temp=malloc(128); //initially has 128 bytes in the buffer
    int eof_checker; //checks for end of file
    char buf_byte; //current byte being read from the file
    do {
        eof_checker=read(fd, temp+i, 1); //attempt to read into dst[i]
        if(eof_checker==-1) { //read error
            free(temp);
            errno=EIO;
            return -1;
        }
        else if(eof_checker==0) { //if we've reached end of file...
            break; //...break from loop!
        }
        buf_byte=*((char*)temp+i); //access the most recently read byte
        i++; //increment counter
        if(i%128==0) { //if we've reached a size limit...
            temp=realloc(temp, 128+i); //...then reallocate an additional 128 bytes
        }
    } while(buf_byte!='\n'); //end when we reach a new line

    if(i==0) { //if we break immediately (EOF)
        free(temp);
        return 1;
    }
    if(buf_byte=='\n') { //write null terminator at the end (& cut off newline if that was the last character we read in)
        ((char*)temp)[i-1]='\0';
    }
    else { //write null terminator at the end
        ((char*)temp)[i]='\0';
    }

    if(format=='d') {
        int num=atoi((char*)temp);
        memcpy(dst, &num, sizeof(int));
    }
    else if(format=='f') {
        float num=(float)atof((char*)temp);
        memcpy(dst, &num, sizeof(float));
    }
    else if(format=='s') {
        int len=strlen((char*)temp)+1; // +1 so that we can copy \0
        memcpy(dst, temp, len);
    }
    else { //if we're given invalid input...
        free(temp);
        errno=EIO; //...set error code...
        return -1; //...and return -1 :(
    }
    free(temp);
    return 0;
}

/**Closes all currently open file descriptors (except for stdin, stdout, & stderr)*/
int clean() {
    DIR* fd_table=opendir("/proc/self/fd"); //open the fd table
    struct dirent* current_fd;
    int fd;
    while((current_fd=readdir(fd_table)) != NULL) { //for each entry in the fd table
        fd=atoi(current_fd->d_name); //get fd from dirent name
        if(fd!=0 && fd!=1 && fd!=2) { //don't close stdin, stdout, stderr...
            if(close(fd)==-1) { //close the fd, set error if needed
                closedir(fd_table);
                errno=EIO;
                return -1;
            }
        }
    }
    closedir(fd_table); //free the fd table directory
    return 0;
}


//old code for opening files in print (using this will produce a cancel character 0x18)
// DIR* fd_table=opendir("/proc/self/fd"); //open the fd table
        // struct dirent* current_fd;
        // struct stat main_file; //for reading inodes
        // struct stat dirent_file;
        // if(stat(filename, &main_file) != -1) { //get the inode of the file we're trying to open
        //     while((current_fd=readdir(fd_table)) != NULL) { //for each entry in the fd table
        //         fstat(atoi(current_fd->d_name), &dirent_file); //get the stat struct for that fd
        //         if(main_file.st_ino == dirent_file.st_ino) { //if inode numbers are the same...
        //             if((fd=open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP)) == -1) { //opens a file to write/append to (creates new file if it doesn't already exist, with rw-r----- perms)
        //                 free(fd_table);
        //                 errno=EIO;
        //                 return -1;
        //             }
        //             break; //...and break!
        //         }
        //     }
        // }
        // free(fd_table); //free the fd table directory
        // if(fd==1) { //if we haven't found the file yet, then it doesn't exist...
        //     if((fd=open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP)) == -1) { //opens a file to write/append to (creates new file if it doesn't already exist, with rw-r----- perms)
        //         errno=EIO;
        //         return -1;
        //     }
        // }
