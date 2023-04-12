#include "who_camera.h"
#include "who_human_face_detection.hpp"
#include "app_wifi.h"
#include "app_httpd.hpp"
#include "app_mdns.h"
#include "freertos/queue.h"

static QueueHandle_t xQueueStreamFrame = NULL;
//static QueueHandle_t xQueueHttpFrame = NULL;

extern "C" void app_main()
{
    app_wifi_main();
    app_mdns_main();

    xQueueStreamFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    //xQueueHttpFrame = xQueueCreate(2, sizeof(camera_fb_t *));

    register_camera(PIXFORMAT_JPEG, FRAMESIZE_240X240, 2, xQueueStreamFrame);
    //register_human_face_detection(xQueueAIFrame, NULL, NULL, xQueueHttpFrame);
    register_httpd(xQueueStreamFrame, NULL, true);
}
