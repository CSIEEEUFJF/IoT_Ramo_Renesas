#ifndef UI_H
#define UI_H

#include "bsp_api.h"
#include "tx_api.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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
bool ui_get_uploaded_photo_info(const char *photo_id, uint16_t *out_width, uint16_t *out_height);
bool ui_copy_uploaded_photo_rows_rgb565(const char *photo_id,
                                        uint16_t start_row,
                                        uint16_t row_count,
                                        uint16_t *out_pixels,
                                        size_t max_pixels);
void ui_invalidate_profile_cache(void);

#endif /* UI_H */
