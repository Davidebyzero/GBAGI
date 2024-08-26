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
#ifndef _DECOMPRESS_H
#define _DECOMPRESS_H
/******************************************************************************/

void initLZW(void);
void closeLZW(void);
S32 setBITS(S32 value);
U8 *decode_string(U8 *buffer, U32 code);
U32 input_code(U8 **input);
BOOL LZW_expand(U8 *in, U8 *out, S32 len);

void PIC_expand(U8 *in, U8 *out, S32 len);

void DecryptBlock(char *start, char *end);
/******************************************************************************/
#endif
/******************************************************************************/
