#include "who_camera.h"
#include "app_wifi.h"
#include "app_httpd.hpp"
#include "app_mdns.h"
#include "freertos/queue.h"

static QueueHandle_t xQueueStreamFrame = NULL;

extern "C" void app_main()
{
    // Configure mdns and wifi to be able to connect and stream on IP and at address: http://192.168.4.1:81/stream
    app_wifi_main();
    app_mdns_main();

    xQueueStreamFrame = xQueueCreate(2, sizeof(camera_fb_t *));

    // JPEG format for fast capture and streaming; 240x240 format as this is the closest the the desired of 172x172;
    // 2 frame buffers to allow for continuous streaming for approx. double the frame rate.
    register_camera(PIXFORMAT_JPEG, FRAMESIZE_240X240, 2, xQueueStreamFrame);
    register_httpd(xQueueStreamFrame, NULL, true);

    // After running connect to wifi name "ESP Stream" and go to address listed above to view/record stream.
}
