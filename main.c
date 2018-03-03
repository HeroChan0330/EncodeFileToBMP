#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <math.h>
#include <memory.h>

#define WINDOWSBM 19778
typedef uint16_t UINT16;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint32_t LONG;
typedef uint8_t byte;

typedef struct tagBITMAPFILEHEADER
{
UINT16 bfType;
DWORD bfSize;
UINT16 bfReserved1;
UINT16 bfReserved2;
DWORD bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER
 {
DWORD biSize;
LONG biWidth;
LONG biHeight;
WORD biPlanes;
WORD biBitCount;
DWORD biCompression;
DWORD biSizeImage;
LONG biXPelsPerMeter;
LONG biYPelsPerMeter;
DWORD biClrUsed;
DWORD biClrImportant;
} BITMAPINFOHEADER;
typedef  struct _bmp
{
BITMAPFILEHEADER file; //文件信息区
BITMAPINFOHEADER info; //图象信息区
DWORD stride;//每行像素的比特数
byte*content;
}BMP;

typedef struct _color{
uint8_t B;
uint8_t G;
uint8_t R;
}Color;

BMP*ReadBMPFile(char*path){
    FILE*fp=fopen(path,"rb+");
    BMP*result=(BMP*)malloc(sizeof(BMP));
    fread(&result->file,1,14,fp);
    fread(&result->info,1,40,fp);
    result->content=(byte*)malloc(result->info.biSizeImage);//分配内存
    fread(result->content,result->info.biSizeImage,1,fp);
    fclose(fp);
    result->stride=(((result->info.biWidth*result->info.biBitCount)+31)>>5)<<2;

    return result;
}

BMP*CreateBMP(int width,int height){
    BMP*result=(BMP*)malloc(sizeof(BMP));
    result->file.bfType=19778;
    result->file.bfSize=4;
    result->file.bfReserved1=0;
    result->file.bfReserved2=54;
    result->file.bfOffBits=977534976;//不知道这个值有什么含义，反正就是这样....
    result->info.biSize=40;
    result->info.biWidth=width;
    result->info.biHeight=height;
    result->info.biPlanes=1;
    result->info.biBitCount=24;
    result->info.biCompression=0;
    //result->info.biSizeImage=
    result->info.biXPelsPerMeter=144;
    result->info.biYPelsPerMeter=144;
    result->info.biClrUsed=0;
    result->info.biClrImportant=0;

    result->stride=(((result->info.biWidth*result->info.biBitCount)+31)>>5)<<2;
    result->info.biSizeImage=result->stride*result->info.biHeight;
    result->content=(byte*)malloc(result->info.biSizeImage);//分配内存
    return result;
}

void DisposeBMP(BMP*bmp){
    free(bmp->content);
    free(bmp);
    bmp=NULL;
}
void SaveBmpAs(char*path,BMP*bmp){
    FILE*fp=fopen(path,"wb+");
    fwrite(&bmp->file,1,14,fp);
    fwrite(&bmp->info,1,40,fp);
    fwrite(bmp->content,1,bmp->info.biSizeImage,fp);
    fclose(fp);
}


void PrintBMPInfo(BMP*m){
    printf("类型为%d\n",m->file.bfType);
    printf("文件大小为%d\n",m->file.bfSize);
    printf("保留字1为%d\n",m->file.bfReserved1);
    printf("保留字2为%d\n",m->file.bfReserved2);
    printf("偏移量为%d\n",m->file.bfOffBits);
    printf("此结构大小为%d\n",m->info.biSize);
    printf("位图的宽度为%d\n",m->info.biWidth);
    printf("位图的高度为%d\n",m->info.biHeight);
    printf("目标设备位图数%d\n",m->info.biPlanes);
    printf("颜色深度为%d\n",m->info.biBitCount);
    printf("位图压缩类型%d\n",m->info.biCompression);
    printf("位图大小%d\n",m->info.biSizeImage);
    printf("位图水平分辨率为%d\n",m->info.biXPelsPerMeter);
    printf("位图垂直分辨率为%d\n",m->info.biYPelsPerMeter);
    printf("位图实际使用颜色数%d\n",m->info.biClrUsed);
    printf("位图显示中比较重要颜色数%d\n",m->info.biClrImportant);
    printf("每行占字节%d\n",m->stride);
}

BMP*StoreFileToBMP(char*path);
void DecodeFileFromBMP(BMP*bmp,char*dest);
char* DecodeFileFromBMP2(BMP*bmp);
int main(int args,char*argv[]){
    if(args!=4){
        printf("BadRequest\n");
        return 1;
    }
    if(strcmp(argv[1],"encode")==0){
        BMP*res=StoreFileToBMP(argv[2]);
        SaveBmpAs(argv[3],res);
        DisposeBMP(res);
        printf("done!");
    }
    else if(strcmp(argv[1],"decode")==0){
        BMP*src=ReadBMPFile(argv[2]);
        DecodeFileFromBMP(src,argv[3]);
        DisposeBMP(src);
        printf("done!");
    }

    return 0;
}

uint32_t GetFileSize(FILE*fp){
    if(!fp) return -1;
    fseek(fp,0L,SEEK_END);
    int size=ftell(fp);
    fseek(fp,0,SEEK_SET);
    return size;
}

BMP*StoreFileToBMP(char*path){
    FILE*fp=fopen(path,"rb+");
    int size=GetFileSize(fp);
    //暂时将文件大小写在保留字1里面
    //if(size>65535) return NULL;
    //修改：将文件大小写在左下角4个像素
    int w=(int)ceil(sqrt(ceil(size/3)));//文件内容的宽度
    int h=ceil(size/(float)(w*3));//文件内容的像素高度
    BMP*result=CreateBMP(w+2,w+2);

    //result->file.bfReserved1=size;//旧设计
    byte*ptr=result->content;

    ptr[0]=size&0xff;
    ptr[1]=(size>>8)&0xff;
    ptr[2]=(size>>16)&0xff;
    ptr[3]=(size>>24)&0xff;
    ptr+=result->stride;

    for(int y=1;y<=h;y++){
        ptr+=3;
        fread(ptr,1,w*3,fp);
        ptr+=result->stride-3;
    }

    return result;
}

void DecodeFileFromBMP(BMP*bmp,char*dest){
    byte*ptr=bmp->content;
    //int size=bmp->file.bfReserved1;
    int size=ptr[0]|(ptr[1]<<8)|(ptr[2]<<16)|(ptr[3]<<24);

    int w=(int)ceil(sqrt(ceil(size/3)));//文件内容的宽度
    int h=ceil(size/(float)(w*3));//文件内容的像素高度

    FILE*fp=fopen(dest,"wb+");

    ptr+=bmp->stride;
    for(int y=1;y<h;y++)
    {
        ptr+=3;
        fwrite(ptr,1,w*3,fp);
        ptr+=bmp->stride-3;
    }
    ptr+=3;
    fwrite(ptr,1,size-w*3*(h-1),fp);
    fclose(fp);
}


char* DecodeFileFromBMP2(BMP*bmp){
    int size=bmp->file.bfReserved1;
    int w=(int)ceil(sqrt(ceil(size/3)));//文件内容的宽度
    int h=ceil(size/(float)(w*3));//文件内容的像素高度
    byte*ptr=bmp->content;
    char*result=(char*)malloc(size+1);
    char*resPtr=result;

    ptr+=bmp->stride;
    for(int y=1;y<h;y++)
    {
        ptr+=3;
        memcpy(resPtr,ptr,w*3);
        resPtr+=w*3;
        ptr+=bmp->stride-3;
    }
    ptr+=3;
    memcpy(resPtr,ptr,size-w*3*(h-1));
    return result;
}

