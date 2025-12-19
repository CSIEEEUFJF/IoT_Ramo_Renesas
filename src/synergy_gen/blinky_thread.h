/* generated thread header file - do not edit */
#ifndef BLINKY_THREAD_H_
#define BLINKY_THREAD_H_
#include "bsp_api.h"
#include "tx_api.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void blinky_thread_entry(void);
                #else
extern void blinky_thread_entry(void);
#endif
#include "sf_spi.h"
#include "sf_spi_api.h"
#ifdef __cplusplus
extern "C" {
#endif
/* SF SPI on SF SPI Instance. */
extern const sf_spi_instance_t g_sf_spi0;
extern TX_QUEUE g_command_queue;
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* BLINKY_THREAD_H_ */
