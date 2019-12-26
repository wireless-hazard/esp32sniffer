// #include <string.h>
// #include "esp_system.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "driver/gpio.h"
// #include "esp_wifi.h"
// #include "esp_event_loop.h"
// #include "esp_mesh.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "lwip/sockets.h"

// #define ACCESSPOINT "apteste"
// #define KEY "magnomaia" 

// void wifi_init(){
//     tcpip_adapter_init();
//     ESP_ERROR_CHECK(esp_event_loop_init(NULL,NULL));
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     wifi_config_t wifi_config = {
//     	.sta = {
//     		.ssid = ACCESSPOINT,
//     		.password = KEY,
//     	},
//     };
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
//     ESP_ERROR_CHECK(esp_wifi_start());
// }

// void app_main(void){

// }
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_mesh.h"

#define apssid "apteste"
#define apsd "magnomaia"
#define APIP "192.168.137.1"
#define PORT 8000

#define WIFI_CONNECTED_BIT BIT0 

static EventGroupHandle_t s_wifi_event_group;
static const char *payload = "Message from ESP32 "; //Mensagem temporaria
static const char *TAG = "Esp_Information";
static const char stringteste[] = APIP;
static ip4_addr_t ipteste[4] = {0,};

static void idf_socket(void *pvParameters){
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    mesh_addr_t testando;
    testando.mip.ip4.addr = ipaddr_addr("192.168.137.1"); //inet_addr(APIP),
    testando.mip.port = 8000;
    memcpy(ipteste,&testando.mip.ip4.addr,sizeof(testando.mip.ip4.addr));
    ESP_LOGI(TAG,""IPSTR"", IP2STR(ipteste));
    struct sockaddr_in destAddr;
 
    int x1 = ((int)ip4_addr1(ipteste));
    int x2 = ((int)ip4_addr2(ipteste));
    int x3 = ((int)ip4_addr3(ipteste));
    int x4 = ((int)ip4_addr4(ipteste));

    char s1[4];
    char s2[4];
    char s3[4];
    char s4[4];
 
    char ip_final[19];

    sprintf(ip_final,"%d.%d.%d.%d",x1,x2,x3,x4);
    printf("%s\n",ip_final);

    destAddr.sin_addr.s_addr = inet_addr(ip_final);
    // destAddr.sin_addr.s_addr = inet_addr(&IPSTR);
    destAddr.sin_family = AF_INET;
    // destAddr.sin_port = htons(&testando.mip.port);
    destAddr.sin_port = htons((unsigned short)testando.mip.port);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
    
    int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    
    int err = connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    ESP_LOGI(TAG,"Conectou %d",err);
    while(1){
        send(sock,payload,strlen(payload), 0);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static esp_err_t eventocallback(void *ctx,system_event_t *evento){
    switch(evento->event_id){
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG,"SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(s_wifi_event_group,WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG,"SYSTEM_EVENT_STA_GOT_IP");
        xTaskCreatePinnedToCore(&idf_socket,"Comunicacao",4096,NULL,5,NULL,0);
        break;
    
    default:
        break;
    }
    return ESP_OK;
}

void wifiInit(){
    
    s_wifi_event_group = xEventGroupCreate();
    
    tcpip_adapter_init();
    esp_event_loop_init(eventocallback,NULL);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
     wifi_config_t wifi_config = {
        .sta = {
            .ssid = apssid,
            .password = apsd
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}

void app_main(){
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
        
    wifiInit();
}