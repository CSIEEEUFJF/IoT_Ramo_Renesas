#ifndef UI_H
#define UI_H

#include "bsp_api.h"
#include "tx_api.h"
#include <stdint.h>
#include <stdbool.h>

void thread_ui_entry(ULONG arg);
bool ui_prepare_uploaded_photo_rgb565(const char *photo_id, uint16_t width, uint16_t height);
bool ui_write_uploaded_photo_tile_rgb565(const char *photo_id,
                                         const uint16_t *pixels,
                                         uint16_t tile_x,
                                         uint16_t tile_y,
                                         uint16_t tile_width,
                                         uint16_t tile_height);
bool ui_store_uploaded_photo_rgb565(const char *photo_id, const uint16_t *pixels, uint16_t width, uint16_t height);
bool ui_has_uploaded_photo(const char *photo_id);
void ui_invalidate_profile_cache(void);

#endif /* UI_H */
