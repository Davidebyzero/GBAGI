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

/******************************************************************************/
#include "makerom.h"
#include "decompress.h"
#include "commands.h"
#include <conio.h>
/******************************************************************************/

/******************************************************************************/
GAMEINFO *gi;

DIRENT dirs[4][256];
const char *dirNames[4] = {"logdir","picdir","viewdir","snddir"};

char fname[1024];
char wordbuf[128];

BOOL VUSED[16];
U8 *volData;
U32 volSize;

const char *objNames[256];
char *objNameData;
U8 objRoomsStart[256];

const char *words[26];
U8 *vocabData, *wordData;
U32 wordsSize;

WORDSET *wordset,*pwords;
FILE *fout;
U32 offs, giPos;


int lwfsize;
U8 *lwPtrs[256];
BOOL LOG_DONE[256];
int strSize;   
U8 *code;
/******************************************************************************/
const char *solidWords[] = {
	"look","marry", "take", "button", "phone", "hallway", "drinks", "hole", "cigarette",
	"yes", "no", "wall", "area", "wallet", "ground", "stairs", "window", "open", "move", "climb", "talk",
	"garbage", "ledge", "under", "over", "drawer", "carpet", "out", "key", "give",
	"lie down", "touch", "empty", "find", "behind", "door", "clothes", "lock", "break",
	"listen", "table", "shelf", "book", "thank you", "box", "jump", "man", "woman",
	"start", "ignite", "fire", "meter", "lips", "basket", "wine", "food", "steal", "knife",
	"cut", "sink", "sit down", "hit", "knock on", "cover", "bottle", "television", "put on",
	"ashtray", "bill", "pass", "ladder", "capsule", "negotiate", "flower", "elevator",
	"paper", "ken sent me", "feel", "bartender", "sleep", "pills", "water", "rope",
	"ring", "blow up", "release", "booze", "remote control", "whiskey", "bathroom",  "mat",
	"notes", "wash", "candy", "doll", "magazine", "old", "building", "get up", "change",
	"channel", "jukebox", "wipe", "attatch", "clothes line", "decrease", "increase",
	"spray", "bar", "church", "lint", "blackjack", "slots" , "dog", "pole", "rough",
	"plants", "boo", "music", "junk food", "gamble", "lever", "split", "rules",
	"swim suit", "moose", "undress", "dress", "artwork", "fan", "yourself", "fire hydrant",
	"enjoy", "disc jockey", "roof", "weapon", "onto", "person", "watch", "waiter", "spit",
	"shout", "dive", "under water", "bucket", "leg", "bubbles", "credit card", "pause",
	"suit", "cooler", "information", "cheer", "stop", "inventory", "crack", "crap", "leak",
	"screw", "fart", "boobs", "ass", "dong", "up yours", "business card", "work",
	"computer console", "transporter", "screen", "fuel pump", "close", "catch", "rock",
	"push", "position", "forest", "help", "swim", "climb", "ground", "plants", "smell", "stand up",
    "knife", "torch", "soup", "catch", "opening", "bag", "get in", "tree", "key", "light",
    "knight", "pump", "eat", "silence", "name", "bird", "feed", "diamond", "throw", "drop",
    "cast", "fence", "stove", "fly", "move", "woman", "remove", "river", "guitar", "initialize",
    "hat", "chair", "keyhole", "duck", "suite", "body", ""
};

#define SG_TOTAL 8
#define SG_WORDMAX 20

