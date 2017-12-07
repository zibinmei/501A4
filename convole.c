#include <stdio.h>


FILE *fptr;
char *buffer;
long filelen;

int main (int argc, char *argv[]){
    if (argc != 4){
        printf("Try:./convole input IRfile outputfile\n");
        return 0;
    }

    //https://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
    fptr = fopen(argv[1],"rb");
    fseek(fptr, 0, SEEK_END);
    filelen = ftell(fptr);
    rewind(fptr);

    buffer = (char *)malloc((filelen+1)*sizeof(char));
    fread(buffer, filelen, 1, fptr);
    fclose(fptr);
    int i = 0;
    while(1){
        if (buffer[i] == '\0') break;
        printf("%u ", buffer[i]);
    }

}
