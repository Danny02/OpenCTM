/* NameMangle.h -- Name mangling to avoid linking conflicts
2009-04-15 : Marcus Geelnard : Public domain */

#ifndef __7Z_NAMEMANGLE_H
#define __7Z_NAMEMANGLE_H

#ifdef LZMA_FOR_CTM
#define LzmaCompress _ctmLzmaCompress
#define LzmaUncompress _ctmLzmaUncompress

#define LzmaEncProps_Init _ctmLzmaEncProps_Init
#define LzmaEncode _ctmLzmaEncode
#define LzmaDecode _ctmLzmaDecode
#define MatchFinder_Construct _ctmMatchFinder_Construct
#define MatchFinder_Create _ctmMatchFinder_Create
#define MatchFinder_Free _ctmMatchFinder_Free
#define MatchFinder_CreateVTable _ctmMatchFinder_CreateVTable
#endif /* LZMA_FOR_CTM */

#endif /* __7Z_NAMEMANGLE_H */
