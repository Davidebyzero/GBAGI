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
#pragma hdrstop

#include "main.h"
#include "e:\programming\gbagi\gbagi.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
/******************************************************************************/
U8 *vocabData;
char fname[1024];
/******************************************************************************/

/******************************************************************************/
U8 *LoadFile(BOOL G_PATH, char *name, int *len)
{
	FILE *f;
	int l;
    U8 *p;

    strcpy(fname,name);
	if((f=fopen(fname,"rb")) == NULL) {
		ShowMessage("Unable to open file for reading!");
		return FALSE;
	}
	fseek(f,0,SEEK_END);
	l=ftell(f);
	fseek(f,0,SEEK_SET);

	p = (U8*) malloc(l);
	fread(p, l, 1, f);
	fclose(f);

    if(len)
    	*len = l;

    return p;
}
/******************************************************************************/
int FindWordx(char *s,U8 *b)
{
	char *s1,*s2;
	while(*b) {
    	s1 = (char*)b+3;
        s2 = s;
        while(*s1&&*s1++==*s2) s2++;
     	if(!*s1&&(*s2==' '||*s2=='\0'))
        	return (b[1]+(b[2]<<8));
        b+=b[0]+1;
    }
    return -1;
}
/******************************************************************************/
U8 *FindWordY(char *s,U8 *b)
{
	while(*b) {
    	if(strcmp(s, (char*)b+3)==0)
        	return b;
        b+=b[0]+1;
    }
    return NULL;
}
/******************************************************************************/

__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	if((vocabData = LoadFile(FALSE, "E:\\programming\\gbagi\\gbarom\\vocab.bin", NULL))==NULL)
    	Close();

    U8 *b=vocabData;
    int wc=0;
	while(*b) {
    	if(b[3]<'a'||b[3]>'z') {
         	b[1] = 0xFF;
            b[2] = 0xFF;
        } else {
    		ListBox1->Items->Add(AnsiString((char*)b+3));
            wc++;
        }
        b+=b[0]+1;
    }
    Label1->Caption = IntToStr(wc);
}

/******************************************************************************/
void __fastcall TForm1::ListBox1Click(TObject *Sender)
{
	CheckBox1->OnClick = NULL;
	CheckBox1->Checked =
    	FindWordY(ListBox1->Items->Strings[ListBox1->ItemIndex].c_str(),vocabData)[1]&0x80?TRUE:FALSE;
	CheckBox1->OnClick = CheckBox1Click;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CheckBox1Click(TObject *Sender)
{
	ListBox1->OnClick = NULL;
	FindWordY(ListBox1->Items->Strings[ListBox1->ItemIndex].c_str(),vocabData)[1] ^= 0x80;
	ListBox1->OnClick = ListBox1Click;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{
	FILE *f = fopen("E:\\programming\\gbagi\\gbarom\\vocab.bin","wb");

    U8 *b=vocabData,*b2;
    char *s;
	while(*b) {
    	if(b[1]!=0xFF||b[2]!=0xFF) {
        	s = (char*)b+3;
            b2 = b+b[0]+1;
			while(*b2) {
             	if(strcmp(s,(char*)b2+3)==0) {
                 	if(b2[1]&0x80)
                    	b[1]|=0x80;
                    b2[1]=0xFF;
                    b2[2]=0xFF;
                }   
        		b2+=b2[0]+1;
            }
        }
        b+=b[0]+1;
    }

    b=vocabData;
	while(*b) {
    	if(b[1]!=0xFF||b[2]!=0xFF) {
         	fputc(b[0],f);
            if(b[1]&0x80)
            	fputc(0x80,f);
            else
            	fputc(0,f);
            fputc(0,f);
            fwrite(b+3,b[0]-2,1,f);
        }
        b+=b[0]+1;
    }
	fputc(0,f);
    fclose(f);
}
//---------------------------------------------------------------------------