const char *solidGroups[SG_TOTAL][SG_WORDMAX] = {
	{"enter","exit", ""},
	{"taste", "eat", "drink", ""},
	{"hug", "kiss", ""},
	{"cabinet","closet","cupboard","wardrobe", ""}, 
	{"bureau","chest","dresser", ""},
	{"purchase", "pay", ""},
	{"car", "taxi", ""},
	{""}
};
/******************************************************************************/
int ErrorMessage(const char *s, ...)
{
	va_list argptr;
	int cnt;

	va_start(argptr, s);
	cnt = vprintf(s, argptr);
	va_end(argptr);

    getch();
    exit(1);

	return(cnt);
}
/******************************************************************************/
U16 fgetw(FILE *f)
{
	return (fgetc(f))+(fgetc(f)<<8);
}
/******************************************************************************/
U32 fgett(FILE *f)
{
	return (fgetc(f))+(fgetc(f)<<8)+(fgetc(f)<<16);
}
/******************************************************************************/
U32 fgetl(FILE *f)
{
	return (fgetc(f))+(fgetc(f)<<8)+(fgetc(f)<<16)+(fgetc(f)<<24);
}
/******************************************************************************/
void fputw(U16 l, FILE *f)
{
	fputc(l&0xFF,f);
	fputc(l>>8,f);
}
/******************************************************************************/
void fputt(U32 l, FILE *f)
{
	fputc(l&0xFF,f);
	fputc((l>>8)&0xFF,f);
	fputc((l>>16)&0xFF,f);
}
/******************************************************************************/
void fputl(U32 l, FILE *f)
{
	fputc(l&0xFF,f);
	fputc((l>>8)&0xFF,f);
	fputc((l>>16)&0xFF,f);
	fputc((l>>24)&0xFF,f);
}
/******************************************************************************/
U16 bGetW(const U8 *p)
{
	return (*p)+(p[1]<<8);
}
/******************************************************************************/
U16 beGetW(const U8 *p)
{
	return (p[1])+(p[0]<<8);
}
/******************************************************************************/
// scan the directory file(s) for the locations of the resources
BOOL LoadDir(BOOL SINGLE, int num)
{
	FILE *f;
	U32 offs;
	int totalEnts, i;
	if(SINGLE) {
		sprintf(fname,"%s%s%s",gi->path,gi->vID,gi->vID[0]?"dir":"dirs");
		if((f=fopen(fname,"rb"))==NULL) {
			ErrorMessage("Unable to open file: %s, for reading!",fname);
			return FALSE;
		}
		fseek(f,num<<1,SEEK_SET);
		offs = fgetw(f);
		if(num==3) {
			fseek(f,0,SEEK_END);
			totalEnts = (ftell(f)-offs)/3;
		} else
			totalEnts = (fgetw(f)-offs)/3;
		fseek(f,offs,SEEK_SET);
	} else {
		sprintf(fname,"%s%s",gi->path,dirNames[num]);
		if((f=fopen(fname,"rb"))==NULL) {
			ErrorMessage("Unable to open file: %s, for reading!",fname);
			return FALSE;
		}
		fseek(f,0,SEEK_END);
		totalEnts = ftell(f)/3;
		fseek(f,0,SEEK_SET);
	}
    if(totalEnts>256) totalEnts=0;
	memset(dirs[num],-1,sizeof(dirs[num]));
	for(i=0;i<totalEnts;i++) {
		if((offs = (fgetc(f)<<16)|(fgetc(f)<<8)|fgetc(f))!=0xFFFFFF) {
			dirs[num][i].offset		= offs&0x0FFFFF;
			dirs[num][i].vol		= (offs>>20)&0xF;
			VUSED[dirs[num][i].vol]	= TRUE;
		}
	}

	fclose(f);
	return TRUE;
}
/******************************************************************************/
// scan the vol files to find out how much space to allocate for all the files
BOOL PrepDirs()
{
	int vn,t,i;
	FILE *f;

	volSize=0;
	for(vn=0;vn<16;vn++) {
    	if(!VUSED[vn]) continue;
		sprintf(fname,"%s%svol.%d",gi->path,gi->vID,vn);
		if((f=fopen(fname,"rb"))==NULL) {
        	VUSED[vn] = FALSE;
        	continue;
			//ErrorMessage("Unable to open file: \"%s\", for reading!",fname);
			//return FALSE;
		}
		for(t=0;t<4;t++) {
			for(i=0;i<256;i++) {
				if(dirs[t][i].vol == vn) {
					fseek(f,dirs[t][i].offset,SEEK_SET);
                    if((fgetw(f)==0x3412)&&((fgetc(f)&0x7F)==vn)) {
						dirs[t][i].length = fgetw(f)+5;
						volSize += dirs[t][i].length;
                    } else {
                    	memset(&dirs[t][i],-1,sizeof(DIRENT));
                    }
				}
			}
		}
		fclose(f);
	}
	return TRUE;
}
/******************************************************************************/
// scan the vol files to find out how much space to allocate for all the files
BOOL ProcessDirs()
{
	int vn, vi, t,i,msgTotal;
	FILE *f;
	U8 *pvol,*p;
	U16 enclen,declen;
	U8 *fbuf;
    char *msg;

	if(!volSize) return FALSE;
	if((volData = (U8*)malloc(volSize))==NULL)
		return FALSE;
	if((fbuf = (U8*)malloc(65535))==NULL)
		return FALSE;
	
	pvol = volData;
	for(vn=0;vn<16;vn++) {
    	if(!VUSED[vn]) continue;
		sprintf(fname,"%s%svol.%d",gi->path,gi->vID,vn);
		if((f=fopen(fname,"rb"))==NULL) {
			ErrorMessage("Unable to open file: %s, for reading!",fname);
			mFree(fbuf);
			return FALSE;	
		}
		for(t=0;t<4;t++) {
			for(i=0;i<256;i++) {
				if(dirs[t][i].vol == vn) {
					fseek(f,dirs[t][i].offset+2,SEEK_SET);

					dirs[t][i].offset = (intptr_t)((intptr_t)pvol-(intptr_t)volData);
					vi 		= fgetc(f);
					declen 	= fgetw(f);
					enclen 	= (gi->version->flags&PACKED_DIRS)?fgetw(f):declen;

                    *pvol++ = 0x34;
                    *pvol++ = 0x12;
                    *pvol++ = vi;
                    *pvol++ = declen&0xFF;
                    *pvol++ = declen>>8;

					fread(fbuf,enclen,1,f);
					if(enclen==declen) { // not compressed
		               	if(t==0) { // logic
		            		msg = (char*)(fbuf+bGetW(fbuf)+2);	// pointer to messages
				    		msgTotal = (U8)*msg++;			// number of messages
							if(msgTotal)
								DecryptBlock(msg + ((msgTotal + 1)<<1),msg+bGetW((U8*)msg));
		                }
						memcpy(pvol,fbuf,declen);
					} else if(vi&0x80) { // compressed picture file
           				PIC_expand(fbuf, pvol, declen);
					} else { // compressed LZW
           				LZW_expand(fbuf, pvol, declen);
					}
					pvol += declen;
				}
			}
		}
		fclose(f);
	}
	mFree(fbuf);
	return TRUE;
}
/******************************************************************************/
BOOL ProcessObject()
{
	FILE *f;
    U16 l;
    int q, nameStart;
    int objCount, i, entSize;
    U8 *p, *iPtr, *wPtr, *u;

	sprintf(fname,"%sobject",gi->path);
	if((f=fopen(fname,"rb")) == NULL) {
		ErrorMessage("Unable to open file: \"%s\" for reading!", fname);
		return FALSE;
	}
	fseek(f,0,SEEK_END);
	l=ftell(f);
	fseek(f,0,SEEK_SET);

	p = (U8*) malloc(l);
	fread(p, l, 1, f);
	fclose(f);

	q=bGetW(p);
	if( q>l || q<3 )// || (gi->version->flags & ENCRYPT_OBJ))
		DecryptBlock((char*)p,(char*)p+l);

    f=fopen("object.bin","wb");
    fwrite(p,l,1,f);
    fclose(f);

    nameStart 	= bGetW(p);
    entSize 	= (gi->version->flags&AMIGA)?4:3;
    objCount	= nameStart/entSize;
    iPtr		= p+entSize;
    strSize		= l-nameStart;
	if((objNameData = (char*) malloc(strSize))==NULL) {
     	ErrorMessage("Unable to allocate %d bytes of memory!", strSize);
     	return FALSE;
    }
    memcpy(objNameData,iPtr+nameStart, strSize);

    u=objRoomsStart;
    for(i=0;i<objCount;i++) {
        objNames[i] = objNameData+(bGetW(iPtr)-nameStart);
    	*u++ = iPtr[2];
     	iPtr+=entSize;
    }

	free(p);

    return TRUE;
}
/******************************************************************************/
U8 *LoadFile(BOOL G_PATH, const char *name, int *len)
{
	FILE *f;
	int l;
    U8 *p;

	if(G_PATH)
    	sprintf(fname,"%s%s",gi->path,name);
    else
    	strcpy(fname,name);
	if((f=fopen(fname,"rb")) == NULL) {
		ErrorMessage("Unable to open file: \"%s\" for reading!", fname);
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
int FindWordx(const char *s,U8 *b)
{
	const char *s1,*s2;
	while(*b) {
    	s1 = (char*)b+3;
        s2 = s;
        if(strcmp(s1,s2)==0)
        	return (b[1]+(b[2]<<8));
        b+=b[0]+1;
    }
    return -1;
}   
/******************************************************************************/
void AddWord(int group, const char *string)
{
	pwords->group = group;
	pwords->string = string;
	pwords++;
}
/******************************************************************************/
const char *FindWord(int group)
{
	int a;
    U8 *p;
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if((p[1]+(p[2]<<8))==group)
            	return (char*)p+3;
            p+=*p;
        }
    }
    return "^";
}
/******************************************************************************/
int FindTotalWordsInGroup(int group)
{
	int a;
    int groupcount = 0;
    U8 *p;
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if((p[1]+(p[2]<<8))==group)
            	groupcount++;
            p+=*p;
        }
    }
    return groupcount;
}
/******************************************************************************/
const char **FindExtraWordInGroup(int group)
{
	int a;
    U8 *p;
    const char **slist;

    int maxgroups = FindTotalWordsInGroup(group);
    int gcnt = 0;
    if(maxgroups<2) return NULL;

    slist = (const char **)malloc(sizeof(const char **)*(maxgroups+1));

	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if((p[1]+(p[2]<<8))==group) {
                if(gcnt) {
                 	slist[gcnt-1] = ((char*)p+3);
                }
             	gcnt++;
            }
            p+=*p;
        }
    }
    slist[gcnt-1] = NULL;
    return slist;
}
/******************************************************************************/
const char *FindWord2(int group)
{
	WORDSET *w;
	w = wordset;
    while(w->group) {
     	if(w->group==group)
        	return(w->string);
        w++;
    }
    return "^";
}
/******************************************************************************/
int FindWordStr(const char *s)
{
	int group, a;
    U8 *p;
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if(strcmp(s,(char*)(p+3))==0) {
            	group = (p[1]+(p[2]<<8))&0x1FFF;
                return(group);
            }
            p+=*p;
        }
    }
    return 0;
}
/******************************************************************************/
void ClearGroup(int group)
{
	U8 *p;
    int a;
	if(group)
	for(a=0;a<26;a++) {
		p=(U8*)words[a];
        if(!p) continue;
        while(*p!=0) {
         	if(((p[1]+(p[2]<<8))&0x1FFF)==group) {
            	p[1]=0;
                p[2]=0;
            }
            p+=*p;
        }
    }
}
/******************************************************************************/
void DoSolidGroups()
{
	int set=0,idx,groups[5];
    const char *s;

    while(solidGroups[set][0][0]) {
    	idx=0;
    	while((s=solidGroups[set][idx])[0]) {
    		if((groups[idx] = FindWordStr(s))!=0) {
            	AddWord(groups[idx],s);
        	}
        	idx++;
    	}
        while(idx)
        	ClearGroup(groups[--idx]);
        set++;
    }
}
/******************************************************************************/
void DoSolidWords()
{
	int idx=0,group;
    const char *s;

    while((s=solidWords[idx])[0]) {
    	if((group = FindWordStr(s))!=0) {
        	AddWord(group,s);
            ClearGroup(group);
        }
        idx++;
    }
}
/******************************************************************************/
void DoRemainingWords()
{
	const char *s;
    int i;
	for(i=1;i<9999;i++) {
		if((s=FindWord(i))[0]!='^') {
        	AddWord(i,s);
        }
	}
	for(i=1;i<9999;i++) {
    	const char **slist = FindExtraWordInGroup(i);
        if(slist) {
        	const char **sl = slist;
        	if(*sl) {
        		AddWord(i,*sl);
        	    sl++;
        	}
        	free(slist);
        }
	}
}
/******************************************************************************/
void AlphaSortWords()
{
	WORDSET *w1 = wordset, *w0, wt;
            /*
    int q=0;
    Form1->ListBox2->Items->Clear();
    while(w1->group) {
    	q++;
    	Form1->ListBox2->Items->Add(AnsiString(w1->string));
		w1++;
    }        */
    w1 = wordset;
    w0 = w1;   
    if(!w1) return;
    w1++;        
    if(!w1) return;
    while(w1->group) {
        if(strcmp(w1->string,w0->string)<0) {
       		memcpy(&wt, w0,sizeof(WORDSET));
       		memcpy( w0, w1,sizeof(WORDSET));
       		memcpy( w1,&wt,sizeof(WORDSET));
    		w1 = wordset;
        }
		w0 = w1;
        w1++;
    }
    w1 = wordset;
    while(w1->group) {
        w1++;
    }         /*
    w1 = wordset;
    while(w1->group) {
    	Form1->ListBox3->Items->Add(AnsiString(w1->string));
		w1++;
    }   */
}
/******************************************************************************/
BOOL ProcessWords()
{
	FILE *f;
    U16 l, wc;
    int strSize, objCount, i, offs,g;
    U8 *msg,*tokData,*wPtr;
	WORDSET *w;

	if((vocabData = LoadFile(FALSE, "vocab.bin", NULL))==NULL)
    	return FALSE;
	if((tokData = LoadFile(TRUE, "words.tok", NULL))==NULL)
    	return FALSE;

    wordData = (U8*)malloc(64000);
    memset(words,0,sizeof(words));
    wPtr = wordData;
    wc=0;
    for(i=0;i<26;i++) {
     	offs = 0;
        if(bGetW(tokData+(i<<1))==0) continue;
        msg = tokData + ((tokData[i<<1]<<8)|(tokData[(i<<1)+1]));
        words[i] = (const char *)wPtr;
        do {
        	offs = *msg++;
        	do
        		wordbuf[offs++] = (*msg&0x7F)^0x7F;
        	while(*msg++<0x80);
       		wordbuf[offs++] = 0;
            g=beGetW(msg);
            if(g!=1&&g!=9999) {
            	l=strlen(wordbuf)+1;
               	wPtr[0] = l+3;
            	wPtr[1] = g&0xFF;
            	wPtr[2] = g>>8;
            	memcpy(wPtr+3,wordbuf,l);
            	wPtr+=l+3;
            	wc++;
            }
            msg+=2;
        } while(msg[0]);
        *wPtr++=0;
            	wc++;
    }
    wordsSize = wPtr-wordData;
	mFree(tokData);

    if(!wc) return TRUE;
	wordset = (WORDSET*)calloc(sizeof(WORDSET),wc);
 	pwords = wordset;
	DoSolidGroups();
    DoSolidWords();
    DoRemainingWords();
    //if(pwords!=wordset)
    	AlphaSortWords();
                     /*
    f=fopen("words.txt","w");

	w = wordset;
    while(w->group) {
     	fprintf(f,"%05d:%s\n",w->group,w->string);
        w++;
    }
    fclose(f);  */

    return TRUE;
}
/******************************************************************************/
int sfcount;
int curlogsX[256], *curlogs;
U8 vVals[256][8];
void DumpLog(int num)
{
	U8 *xcode=code,*log,*end,*flags,*logDir;
    int pc,op,g,a,i;
    char *p;

	if(LOG_DONE[num]) return;
    LOG_DONE[num]=TRUE;

	if(dirs[0][num].vol==-1) return;
	if((logDir = &volData[dirs[0][num].offset]) == NULL) return;

	log=logDir+5;
    end=logDir+7+(logDir[5]+(logDir[6]<<8));

    code=log+2;

    if(lwPtrs[num]&&curlogs==curlogsX) return;
    sfcount = 0;
    *curlogs++ = num;

    while(code<end) {
        op = *code++;
     	if(op==0xFF) {
        	ExecuteIF();
        } else {
        	switch(op) {
             	case 3: // assignn
            		for(i=0;i<8;i++) {
                    	if(vVals[*code][i]==code[1])
                        	break;
                		if(!vVals[*code][i]) {
                        	vVals[*code][i] = code[1];
                            break;
                        }
                    }
                    break;
             	case 22:
             	 	DumpLog(*code);
                    break;
                case 23:
            		for(i=0;i<8;i++) {
                		if(!vVals[*code][i]) break;
             			 DumpLog(vVals[*code][i]);
                	}
                    break;
            }
            code += agiCommands[op].nParams;
        }
    }

    *curlogs--=-1;
	code=xcode;
}
/******************************************************************************/
void ExecuteIF()
{
	int op;
    int saidCnt,g;
    int *m;
    int n;
	for(;;) {
    	op = *code++;
        if(op==0xFF) break;
    	if(op<0xFC) {
        	if(op==0xE) {
             	saidCnt=*code++;
               	//fprintf(lf,"( ");
                while(saidCnt) {
                	g = (code[0]+(code[1]<<8));
                    if(g&&g!=9999) {
                    	for(m=curlogsX;m<curlogs;m++) {
                        	n=*m;
                        	if(n<0||n>255) {
                            	m=m;
                            	ErrorMessage("M");
                            }
                            if(!lwPtrs[n])
                            	lwPtrs[n] = (U8*)calloc(lwfsize,1);
                        	if(!lwPtrs[n])
                            	lwPtrs[n]=lwPtrs[n];
                            if((g>=4096)||((g>>3)>=lwfsize)) {
                            	g=g;
                            	//ErrorMessage("G");
                            } else {
                            	lwPtrs[n][g>>2]|=((*curlogsX==0||m==curlogsX)?1:2)<<((g&3)<<1);
                            }
                        }
                    }
                    	//fprintf(lf,"\"%s\" ",FindWord2(g));
                    code+=2;
                    saidCnt--;
                }
               	//fprintf(lf,")\n");
                sfcount++;
            } else code += testCommands[op].nParams;
		}
	}
	code+=2;
    //fprintf(lf,"-\n");
}
/******************************************************************************/
U32 ptrs[14];
U32 wm[256];
BOOL OutputGame()
{
	int t,i,j,len,q,a,g;
    U32 fstart;
	WORDSET *w,*w1;
    char ccc;
    int wSize;
    int maxgrp, ps, nWc;
    char *wBuf=NULL,*wPtr;

    memset(ptrs,0,sizeof(ptrs));

    ptrs[1] = offs;
    fwrite(volData,volSize,1,fout);
	offs += volSize;
    ALIGN(fout);

    for(t=0;t<4;t++) {
    	ptrs[2+t] = offs;
    	for(i=0;i<256;i++) {
    		fputl(
        		(dirs[t][i].vol==-1)?0:ptrs[1]+dirs[t][i].offset,
                fout
            );
    	}
    	offs += 1024;
    }
    ALIGN(fout);

    ptrs[6] = offs;
    fwrite(objNameData,strSize,1,fout);
	offs += strSize;
    ALIGN(fout);

    ptrs[7] = offs;
    for(i=0;i<256;i++) {
        fputl(
          	(objNames[i])?
            	(ptrs[6]+(intptr_t)((intptr_t)objNames[i]-(intptr_t)objNameData)):0,
            fout
        );
    }
    offs += 1024;

    ptrs[8] = offs;
    fwrite(objRoomsStart,256,1,fout);
	offs += 256;

    maxgrp=0;
    wSize = 1;
	w = wordset;
    if(w) {
    if(w->group) {
    	ccc = w->string[0];
    	while(w->group) {
        	if(w->string[0]!=ccc) {
        		wSize++;
        	}
    		wSize += strlen(w->string)+1;
        	wSize += 3;
        	w++;
    	}
    }
    wBuf = (char*)calloc(wSize,1);
    wPtr = wBuf;
    w = wordset;
    if(w->group) {
    ccc = w->string[0];
    ps=offs;
    nWc=0;
    while(w->group) {
        g = w->group;
        if(!(g&0x8000)) {
        	if(g>maxgrp)
        		maxgrp = g;
        	q=FindWordx(w->string,vocabData);
        	if((q!=-1)&&(q&0x80)) {
    			w1 = wordset;
   				while(w1->group) {
        			if(w1->group==g)  {
        				w1->group|=0x8000;
                    }
        			w1++;
        		}
        	}
        }
        w++;
	}
    w = wordset;
    while(w->group) {
        if(w->string[0]!=ccc) {
        	ccc = w->string[0];
        	*wPtr++ = 0;
            ps++;
        }
        w->addr = ps;
    	len = strlen(w->string)+1;

        *wPtr++ = (len+3);
        //q=FindWordx(w->string,vocabData);

        *wPtr++ = (w->group&0xFF);
        *wPtr++ = (w->group>>8);

    	memcpy(wPtr,w->string,len);
        w->string = wPtr;
        wPtr += len;
        ps += len+3;
        nWc++;
        w++;
    } }
    *wPtr++ = 0;
    maxgrp++;
	}

    ptrs[9] = offs;
    if(wBuf) {
    	fwrite(wBuf,wSize,1,fout);
		offs += wSize;
    } else {
     	fputl(0,fout);
        offs+=4;
    }
    ALIGN(fout);


    ptrs[10] = offs;
    if(wBuf) {

    ccc='a';
    memset(words,0,sizeof(words));
    for(a=0;a<26;a++) {
		w = wordset;
    	while(w->group&0x1FFF) {
    		if(w->string[0]==ccc) {
        		words[a] = w->string-3;
                break;
            }
        	w++;
    	}
        ccc++;
    }

    	for(i=0;i<26;i++) {
        	q = (words[i])?
        		(ptrs[9]+(intptr_t)((intptr_t)words[i]-(intptr_t)wBuf)):0;
        	fwrite(&q,4,1,fout);
    	}
    } else {
     	for(i=0;i<26;i++) {
         	fputl(0,fout);
        }
    }
    offs += 26*4;
    ALIGN(fout);

    mFree(wBuf);



    lwfsize = (maxgrp>>2)+1;

    memset(vVals,0,sizeof(vVals));
    if(lwfsize)
    	for(i=0;i<256;i++) {
    		memset(LOG_DONE,0,sizeof(LOG_DONE));
   			curlogs = curlogsX;
    		code = NULL;
    		DumpLog(i);
            if(i&&lwPtrs[i]&&lwPtrs[0]) {
             	for(j=0;j<lwfsize;j++) {
                 	lwPtrs[i][j]|=lwPtrs[0][j]<<1;
                }
            }
    	}

    ptrs[11] = offs;
    for(i=0;i<256;i++) {
        if(lwPtrs[i]) {
        	wm[i] = offs;
        	fwrite(lwPtrs[i],lwfsize,1,fout);
            offs += lwfsize;
        } else
        	wm[i] = 0;
    }
    ALIGN(fout);

    ptrs[12] = offs;
    fwrite(wm,sizeof(wm),1,fout);
    offs += sizeof(wm);

    ptrs[13] = maxgrp;

    fstart = ftell(fout);

    fseek(fout,giPos,SEEK_SET);
    giPos += 80;

  	fwrite("AGi",3,1,fout);
    fputc(gi->version->major,fout);
    fputw(gi->version->minor,fout);
  	fwrite(gi->title,strlen(gi->title)+1,1,fout);
    for(i=strlen(gi->title)+1;i<26;i++)
    	fputc(0,fout);

    fwrite(ptrs+1,sizeof(ptrs)-8,1,fout);
    fseek(fout,fstart,SEEK_SET);   

   	return TRUE;
}
/******************************************************************************/
BOOL ProcessGame(GAMEINFO *gmInfo)
{
	int i;
    FILE *f;

	memset(VUSED,0,sizeof(VUSED));
	volData = NULL;
    memset(objNames,0,sizeof(objNames));
    memset(lwPtrs,0,sizeof(lwPtrs));
    objNameData = NULL;
    vocabData = NULL;
    wordData = NULL;
    wordset = NULL;

    gi = gmInfo;

	for(i=0;i<4;i++) {
		if(!LoadDir(gi->version->flags&SINGLE_DIR, i))
			return FALSE;
	}
	if(!PrepDirs())
		return FALSE;
	if(!ProcessDirs())
		return FALSE;
	if(!ProcessObject())
		return FALSE;
	if(!ProcessWords())
		return FALSE;

    if(!OutputGame())
    	return FALSE;
                         /*
    f=fopen("vols.bin","wb");
    fwrite(volData,volSize,1,f);
    fclose(f);     */
		
	return TRUE;
}
/******************************************************************************/
void FreeGame()
{
	int i;

	mFree(volData);
	mFree(objNameData);
    mFree(vocabData);
    mFree(wordData);
    mFree(wordset);
 
    for(i=0;i<256;i++) {
    	mFree(lwPtrs[i]);
    }
}
/******************************************************************************/