#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

// wifi info
#define WIFI_SSID "Group_16"
#define WIFI_PASS "smart-systems"
#define WEB_SERVER "192.168.1.127"
// identification
#define HUB_ID "G016"
#define CERT "5039"
// LED Output pins definitions
#define BLUEPIN   33
// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)

char REQUEST[] =  "0000\t" HUB_ID "\t" CERT "\t";
char fob_id[4] = "XXXX";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static esp_err_t event_handler(void *ctx, system_event_t *event) {
  switch(event->event_id) {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    /* This is a workaround as ESP32 WiFi libs don't currently
       auto-reassociate. */
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void initialise_wifi(void) {
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  esp_event_loop_init(event_handler, NULL);
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  wifi_config_t wifi_config = {
    .sta = {
      .ssid = WIFI_SSID,
      .password = WIFI_PASS,
    },
  };
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
  esp_wifi_start();
}

void tcp_client() {
  for (int i = 0; i < 4; i++) {
    REQUEST[i] = fob_id[i];
  }
  printf("tcp_client task started \n");
  struct sockaddr_in tcpServerAddr;
  tcpServerAddr.sin_addr.s_addr = inet_addr(WEB_SERVER);
  tcpServerAddr.sin_family = AF_INET;
  // socket number
  tcpServerAddr.sin_port = htons( 3010 );
  int s, r;
  char recv_buf[64];
  xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,false,true,portMAX_DELAY);
  s = socket(AF_INET, SOCK_STREAM, 0);
  if(s < 0) {
    printf( "... Failed to allocate socket.\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    return;
  }
  printf("... allocated socket\n");
    if(connect(s, (struct sockaddr *)&tcpServerAddr, sizeof(tcpServerAddr)) != 0) {
      printf( "... socket connect failed errno=%d \n", errno);
      close(s);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      return;
  }
  printf("... connected \n");
  if( write(s , REQUEST , strlen(REQUEST)) < 0)
  {
    printf( "... Send failed \n");
    close(s);
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    return;
  }
  printf("... socket send success\n");
  do {
    bzero(recv_buf, sizeof(recv_buf));
    r = read(s, recv_buf, sizeof(recv_buf)-1);
    printf("RESPONSE:");
    for(int i = 0; i < r; i++) {
      putchar(recv_buf[i]);
    }
    if (recv_buf[0] == '1') {
      gpio_set_level(BLUEPIN, 1);
      printf("URL \n");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      gpio_set_level(BLUEPIN, 0);
      printf("LOCKED\n");
    } else if (recv_buf[0] == '0') {
      printf("ACCESS DENIED\n");
    }
  } while(r > 0);
  printf("... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
  close(s);
  printf("... new request in 5 seconds");
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  printf("...tcp_client task closed\n");
}

/* ====================================================================================*/

// Configure UART
static void uart_init() {
  uart_config_t uart_config = {
    .baud_rate = 1200, // Slow BAUD rate
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_config);
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_set_line_inverse(UART_NUM_1,UART_INVERSE_RXD);
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// GPIO init for LEDs
static void led_init() {
  gpio_pad_select_gpio(BLUEPIN);
  gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
  gpio_set_level(BLUEPIN, 0);
}

// Receives task -- looks for Start byte then stores received values
int ir_rx() {
  // Buffer for input data
  uint8_t *data_in = (uint8_t *) malloc(BUF_SIZE);
  int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 20 / portTICK_RATE_MS);
  if (len_in > 0) {
    for (int i=0; i < 24; i++) {
      /* !!!!!!!!!!!!!!!!!!!!!!!!*/
      if (data_in[i] == 0x0A && i + 4 < len_in) {
        for (int j = 0; j < 4; j++) {
          fob_id[j] = data_in[i+j+1];
        }
        printf("Received comm from device\n");
        free(data_in);
        return 1;
      }
    }
  }
  free(data_in);
  return 0;
}

void app_main() {
  // init
  uart_init();
  led_init();
  nvs_flash_init();
  initialise_wifi();
  // loop: check for ir signal, send to server
  while(1) {
    if(ir_rx()){
      printf("ID: %c%c%c%c\n", fob_id[0], fob_id[1], fob_id[2], fob_id[3]);
      tcp_client();
    }
  }
}
