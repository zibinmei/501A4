#include <stdio.h>





void fileReader(char *fname){
//https://stackoverflow.com/questions/26370012/read-bytes-from-file-and-store-in-array-c
    FILE *fptr;
    char *buffer;
    long filelen;

    fptr = fopen(fname,"rb");
    if (fptr == NULL){
        printf("ERROR: Unable to open file\n");
        exit(0);
    }


    fseek(fptr, 0, SEEK_END);
    filelen = ftell(fptr);
    char bytes[filelen];
    rewind(fptr);

    buffer = (char *)malloc((filelen+1)*sizeof(char));
    fread(bytes, filelen, 1, fptr);
    fclose(fptr);



    printf("%x ", bytes[1]);


}


int main (int argc, char *argv[]){
    if (argc != 4){
        printf("Try:./convole input IRfile outputfile\n");
        return 0;
    }


}
