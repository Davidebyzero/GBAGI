/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>  
#include <string.h>
#pragma hdrstop

#include "maike.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
typedef unsigned char	U8;
typedef signed char		S8;
typedef unsigned short	U16;
typedef signed short	S16;
typedef unsigned long	U32;
typedef signed long		S32;
typedef int				BOOL;
//---------------------------------------------------------------------------
/*const*/	char *words[26];		// a-z
/*const*/	U32 words32[26];		// a-z
/*const*/	char *wordData;		// the raw data pointed to by "words[26]"
    char fname[100];
    int pos,l;
    U8 *volPtrs[16],*p,*msg,*wPtr,*nPtr,*iPtr,*end;
char wordbuf[100];
FILE *f;
U8 *b;
    char VocString[128];
/*****************************************************************************/
U16 bGetW(U8 *p)
{
	return (*p)+(p[1]<<8);
}
/*****************************************************************************/
U16 beGetW(U8 *p)
{
	return (p[1])+(p[0]<<8);
}
int FindWord(char *s,U8 *b)
{
	int g=-1;
	while(*b) {
		if(strcmp(s,(char*)b+3)==0) {
        	if(g==-1||(!(g&0x80)))
        		g = (b[1]+(b[2]<<8));
        }
        b+=b[0]+1;
    }
    return g;
}
//---------------------------------------------------------------------------
void DoVocab(char *fn)
{

	f=fopen(fn,"rb");
    fseek(f,0,SEEK_END);
    l=ftell(f)-2;
    fseek(f,2,SEEK_SET);
    b=(BYTE*)malloc(l);
    fread(b,l,1,f);
    fclose(f);
	f=fopen("E:\\programming\\gbagi\\gbarom\\vocab.bin","rb+");
    fseek(f,0,SEEK_END);
    if(ftell(f)>0)
    	fseek(f,-1,SEEK_END);

    int Offset = 52;
    int type,group;
    while(Offset < l) {
    	int i = b[Offset++];
        while((BYTE) (VocString[i++] = b[Offset++]) < 0x80);
        VocString[i-1] &= 0x7F;
        VocString[i] = '\0';
	    type  = b[Offset]&0xFF;
        type += (WORD)(((WORD)b[Offset+1]>>4) &0xF)<<8;
        group = (((b[Offset+1] << 8) & 0x0F00) + ((b[Offset+2]) & 0xFF)) & 0xFFF;
        Offset += 3;
        fputc(strlen(VocString)+3,f);
        fputc(type&0xFF,f);
        fputc(type>>8,f);
        fwrite(VocString,strlen(VocString)+1,1,f);
    }
    fputc(0,f);
    fclose(f);
    free(b);
}
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	f=fopen("E:\\programming\\gbagi\\gbarom\\vocab.bin","wb");
    fclose(f);

    DoVocab("C:\\WINDOWS\\Desktop\\scistudio3\\template\\vocab.000");
    DoVocab("E:\\scigames\\kq4\\vocab.000");
    DoVocab("E:\\scigames\\sq3\\vocab.000");
    DoVocab("E:\\scigames\\pq2\\vocab.000");
    DoVocab("E:\\scigames\\lsl2\\vocab.000");
    DoVocab("E:\\scigames\\lsl3\\vocab.000");
    DoVocab("E:\\scigames\\ci\\vocab.000");
    DoVocab("E:\\scigames\\lb1\\vocab.000");

	f=fopen("E:\\programming\\gbagi\\gbarom\\vocab.bin","rb");
    fseek(f,0,SEEK_END);
    l=ftell(f);
    fseek(f,0,SEEK_SET);
    b=(BYTE*)malloc(l);
    fread(b,l,1,f);
    fclose(f);



    char*path="E:\\agigames\\Leisure suit larry 1 (AGI)\\";
	sprintf(fname,"%swords.tok",path);
	if((f=fopen(fname,"rb")) == NULL) {
		ShowMessage("Unable to open file: \"words.tok\" for reading!");
		return;
	}
	fseek(f,0,SEEK_END);
	l=ftell(f);
	fseek(f,0,SEEK_SET);

	p = (U8*) malloc(l);
	fread(p, l, 1, f);
	fclose(f);

    wordData = (char*)malloc(64000);
    memset(words,0,sizeof(words));
    wPtr = wordData;
    int wc=0;
    int xa=0,xb=0;
    for(int i=0;i<26;i++) {
     	pos = 0;
        if(bGetW(p+(i<<1))==0) continue;
        msg = p + ((p[i<<1]<<8)|(p[(i<<1)+1]));
        words[i] = wPtr;
        do {
        	pos = *msg++;
            iPtr=wPtr;
            wPtr+=3;
        	do
        		wordbuf[pos++] = (*msg&0x7F)^0x7F;
        	while(*msg++<0x80);
       		wordbuf[pos++] = 0;

            l=strlen(wordbuf)+1;
            memcpy(wPtr,wordbuf,l);
            wPtr+=l;

            if((msg[1]+(msg[0]<<8)>0)&&((msg[1]+(msg[0]<<8))!=9999)) {
            int g = FindWord(wordbuf,b);
            if((g!=-1)&&(g&0x80)){xa++;
            	ListBox1->Items->Add(AnsiString(wordbuf));
            }else{    xb++;
            	ListBox2->Items->Add(AnsiString(wordbuf));
            }
            }

            iPtr[0] = l+3;
            iPtr[2] = *msg++;
            iPtr[1] = *msg++;
            wc++;
        } while(msg[0]);
        *wPtr++=0;
    }
    Label1->Caption=IntToStr(xa);
    Label2->Caption=IntToStr(xb);
}
//---------------------------------------------------------------------------
