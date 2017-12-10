#include <stdio.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <fstream>


#define SIZE       8
#define PI         3.141592653589793
#define TWO_PI     (2.0 * PI)
#define SWAP(a,b)  tempr=(a);(a)=(b);(b)=tempr




using namespace std;

//this struct from the TA and
//https://stackoverflow.com/questions/11770451/what-is-the-meaning-of-attribute-packed-aligned4
struct Header{
    //sub 1
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    //sub 2
    char     fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;
    uint16_t trash;
    //sub 3
    char     datachunk_id[4];
    uint32_t datachunk_size;
}__attribute__((packed,aligned(1)));

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

void zeropadding(vector<int16_t> &samples,int paddedSize,vector<double> paddedsamples){

    vector<double> unpadded(samples.size());
    for (int i = 0; i < samples.size();i++){
        unpadded[i] = (double)samples[i] /(-32768.0) * -1;
    }

    cout << "nn "<< paddedSize<< endl;
    //padding stat here
    int datachunk_size = samples.size();
    paddedsamples.resize(paddedSize);

    for (int i=0; i < paddedSize;i++){
        //if this odd index put 0
        if (i % 2 ){
            paddedsamples[i] =0.0;
        }
        //if this is even index put 1
        else{
            //if this index is out of bound of sample put 0
            if (i/2 < datachunk_size){
                paddedsamples[i] = unpadded[i/2];
            }
            else{
                paddedsamples[i] = 0.0;
            }

        }

    }
    for (int i = paddedsamples.size(); i < 2*paddedSize; i++){
        paddedsamples.push_back(0);
    }

}


//  The four1 FFT from Numerical Recipes in C,
//  p. 507 - 508.
//  Note:  changed float data types to double.
//  nn must be a power of 2, and use +1 for
//  isign for an FFT, and -1 for the Inverse FFT.
//  The data is complex, so the array size must be
//  nn*2. This code assumes the array starts
//  at index 1, not 0, so subtract 1 when
//  calling the routine (see main() below).

void four1(vector<double> &data, int nn, int isign)
{
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;

    for (i = 1; i < n; i += 2) {
    	if (j > i) {
            cout<< "swap"<<endl;
    	    SWAP(data[j], data[i]);
    	    SWAP(data[j+1], data[i+1]);
    	}
    	m = nn;
    	while (m >= 2 && j > m) {
    	    j -= m;
    	    m >>= 1;
    	}
    	j += m;
    }
    cout<< "here1" <<endl;
    mmax = 2;
    while (n > mmax) {
    	istep = mmax << 1;
    	theta = isign * (6.28318530717959 / mmax);
    	wtemp = sin(0.5 * theta);
    	wpr = -2.0 * wtemp * wtemp;
    	wpi = sin(theta);
    	wr = 1.0;
    	wi = 0.0;
    	for (m = 1; m < mmax; m += 2) {
    	    for (i = m; i <= n; i += istep) {
    		j = i + mmax;
    		tempr = wr * data[j] - wi * data[j+1];
    		tempi = wr * data[j+1] + wi * data[j];
    		data[j] = data[i] - tempr;
    		data[j+1] = data[i+1] - tempi;
    		data[i] += tempr;
    		data[i+1] += tempi;
    	    }
    	    wr = (wtemp = wr) * wpr - wi * wpi + wr;
    	    wi = wi * wpr + wtemp * wpi + wi;
    	}
    	mmax = istep;
    }
}




//Calculate complex number
void complex(vector<double> &X, vector<double> &K , vector<double> &Y){
    Y.resize(X.size());
    for (int i =0; i < X.size(); i+=2){
        Y[i] = X[i]*K[i] - X[i+1]*K[i+1];
        Y[i+1] = X[i+1]*K[i] + X[i]* K[i+1];
    }
}
//from the test.c on d2l
void scale(vector<double> &input){
    int N = input.size();
    // Calculate amplitude
    for (int i = 0; i< N;  i ++) {
    // Scale results by N
        input[i] = input[i] / (double)N;
    }
}
//remove peak
void removePeak(vector<double> &input){
    for (int i = 0; i< input.size();  i ++) {
        input[i] *= 0.8;
    }
}
//convert back to int
void toInt(vector<double> &in, vector<int16_t> &out){
    for (int i = 0; i < in.size();i++){
        out[i] = static_cast<int>(in[i] * 32768) ;
    }
}



//main
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
    //pad data
    vector<double> dataDoublePadded(sounddata.size());
    vector<double> irDoublePadded(irdata.size());

    //calculates needed length
    int longest = sounddata.size();
    if (irdata.size() > sounddata.size())
        longest = irdata.size();
    longest /= 2;
    //get nn
    double n = log(longest)/log(2);
    double n1 = ceil(n);
    int nn = int(pow(2,n1));
    cout <<"padding" <<endl;
    zeropadding(sounddata,nn,dataDoublePadded);
    cout << "size unpadded "<< sounddata.size()<<endl;
    cout << "size padded "<< dataDoublePadded.size() << endl;
    zeropadding(irdata,nn,irDoublePadded);
    //use fft on paddedsamples
    cout << "FFT"<<endl;
    four1( dataDoublePadded,nn , 1);
    cout << "FFT"<<endl;
    four1( irDoublePadded, nn, 1);
    //do complex multiply
    vector<double> resultDoublePadded(dataDoublePadded.size());
    complex(dataDoublePadded,irDoublePadded,resultDoublePadded);
    //do IFFT
    cout<<"iFFT"<< endl;
    four1( resultDoublePadded,nn , -1);
    //Scale
    scale(resultDoublePadded);
    //remove imaginary
    vector<double> resultDouble(soundHeader.datachunk_size);
    for (int i = 1 ; i < resultDouble.size(); i ++ ){
        resultDouble[i] = resultDoublePadded[i*2];
    }
    //remove peak
    removePeak(resultDouble);
    //to int
    vector<int16_t> result(soundHeader.datachunk_size);
    toInt(resultDouble,result);
    //bulid out header
    Header resultHeader = soundHeader;
    resultHeader.datachunk_size = result.size();
    resultHeader.chunk_size = 38+result.size();
    //    write file
    fileWritter(resultHeader,result, outputname);




    return 0;
}
