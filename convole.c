#include <stdio.h>





void fileReader(char *fname){
    //https://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
    FILE *fptr;
    char *buffer;
    long filelen;

    fptr = fopen(fanme,"rb");
    if (fptr == NULL){
        exit(0);
    }


    fseek(fptr, 0, SEEK_END);
    filelen = ftell(fptr);
    rewind(fptr);

    buffer = (char *)malloc((filelen+1)*sizeof(char));
    fread(buffer, filelen, 1, fptr);
    fclose(fptr);



    printf("%x ", buffer);


}


int main (int argc, char *argv[]){
    if (argc != 4){
        printf("Try:./convole input IRfile outputfile\n");
        return 0;
    }


}
