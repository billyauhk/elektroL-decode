/*
l15data2pbm.c
 An experimental Russia Elektro-L Level 1.5 raw data file to PBM convertor.
 The data could be donwloaded at ftp://electro:electro@ftp.ntsomz.ru/
 It is observed that the raw data is at the last bytes of the file.
 And we think we could skip the file headers owing to this observed/empirical property.

 Author: Bill Yau
 Starting date: 25Aug2013

 Invocation: ./l15data2pbm {filename}
 Then the L15 file will be converted to PBM (without combining colors)

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>

int main(int argc, char* argv[]){
  int retVal;             // Temporary store of return values
  int infile, outfile;    // File Handles
  long long filesize;     // File size
  unsigned short channel; // Channel Name (1-3: High Res, 4-9,0: Low Res)
  struct stat sb;         // Struct for stat()
  unsigned int readCount, i;
#define BUFFER_SIZE 2000
  char buffer[BUFFER_SIZE]; // Buffer for outfile name (automatically named by changing L15 to pbm)
  char tempByte;

// Check Arguments
  if(argc>1){
    printf("Filename: %s\n",argv[1]);
  }else{
    printf("No filename!\n");
    exit(-1);
  }

// Check input file size
  if((retVal = stat(argv[1],&sb)) == -1){
    perror("stat");exit(-1);
  }else{
    filesize = (long long) sb.st_size;
    printf("File size: %lld bytes\n",filesize);
  }
// Check channel name
// Using a dirty heuristic: _.L15 (the digit at _ is unique)
  channel = argv[1][strlen(argv[1])-5]-'0';
  printf("Channel number: %u\n",channel);

// Open Files
  if((infile = open(argv[1],O_RDONLY)) == -1){
    perror("Cannot open input file");exit(-1);
  }else{
    printf("File opened\n");
  }
  // Generate output file name
  strncpy(buffer,argv[1],BUFFER_SIZE);
  if(buffer[BUFFER_SIZE-1] != '\0'){
    printf("File name too long!\n");exit(-1);
  }
  strncpy(buffer+strlen(buffer)-3,"pbm",3);
  printf("Output file name: %s\n",buffer);
  if((outfile = open(buffer,O_WRONLY|O_CREAT|O_TRUNC,0644)) == -1){
    perror("Cannot open output file");
  }

// Seek to position and write pbm header
  switch(channel){
    case 1: case 2: case 3:
    /* High resolution data: 11136*11136 of 2 bytes, 10 bits dynamic range */
      if((retVal = lseek(infile,-11136*11136*2,SEEK_END)) == -1){
        printf("Size; %d\n",11136*11136*2);
        perror("Cannot seek to position");exit(-1);
      }else{
        printf("Moved to the %d-th byte\n",retVal);
      }
      if((retVal = snprintf(buffer,BUFFER_SIZE,"P5 11136 11136 1023\n")) < 0){
        perror("snprintf error");exit(-1);
      }
      if((retVal = write(outfile,buffer,retVal)) == -1){
        perror("Write PBM header error");exit(-1);
      }
      break;
    case 4: case 5: case 6: case 7:
    case 8: case 9: case 0:
    /* Low resolution data: 2784*2784 of 2 bytes, 8 bits dynamic range (but DN is still 10 bits)*/
      if((retVal = lseek(infile,-2784*2784*2,SEEK_END)) == -1){
        perror("Cannot seek to the position");exit(-1);
      }else{
        printf("Moved to the %d-th byte\n",retVal);
      }
      if((retVal = snprintf(buffer,BUFFER_SIZE,"P5 2784 2784 1023\n")) < 0){
        perror("snprintf error");exit(-1);
      }
      if((retVal = write(outfile,buffer,retVal)) == -1){
        perror("Write PBM header error");exit(-1);
      }
      break;
    default:
      printf("Unknown channel name...\n");exit(-1);
      break;
  }

// Extract data, convert endianess and output
  while((readCount = read(infile,buffer,BUFFER_SIZE)) != 0){
    // Swap bytes and fix the endianess
    for(i=0;i<(readCount >> 1);i++){
      buffer[2*i+1] ^= buffer[2*i+0];
      buffer[2*i+0] ^= buffer[2*i+1];
      buffer[2*i+1] ^= buffer[2*i+0];
    }
    if((retVal = write(outfile,buffer,readCount)) != readCount){
      perror("Something wrong with output file writing");
    }
  }

  if((retVal = close(infile)) == -1){
    perror("Cannot close input file");
  }
  if((retVal = close(outfile)) == -1){
    perror("Cannot close output file");
  }

  printf("Program ends here\n");
  return 0;
}
