//#include "HDRLoader.h"
//
//#include <stdio.h>
//
///* Size of the buffer read in to identify texture and model formats */
//#define FILEIO_BUF_SIZE 20
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//	/* extdata is to be used as flags or other data to be passed to the function */
//	typedef void* (*Texreader)(const char *filename, FILE *fp, int *w, int *h,
//		GLenum *dataformat, GLenum *datatype, long extdata);
//	/* returns true if this is the proper imagefile type */
//	typedef int(*Texidentifier)(const char *filename, void *buf, long extdata);
//
//	typedef struct Texreaderfmt {
//		Texreader           read;
//		Texidentifier       ident;
//		long                extdata;
//	} Texreaderfmt;
//
//	extern int          curtexname;
//	extern const char **texnames;
//
//	void    iv_fileio_init(void);
//
//	void    iv_fileio_register_texfmt(const char *name, Texreader func,
//		Texidentifier ident, long extdata);
//	void*   iv_fileio_read_texfile(const char *filename, int *w, int *h,
//		GLenum *dataformat, GLenum *datatype);
//
//#ifdef __cplusplus
//}
//#endif
//
///****************************************************************************\
//
//fileio.c: File I/O interface for Anyview
//by Cliff Woolley and Ben Cummings
//
//\****************************************************************************/
//
//#include <assert.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <GL/glut.h>
//#include <gsh.h>
//
//#include "fileio.h"
//
//#include "atf.h"
//#include "bmp.h"
//#include "pfm.h"
//#include "ppm.h"
//#include "exr.h"
//#include "hdr.h"
//#include "png.h"
//#include "tga.h"
//
///* globals */
//
//const char    **texnames;
//int             curtexname = 0;
//int             numtexnames = 0;
//
//Texreaderfmt   *texfmts;
//int             curtexfmt = 0;
//int             numtexfmts = 0;
//
//GLuint         *texhandles;
//int             curtexhandle = 0;
//int             numtexhandles = 0;
//
///* macros */
//#define ADD_TO_LIST(item, list, listsize, listfullness)         \
//    {                                                           \
//        void *voidptr;                                          \
//        if (listfullness >= listsize) {                         \
//            if (listsize == 0) {                                \
//                listsize = 2;                                   \
//            }                                                   \
//            else {                                              \
//                listsize *= 2;                                  \
//            }                                                   \
//            voidptr = realloc((void *)list,                     \
//                              (sizeof(item) * listsize));       \
//            if (voidptr == NULL) {                              \
//                free((void *)list);                             \
//                fprintf(stderr, "Error: out of memory\n");      \
//                assert(0);                                      \
//                exit(1);                                        \
//            }                                                   \
//            list = voidptr;                                     \
//        }                                                       \
//        list[listfullness++] = item;                            \
//    }
//
///* functions */
//
//void iv_fileio_init(void)
//{
//	iv_pfm_init();
//	iv_atf_init();
//	iv_bmp_init();
//	iv_ppm_init();
//	iv_exr_init();
//	iv_hdr_init();
//#ifndef __APPLE__
//	iv_png_init();
//	iv_tga_init();
//#endif
//}
//
//void iv_fileio_register_texfmt(const char *name, Texreader func,
//	Texidentifier ident, long extdata)
//{
//	Texreaderfmt readfmt;
//
//	assert(func);
//	assert(ident);
//
//	readfmt.read = func;
//	readfmt.ident = ident;
//	readfmt.extdata = extdata;
//	ADD_TO_LIST(readfmt, texfmts, numtexfmts, curtexfmt);
//	ADD_TO_LIST(name, texnames, numtexnames, curtexname);
//
//	return;
//}
//
//void* iv_fileio_read_texfile(const char *filename, int *w, int *h,
//	GLenum *dataformat, GLenum *datatype)
//{
//	int i;
//	FILE *fp = fopen(filename, "rb");
//	char buf[FILEIO_BUF_SIZE];
//	void *texmap = NULL;
//
//	if (fp == NULL) {
//		gsh_Printf("Unable to open file \"%s\".\n", filename);
//		return NULL;
//	}
//
//	fgets(buf, sizeof(buf), fp);
//	rewind(fp);
//
//	for (i = 0; i<curtexfmt; ++i) {
//		if (texfmts[i].ident(filename, buf, texfmts[i].extdata)) {
//			texmap = texfmts[i].read(filename, fp, w, h, dataformat, datatype,
//				texfmts[i].extdata);
//			break;
//		}
//	}
//
//	if (i == curtexfmt) {
//		gsh_Printf("Error: cannot open %s: cannot read the format\n",
//			filename);
//		assert(texmap == NULL);
//	}
//	else {
//		assert(texmap != NULL);
//	}
//
//	fclose(fp);
//	return texmap;
//}
//
//
//
///****************************************************************************\
//
//hdr.c: RADIANCE .HDR file parser for anyview
//extracted, adapted, and de-C++-ified by Cliff Woolley
//from R.I.S.E version 0.9.3 build 67 by Aravind Krishnaswamy:
//
////  HDRReader.cpp - Implementation of the HDRReader class
////
////  Author: Aravind Krishnaswamy
////  Date of Birth: November 17, 2001
////
////This license is based off the original BSD license.
//
////Copyright (c) 2001-2004, Aravind Krishnaswamy
////All rights reserved.
//
////Redistribution and use in source and binary forms,
////with or without modification, are permitted provided
////that the following conditions are met:
//
////Redistributions of source code must retain the above copyright notice,
////this list of conditions and the following disclaimer.
//
////Redistributions in binary form must reproduce the above copyright notice,
////this list of conditions and the following disclaimer in the documentation
////and/or other materials provided with the distribution.
//
////THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
////AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
////IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
////ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
////LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
////CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
////SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
////INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
////CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
////ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
////THE POSSIBILITY OF SUCH DAMAGE.
//
//\****************************************************************************/
//
//
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <GL/gl.h>
//
//static const char *szSignature = "#?RADIANCE";
//static const char *szFormat = "FORMAT=32-bit_rle_rgbe";
//
//void *read_hdr(const char *filename, FILE *fp, int *w, int *h,
//	GLenum *dataformat, GLenum *datatype, long extdata);
//int ident_hdr(const char *filename, void *buf, long extdata);
//
//void iv_hdr_init(void)
//{
//	iv_fileio_register_texfmt("HDR", read_hdr, ident_hdr, 0);
//}
//
//#include <math.h>
//#ifdef WIN32
///* sigh. */
//_CRTIMP float  __cdecl powf(float, float);
//#endif
//
//#define ldexp(m,e) ((m) * powf(2.0f,(e)))
//
///* HDR Function Definitions */
//void *read_hdr(const char *filename, FILE *fp, int *w, int *h,
//	GLenum *dataformat, GLenum *datatype, long extdata)
//{
//	char buf[1024] = { 0 };
//	char col[4] = { 0 };
//	unsigned char *imagergbe;
//	float *image;
//	int bFlippedX = 0;
//	int bFlippedY = 0;
//	int cnt, x, y, i, component;
//
//	/* Verify the RADIANCE signature */
//	fseek(fp, 0, SEEK_SET);
//	fread(buf, 1, sizeof(szSignature) - 1, fp);
//
//	if (strncmp(szSignature, buf, sizeof(szSignature) - 1) != 0) {
//		fprintf(stderr, "read_hdr(): RADIANCE signature not found!\n");
//		return NULL;
//	}
//
//	/* Next, skip past comments until we reach the portion that
//	* tells us the dimensions of the image */
//	/* Check to see if each line contains the format string */
//	do {
//		fgets(buf, sizeof(buf), fp);
//	} while (!feof(fp) && strncmp(buf, "FORMAT", 6));
//
//	if (feof(fp)) {
//		fprintf(stderr, "read_hdr(): unexpected EOF looking for FORMAT string\n");
//		return NULL;
//	}
//
//	/* Check if the format string is ok */
//	if (strncmp(buf, szFormat, sizeof(szFormat) - 1) != 0) {
//		fprintf(stderr, "read_hdr(): The FORMAT is not %s\n", szFormat);
//		return NULL;
//	}
//
//	/* Now look for the -Y or +Y */
//	/* Check to see if each line contains the size string */
//	do {
//		fgets(buf, sizeof(buf), fp);
//	} while (!feof(fp) &&
//		(strncmp(buf, "-Y", 2) && strncmp(buf, "+Y", 2)));
//
//	if (feof(fp)) {
//		fprintf(stderr, "read_hdr(): unexpected EOF looking for image dimensions\n");
//		return NULL;
//	}
//
//	if (strncmp(buf, "-Y", 2) == 0) {
//		bFlippedY = 1;
//	}
//
//	/* Find the X */
//	cnt = 0;
//	while ((cnt < sizeof(buf)) && buf[cnt] != 'X') {
//		cnt++;
//	}
//
//	if (cnt == sizeof(buf)) {
//		fprintf(stderr, "read_hdr(): error reading image dimensions\n");
//		return NULL;
//	}
//
//	if (buf[cnt - 1] == '-') {
//		/* Flip the X */
//		bFlippedX = 1;
//	}
//
//	/* Read the dimensions */
//	sscanf(buf, "%*s %u %*s %u", h, w);
//
//	if ((*w == 0) || (*h == 0)) {
//		fprintf(stderr, "read_hdr(): Something is wrong with image dimensions\n");
//		return NULL;
//	}
//
//	imagergbe = (unsigned char *)calloc((*w) * (*h) * 4, sizeof(unsigned char));
//
//	/* Do the RLE compression stuff
//	* Some of the RLE decoding stuff comes from ggLibrary */
//	for (y = 0; y < (*h); y++) {
//		int start = bFlippedY ? ((*h) - y - 1)*(*w) : y*(*w);
//		int step = bFlippedX ? -1 : 1;
//
//		/* Start by reading the first four bytes of every scanline */
//		if (fread(col, 1, 4, fp) != 4) {
//			fprintf(stderr, "read_hdr(): unexpected EOF reading data\n");
//			free(imagergbe);
//			return NULL;
//		}
//
//		/* This is Radiance's new RLE scheme
//		* Header of 0x2, 0x2 is RLE encoding */
//		if (col[0] == 2 && col[1] == 2 && col[2] >= 0) {
//
//			/* Each component is run length encoded seperately
//			* This will naturally lead to better compression */
//			for (component = 0; component < 4; component++) {
//				int pos = start;
//
//				/* Keep going until the end of the scanline */
//				x = 0;
//				while (x < (*w)) {
//					/* Check to see if we have a run */
//					unsigned char num;
//					if (fread(&num, 1, 1, fp) != 1) {
//						fprintf(stderr, "read_hdr(): unexpected EOF reading data\n");
//						free(imagergbe);
//						return NULL;
//					}
//					if (num <= 128) {
//						/* No run, just values, just just read all the values */
//						for (i = 0; i<num; i++) {
//							if (fread(&imagergbe[component + pos * 4], 1, 1, fp) != 1) {
//								fprintf(stderr, "read_hdr(): unexpected EOF reading data\n");
//								free(imagergbe);
//								return NULL;
//							}
//							pos += step;
//						}
//					}
//					else {
//						/* We have a run, so get the value and set all the
//						* values for this run */
//						unsigned char value;
//						if (fread(&value, 1, 1, fp) != 1) {
//							fprintf(stderr, "read_hdr(): unexpected EOF reading data\n");
//							free(imagergbe);
//							return NULL;
//						}
//						num -= 128;
//						for (i = 0; i<num; i++) {
//							imagergbe[component + pos * 4] = value;
//							pos += step;
//						}
//					}
//					x += num;
//				}
//			}
//		}
//		else {
//			/* This is the old Radiance RLE scheme. it's a bit simpler.
//			* All it contains is either runs or raw data, runs have
//			* their header, which we check for right away. */
//			int pos = start;
//			for (x = 0; x<(*w); x++) {
//				if (x > 0) {
//					if (fread(col, 1, 4, fp) != 4) {
//						fprintf(stderr, "read_hdr(): unexpected EOF reading data\n");
//						free(imagergbe);
//						return NULL;
//					}
//				}
//
//				/* Check for the RLE header for this scanline */
//				if (col[0] == 1 && col[1] == 1 && col[2] == 1) {
//
//					/* Do the run */
//					int num = ((int)col[3]) & 0xFF;
//					unsigned char r = imagergbe[(pos - step) * 4];
//					unsigned char g = imagergbe[(pos - step) * 4 + 1];
//					unsigned char b = imagergbe[(pos - step) * 4 + 2];
//					unsigned char e = imagergbe[(pos - step) * 4 + 3];
//
//					for (i = 0; i<num; i++) {
//						imagergbe[pos * 4 + 0] = r;
//						imagergbe[pos * 4 + 1] = g;
//						imagergbe[pos * 4 + 2] = b;
//						imagergbe[pos * 4 + 3] = e;
//						pos += step;
//					}
//
//					x += num - 1;
//				}
//				else {
//					/* No runs here, so just read the data thats there */
//					imagergbe[pos * 4 + 0] = buf[0];
//					imagergbe[pos * 4 + 1] = buf[1];
//					imagergbe[pos * 4 + 2] = buf[2];
//					imagergbe[pos * 4 + 3] = buf[3];
//					pos += step;
//				}
//			}
//		}
//	}
//
//	/* convert rgbe to IEEE floats
//	* (ref "Real Pixels" by Greg Ward, Graphics Gems II) */
//	image = (float *)malloc((*w) * (*h) * 3 * sizeof(float));
//	for (i = 0; i<(*w)*(*h); i++) {
//		unsigned char exponent = imagergbe[i * 4 + 3];
//		if (exponent == 0) {
//			image[i * 3 + 0] = 0.0f;
//			image[i * 3 + 1] = 0.0f;
//			image[i * 3 + 2] = 0.0f;
//		}
//		else {
//			float v = ldexp(1.0f / 256.0f, (float)(exponent - 128));
//			image[i * 3 + 0] = (imagergbe[i * 4 + 0] + 0.5f) * v;
//			image[i * 3 + 1] = (imagergbe[i * 4 + 1] + 0.5f) * v;
//			image[i * 3 + 2] = (imagergbe[i * 4 + 2] + 0.5f) * v;
//		}
//	}
//
//	free(imagergbe);
//
//	*dataformat = GL_RGB;
//	*datatype = GL_FLOAT;
//
//	return image;
//}
//
//int ident_hdr(const char *filename, void *buf, long extdata)
//{
//	return (strncmp(szSignature, buf, sizeof(szSignature) - 1) == 0);
//}
//
