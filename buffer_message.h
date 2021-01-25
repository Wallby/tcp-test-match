/*
Copyright (c) 2020 MTJH ten Velden

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

// NOTE: this file is to be replicated on the loader side, hence it needs to comply to conventions of the C language

#ifndef PTPF_BUFFERMESSAGE_H
#define PTPF_BUFFERMESSAGE_H
#include "tcp_mini.h"

enum
{
	EMessageType_Buffer = 1
};

/*
 * reserved enum indices..
 * EBufferFormat_None = 0
 * EBufferFormat_Ints = 2
 * EBufferFormat_Floats = 3
 * EBufferFormat_Chars = 4
 */
enum EBufferFormat
{
	EBufferFormat_None = 0 //< for yet uninterpreted buffers
};

struct buffer_message_t //< .._message_t may not be sent, but can be used for interpretation
{
	int type;
	int format;
} buffer_message_default = {EMessageType_Buffer};

struct unformattedbuffer_t //< .._t must have a size as the last argument, which described the size of the "buffer"
{
	int type;
	int format;
	int size;
} unformattedbuffer_default = {EMessageType_Buffer, EBufferFormat_None};
#endif

