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
BITMAPFILEHEADER file; //�ļ���Ϣ��
BITMAPINFOHEADER info; //ͼ����Ϣ��
DWORD stride;//ÿ�����صı�����
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
    result->content=(byte*)malloc(result->info.biSizeImage);//�����ڴ�
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
    result->file.bfOffBits=977534976;//��֪�����ֵ��ʲô���壬������������....
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
    result->content=(byte*)malloc(result->info.biSizeImage);//�����ڴ�
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
    printf("����Ϊ%d\n",m->file.bfType);
    printf("�ļ���СΪ%d\n",m->file.bfSize);
    printf("������1Ϊ%d\n",m->file.bfReserved1);
    printf("������2Ϊ%d\n",m->file.bfReserved2);
    printf("ƫ����Ϊ%d\n",m->file.bfOffBits);
    printf("�˽ṹ��СΪ%d\n",m->info.biSize);
    printf("λͼ�Ŀ��Ϊ%d\n",m->info.biWidth);
    printf("λͼ�ĸ߶�Ϊ%d\n",m->info.biHeight);
    printf("Ŀ���豸λͼ��%d\n",m->info.biPlanes);
    printf("��ɫ���Ϊ%d\n",m->info.biBitCount);
    printf("λͼѹ������%d\n",m->info.biCompression);
    printf("λͼ��С%d\n",m->info.biSizeImage);
    printf("λͼˮƽ�ֱ���Ϊ%d\n",m->info.biXPelsPerMeter);
    printf("λͼ��ֱ�ֱ���Ϊ%d\n",m->info.biYPelsPerMeter);
    printf("λͼʵ��ʹ����ɫ��%d\n",m->info.biClrUsed);
    printf("λͼ��ʾ�бȽ���Ҫ��ɫ��%d\n",m->info.biClrImportant);
    printf("ÿ��ռ�ֽ�%d\n",m->stride);
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
    //��ʱ���ļ���Сд�ڱ�����1����
    //if(size>65535) return NULL;
    //�޸ģ����ļ���Сд�����½�4������
    int w=(int)ceil(sqrt(ceil(size/3)));//�ļ����ݵĿ��
    int h=ceil(size/(float)(w*3));//�ļ����ݵ����ظ߶�
    BMP*result=CreateBMP(w+2,w+2);

    //result->file.bfReserved1=size;//�����
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

    int w=(int)ceil(sqrt(ceil(size/3)));//�ļ����ݵĿ��
    int h=ceil(size/(float)(w*3));//�ļ����ݵ����ظ߶�

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
    int w=(int)ceil(sqrt(ceil(size/3)));//�ļ����ݵĿ��
    int h=ceil(size/(float)(w*3));//�ļ����ݵ����ظ߶�
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

