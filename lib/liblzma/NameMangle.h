/* NameMangle.h -- Name mangling to avoid linking conflicts
2010-09-13 : Marcus Geelnard : Public domain */

#ifndef __7Z_NAMEMANGLE_H
#define __7Z_NAMEMANGLE_H

#ifdef LZMA_PREFIX_CTM

/* Alloc.c */
#define MyAlloc _ctm_MyAlloc
#define MyFree _ctm_MyFree
#ifdef _WIN32
#define MidAlloc _ctm_MidAlloc
#define MidFree _ctm_MidFree
#define SetLargePageSize _ctm_SetLargePageSize
#define BigAlloc _ctm_BigAlloc
#define BigFree _ctm_BigFree
#endif /* _WIN32 */

/* LzFind.c */
#define MatchFinder_GetPointerToCurrentPos _ctm_MatchFinder_GetPointerToCurrentPos
#define MatchFinder_GetIndexByte _ctm_MatchFinder_GetIndexByte
#define MatchFinder_GetNumAvailableBytes _ctm_MatchFinder_GetNumAvailableBytes
#define MatchFinder_ReduceOffsets _ctm_MatchFinder_ReduceOffsets
#define MatchFinder_MoveBlock _ctm_MatchFinder_MoveBlock
#define MatchFinder_NeedMove _ctm_MatchFinder_NeedMove
#define MatchFinder_ReadIfRequired _ctm_MatchFinder_ReadIfRequired
#define MatchFinder_Construct _ctm_MatchFinder_Construct
#define MatchFinder_Free _ctm_MatchFinder_Free
#define MatchFinder_Create _ctm_MatchFinder_Create
#define MatchFinder_Init _ctm_MatchFinder_Init
#define MatchFinder_Normalize3 _ctm_MatchFinder_Normalize3
#define GetMatchesSpec1 _ctm_GetMatchesSpec1
#define Bt3Zip_MatchFinder_GetMatches _ctm_Bt3Zip_MatchFinder_GetMatches
#define Hc3Zip_MatchFinder_GetMatches _ctm_Hc3Zip_MatchFinder_GetMatches
#define Bt3Zip_MatchFinder_Skip _ctm_Bt3Zip_MatchFinder_Skip
#define Hc3Zip_MatchFinder_Skip _ctm_Hc3Zip_MatchFinder_Skip
#define MatchFinder_CreateVTable _ctm_MatchFinder_CreateVTable

/* LzmaDec.c */
#define LzmaDec_InitDicAndState _ctm_LzmaDec_InitDicAndState
#define LzmaDec_Init _ctm_LzmaDec_Init
#define LzmaDec_DecodeToDic _ctm_LzmaDec_DecodeToDic
#define LzmaDec_DecodeToBuf _ctm_LzmaDec_DecodeToBuf
#define LzmaDec_FreeProbs _ctm_LzmaDec_FreeProbs
#define LzmaDec_Free _ctm_LzmaDec_Free
#define LzmaProps_Decode _ctm_LzmaProps_Decode
#define LzmaDec_AllocateProbs _ctm_LzmaDec_AllocateProbs
#define LzmaDec_Allocate _ctm_LzmaDec_Allocate
#define LzmaDecode _ctm_LzmaDecode

/* LzmaEnc.c */
#define LzmaEncProps_Init _ctm_LzmaEncProps_Init
#define LzmaEncProps_Normalize _ctm_LzmaEncProps_Normalize
#define LzmaEncProps_GetDictSize _ctm_LzmaEncProps_GetDictSize
#define LzmaEnc_FastPosInit _ctm_LzmaEnc_FastPosInit
#define LzmaEnc_SaveState _ctm_LzmaEnc_SaveState
#define LzmaEnc_RestoreState _ctm_LzmaEnc_RestoreState
#define LzmaEnc_SetProps _ctm_LzmaEnc_SetProps
#define LzmaEnc_InitPriceTables _ctm_LzmaEnc_InitPriceTables
#define LzmaEnc_Construct _ctm_LzmaEnc_Construct
#define LzmaEnc_Create _ctm_LzmaEnc_Create
#define LzmaEnc_FreeLits _ctm_LzmaEnc_FreeLits
#define LzmaEnc_Destruct _ctm_LzmaEnc_Destruct
#define LzmaEnc_Destroy _ctm_LzmaEnc_Destroy
#define LzmaEnc_Init _ctm_LzmaEnc_Init
#define LzmaEnc_InitPrices _ctm_LzmaEnc_InitPrices
#define LzmaEnc_PrepareForLzma2 _ctm_LzmaEnc_PrepareForLzma2
#define LzmaEnc_MemPrepare _ctm_LzmaEnc_MemPrepare
#define LzmaEnc_Finish _ctm_LzmaEnc_Finish
#define LzmaEnc_GetNumAvailableBytes _ctm_LzmaEnc_GetNumAvailableBytes
#define LzmaEnc_GetCurBuf _ctm_LzmaEnc_GetCurBuf
#define LzmaEnc_CodeOneMemBlock _ctm_LzmaEnc_CodeOneMemBlock
#define LzmaEnc_Encode _ctm_LzmaEnc_Encode
#define LzmaEnc_WriteProperties _ctm_LzmaEnc_WriteProperties
#define LzmaEnc_MemEncode _ctm_LzmaEnc_MemEncode
#define LzmaEncode _ctm_LzmaEncode

/* LzmaLib.c */
#define LzmaCompress _ctm_LzmaCompress
#define LzmaUncompress _ctm_LzmaUncompress

/* LzFindMt.c */
#define MatchFinderMt_Construct _ctm_MatchFinderMt_Construct
#define MatchFinderMt_Destruct _ctm_MatchFinderMt_Destruct
#define MatchFinderMt_Create _ctm_MatchFinderMt_Create
#define MatchFinderMt_CreateVTable _ctm_MatchFinderMt_CreateVTable
#define MatchFinderMt_ReleaseStream _ctm_MatchFinderMt_ReleaseStream

/* Threads.c */
#define Thread_Close _ctm_Thread_Close
#define Thread_Create _ctm_Thread_Create
#define Thread_Wait _ctm_Thread_Wait
#define Thread_HardwareConcurrency _ctm_Thread_HardwareConcurrency
#define ManualResetEvent_Create _ctm_ManualResetEvent_Create
#define ManualResetEvent_CreateNotSignaled _ctm_ManualResetEvent_CreateNotSignaled
#define AutoResetEvent_Create _ctm_AutoResetEvent_Create
#define AutoResetEvent_CreateNotSignaled _ctm_AutoResetEvent_CreateNotSignaled
#define Event_Close _ctm_Event_Close
#define Event_Reset _ctm_Event_Reset
#define Event_Set _ctm_Event_Set
#define Event_Wait _ctm_Event_Wait
#define Semaphore_Create _ctm_Semaphore_Create
#define Semaphore_ReleaseN _ctm_Semaphore_ReleaseN
#define Semaphore_Release1 _ctm_Semaphore_Release1
#define Semaphore_Wait _ctm_Semaphore_Wait
#define Semaphore_Close _ctm_Semaphore_Close
#define CriticalSection_Init _ctm_CriticalSection_Init

#endif /* LZMA_PREFIX_CTM */

#endif /* __7Z_NAMEMANGLE_H */
