#pragma once

#include <stdint.h>
#include "port_x.h"

#ifdef ARDUINO8266_SERVER_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif
#define	HOMEKIT_OVERCLOCK_PAIR_VERIFY
	uint32_t homekit_random();
	void homekit_random_fill(uint8_t *data, size_t size);

	void homekit_system_restart();
	void homekit_overclock_start();
	void homekit_overclock_end();
	
	void esp_increase_alligned_pointer(byte** ppointer, size_t size);
	void* malloc_buffered(byte** ppointer, size_t size);
#ifdef ARDUINO8266_SERVER_CPP
#ifdef __cplusplus
}
#endif

#endif

#ifdef ESP_OPEN_RTOS
#include <spiflash.h>
#define ESP_OK 0
#endif



#ifdef ESP_IDF
#include <esp_system.h>
#include <esp_spi_flash.h>
#define SPI_FLASH_SECTOR_SIZE SPI_FLASH_SEC_SIZE
#define spiflash_read(addr, buffer, size) (spi_flash_read((addr), (buffer), (size)) == ESP_OK)
#define spiflash_write(addr, data, size) (spi_flash_write((addr), (data), (size)) == ESP_OK)
#define spiflash_erase_sector(addr) (spi_flash_erase_sector((addr) / SPI_FLASH_SECTOR_SIZE) == ESP_OK)
#elif defined ESP32 && defined ARDUINO
#include <esp_system.h>
#include <esp_spi_flash.h>
#define SPI_FLASH_SECTOR_SIZE SPI_FLASH_SEC_SIZE
#define spiflash_read(addr, buffer, size) (spi_flash_read((addr), (buffer), (size)) == ESP_OK)
#define spiflash_write(addr, data, size) (spi_flash_write((addr), (data), (size)) == ESP_OK)
#define spiflash_erase_sector(addr) (spi_flash_erase_sector((addr) / SPI_FLASH_SECTOR_SIZE) == ESP_OK)


//#define HOMEKIT_DEBUG
#define EX_STORAGE_CHAR //will use external storage

#elif defined(ESP8266) && defined(ARDUINO)

#include <Arduino.h>


#define ACCESSORY_ID_SIZE   17
//#define ARDUINO8266_SERVER


#define EX_STORAGE_CHAR //will use external storage
#include <spi_flash.h>
#define SPI_FLASH_SECTOR_SIZE SPI_FLASH_SEC_SIZE
//#include <lwip2/sockets.h>

#define portTICK_PERIOD_MS 10
#if( configUSE_16_BIT_TICKS == 1 )
typedef uint16_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif
//#error "Not supported platform"
//#define LWIP_SOCKET 1
//#define LWIP_TIMEVAL_PRIVATE 0
#include <lwip/sockets.h>

#include <queue.h>
#include "types.h"
#include <lwip/init.h>
#include <lwip/ip_addr.h>
//#define ipv4_addr ip_addr
//#define ipv4_addr_t ip_addr_t
typedef struct {
	homekit_characteristic_t *characteristic;
	homekit_value_t value;
} characteristic_event_t;

#define MAX_PAIRINGS 4
//#define malloc(p) os_malloc(p)
//#define free(p) os_free(p)

#else
#error "Not supported platform"
#endif


#ifdef ESP_IDF
#define SERVER_TASK_STACK 12288
#elif defined ESP32 && defined ARDUINO
#define SERVER_TASK_STACK 12288

#else
#define SERVER_TASK_STACK 2048

#endif
#if defined(ARDUINO) &&( (defined(ESP8266) && defined(ARDUINO8266_SERVER_CPP)) ||  defined(ESP32))
#ifdef __cplusplus
extern "C" {
#endif
	void* malloc_from_buffer(void* buffer, size_t buffer_size, size_t size, size_t* buffer_offset);
	void free_from_buffer(void* buffer, size_t buffer_size, void* pointer);
#ifdef __cplusplus
}
#endif

#ifdef USE_STACK_BUFFER
#define DECLARE_ALLOCATOR(size) \
	byte stack_buff[size]; \
	size_t stack_buff_pos=0;

#define _MALLOC(size) malloc_from_buffer(stack_buff,sizeof(stack_buff),size,&stack_buff_pos)
#define _FREE(pointer) free_from_buffer(stack_buff, sizeof(stack_buff),pointer)

#else
#define DECLARE_ALLOCATOR(size)
#define _MALLOC(size) malloc(size)
#define _FREE(pointer) free(pointer)
#endif 

#endif
  
#ifndef ARDUINO8266_SERVER_CPP
void homekit_mdns_init();
#endif
void homekit_mdns_configure_init(const char *instance_name, int port);
void homekit_mdns_add_txt(const char *key, const char *format, ...);
void homekit_mdns_add_txt_ex(const char *key, const char *val);
void homekit_mdns_configure_finalize();

void homekit_mdns_stop();