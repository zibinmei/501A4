#include <stdio.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <fstream>
#include <string>


using namespace std;

//this struct from the TA and
//https://stackoverflow.com/questions/11770451/what-is-the-meaning-of-attribute-packed-aligned4
struct Header{
    //sub 1
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    //sub 2
    char fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;
    uint16_t trash;
    //sub 3
    char datachunk_id[4];
    uint32_t datachunk_size;
    //define default
    // Header(): chunk_id("RIFF"),chunk_size(0),format("WAVE"), fmtchunk_id("ftm "), fmtchunk_size(18), audio_format(1),num_channels(1),sample_rate(44100),byte_rate(44100*16), block_align(2), bps(16),trash(0),datachunk_id("data"){}
}__attribute__((packed));

void fileReader(char *filename, Header &fileHeader,vector<int16_t> &result){
    cout<< "reading file: "<< filename<<endl;
    FILE* file = fopen(filename, "rb");
    fread(&fileHeader, sizeof(fileHeader), 1, file);
    fclose(file);

//http://www.cplusplus.com/reference/fstream/ofstream/open/
//https://stackoverflow.com/questions/36644616/read-a-binary-file-into-a-stdvectoruint16-t-instead-of-stdvectorchar
    ifstream ifs(filename, ios::binary|ios::ate);

    result.resize(fileHeader.datachunk_size);
    ifs.seekg(48);
    int i= 0;
    while (ifs.read((char *)&result[i], 2)){
        i++;
    }
}

void fileWritter(Header &fileHeader, vector<int16_t> &data, char *filename){
    cout<<"writting to: " << filename<< endl;
    FILE* file = fopen(filename,"wb");
    fwrite(&fileHeader, sizeof(fileHeader), 1, file);
    fclose(file);
    //written the data
    ofstream ofs;
    cout << "outsize: "<< data.size() << endl;
    ofs.open (filename, ofstream::app | ofstream::binary);
    const char* pointer = reinterpret_cast<const char*>(&data[0]);
    ofs.write(pointer, data.size()*sizeof(int16_t));

    ofs.close();


}

//convolve from prof
void convolve(vector<float> &x, int N, vector<float> &h, int M, vector<float> &y, int P)
{
    cout<< "convolve..." << endl;
  int n, m;

  /*  Make sure the output buffer is the right size: P = N + M - 1  */
  if (P != (N + M - 1)) {
    printf("Output signal vector is the wrong size\n");
    printf("It is %-d, but should be %-d\n", P, (N + M - 1));
    printf("Aborting convolution\n");
    return;
  }

  /*  Clear the output buffer y[] to all zero values  */
  for (n = 0; n < P; n++)
    y[n] = 0.0;

  /*  Do the convolution  */
  /*  Outer loop:  process each input value x[n] in turn  */
  for (n = 0; n < N; n++) {
    /*  Inner loop:  process x[n] with each sample of h[]  */
    for (m = 0; m < M; m++)

      y[n+m] += x[n] * h[m];
  }
}


void toFloat(vector<int16_t> &in, vector<float> &out){
    for (int i = 0; i < in.size();i++){
        out[i] = (float)in[i] /(-32768.0) * -1;
    }
}

void toInt(vector<float> &in, vector<int16_t> &out){
    for (int i = 0; i < in.size();i++){
        out[i] = static_cast<int>(in[i] * 32768) ;
    }
}

int main(int argc, char *argv[])
{

    //get input
    if ( argc != 4){
        printf("invalid parameter. Try: %s <soundfile> <irfile> <output>\n", argv[0]);
        return 0;
    }
    char * soundfile = argv[1];
    char * irfile = argv[2];
    char * outputname = argv[3];
    Header soundHeader;
    Header irHeader;
    vector<int16_t> sounddata;
    vector<int16_t> irdata;
    //read file
    fileReader(soundfile, soundHeader,sounddata);
    fileReader(irfile, irHeader,irdata);
    //turn data to float
    vector<float> datafloat(sounddata.size());
    vector<float> irfloat(irdata.size());
    toFloat(sounddata,datafloat);
    toFloat(irdata,irfloat);
//    convolve
    vector<float> outputfloat;
    outputfloat.resize(datafloat.size()+irfloat.size()-1);
    convolve(datafloat,datafloat.size(), irfloat,irfloat.size(), outputfloat,outputfloat.size());
    //bulid out header
    Header resultHeader = soundHeader;
    resultHeader.datachunk_size = outputfloat.size();
    resultHeader.chunk_size = 38+outputfloat.size();
    //redo output
    vector<int16_t> output(outputfloat.size());
    toInt(outputfloat,output);
//    write file
    fileWritter(resultHeader,output,outputname);
    //
    cout << "chunk_id: "<< soundHeader.chunk_id<< "   "<<sizeof(soundHeader.chunk_id) << endl;
    cout << "chunk_size: "<<soundHeader.chunk_size<<endl;
    cout << "format: "<< soundHeader.format<<endl;
    cout << "sub1 id: "<<soundHeader.fmtchunk_id<<endl;
    cout << "fmtchunk_size: "<<soundHeader.fmtchunk_size<<endl;
    cout << "sub2 id: "<<soundHeader.datachunk_id<<endl;
    cout << "datachunk_size: "<<soundHeader.datachunk_size<<endl;


    return 0;
}
