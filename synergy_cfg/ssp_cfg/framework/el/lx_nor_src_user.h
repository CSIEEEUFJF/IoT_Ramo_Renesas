/* generated configuration header file - do not edit */
#ifndef LX_NOR_SRC_USER_H_
#define LX_NOR_SRC_USER_H_
#if (1)
#include "lx_src_user.h"
#endif
#if (1)
#define LX_DIRECT_READ
#else
      /** If direct read disabled LevelX requires a buffer of a sector size i.e 512 bytes or 128 words */
      #define SSP_LX_READ_BUFFER_SIZE_WORDS (128U)
      #endif
#if (0)
      #define LX_FREE_SECTOR_DATA_VERIFY
      #endif
#define LX_NOR_SECTOR_MAPPING_CACHE_SIZE (16)
#endif /* LX_NOR_SRC_USER_H_ */
