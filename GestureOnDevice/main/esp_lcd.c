#include <string.h>
#include "esp_timer.h"
#include "esp_camera.h"
#include "esp_lcd.h"
#include "fb_gfx.h"

#include "logo_en_240x240_lcd.h"

static const char *TAG = "esp_lcd";

static scr_driver_t g_lcd;
static scr_info_t g_lcd_info;

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
TaskHandle_t lcd_task_handle;
static bool gReturnFB = true;
static int colour_det = 65535; // white
static int countdown_det = 0;
static int action_det = 0;
static uint16_t *strip_buf;

typedef enum
{
    INWARDS = 1,
    BOTHUP,
    CIRCLE,
    HDOWN,
    HUP,
    LEFT,
    RIGHT
} action_name_t;

static void task_process_handler(void *arg)
{
    lcd_frame_t *frame = NULL;

    uint64_t start_time = esp_timer_get_time();
    int total_frames = 0;
    camera_fb_t *fb = (camera_fb_t *) malloc(sizeof (camera_fb_t));
    if (fb == NULL) {
        ESP_LOGE(TAG, "fb memory allocation failed! line %d", __LINE__);
        return;
    }
    fb->buf = (void *) strip_buf;
    fb->len = 240 * (240 - 172);
    fb->width = 240;
    fb->height = (240 - 172);
    fb->format = PIXFORMAT_RGB565;

    while (true) {
        if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY)) {
            g_lcd.draw_bitmap(34, 0, frame->width, frame->height, (uint16_t *)frame->buf);

            for (int i = 0; i < 240 * (240 - 172); i++) {
                strip_buf[i] = colour_det;
            }
            total_frames++;
            uint64_t total_time = (esp_timer_get_time() - start_time) / 1000;
            int avg_fps = (total_frames * 1000) / total_time;
            // write fps on display
            if (countdown_det > 0){
                fb_gfx_printf(fb, 40, 12, 0xffff, "%3d", countdown_det);
            } else if (countdown_det == -1){
                // do nothing
            }else {
                //display the guess from the gesture
                switch (action_det) {
                    case INWARDS:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "BOTH INWARDS");
                    break;
                    case BOTHUP:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "BOTH HANDSUP");
                    break;
                    case CIRCLE:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "CIRCLE");
                    break;
                    case HDOWN:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "HAND DOWN");
                    break;
                    case HUP:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "HAND UP");
                    break;
                    case LEFT:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "SWIPE LEFT");
                    break;
                    case RIGHT:
                    fb_gfx_printf(fb, 40, 12, 0xffff, "SWIPE RIGHT");
                    break;
                    }
            }
            
            fb_gfx_printf(fb, 40, 42, 0xffff, "avg fps:%3d", avg_fps);
            g_lcd.draw_bitmap(0, 172, fb->width, fb->height, (uint16_t *)fb->buf);
            free(frame);
        }
    }
    free(fb);
}

esp_err_t register_lcd(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const bool return_fb)
{
    spi_config_t bus_conf = {
        .miso_io_num = BOARD_LCD_MISO,
        .mosi_io_num = BOARD_LCD_MOSI,
        .sclk_io_num = BOARD_LCD_SCK,
        .max_transfer_sz = 2 * 240 * 240 + 10,
    };
    spi_bus_handle_t spi_bus = spi_bus_create(SPI2_HOST, &bus_conf);

    scr_interface_spi_config_t spi_lcd_cfg = {
        .spi_bus = spi_bus,
        .pin_num_cs = BOARD_LCD_CS,
        .pin_num_dc = BOARD_LCD_DC,
        .clk_freq = 40 * 1000000,
        .swap_data = 0,
    };

    scr_interface_driver_t *iface_drv;
    scr_interface_create(SCREEN_IFACE_SPI, &spi_lcd_cfg, &iface_drv);
    esp_err_t ret = scr_find_driver(SCREEN_CONTROLLER_ST7789, &g_lcd);
    if (ESP_OK != ret) {
        return ret;
        ESP_LOGE(TAG, "screen find failed");
    }

    scr_controller_config_t lcd_cfg = {
        .interface_drv = iface_drv,
        .pin_num_rst = BOARD_LCD_RST,
        .pin_num_bckl = BOARD_LCD_BL,
        .rst_active_level = 0,
        .bckl_active_level = 0,
        .offset_hor = 0,
        .offset_ver = 0,
        .width = 240,
        .height = 240,
        .rotate = 0,
    };
    ret = g_lcd.init(&lcd_cfg);
    if (ESP_OK != ret) {
        return ESP_FAIL;
        ESP_LOGE(TAG, "screen initialize failed");
    }

    g_lcd.get_info(&g_lcd_info);
    ESP_LOGI(TAG, "Screen name:%s | width:%d | height:%d | colour_type: %d",
             g_lcd_info.name, g_lcd_info.width, g_lcd_info.height, g_lcd_info.color_type);

    app_lcd_set_colour(0x000000);
    vTaskDelay(pdMS_TO_TICKS(200));
    app_lcd_draw_wallpaper();
    vTaskDelay(pdMS_TO_TICKS(200));

    strip_buf = (uint16_t *) heap_caps_malloc(g_lcd_info.width * (g_lcd_info.height - 172) * sizeof(uint16_t),
                                              MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (strip_buf == NULL) {
        ESP_LOGE(TAG, "strip buffer not allocated");
        return ESP_FAIL;
    }

    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    gReturnFB = return_fb;
    
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, configMAX_PRIORITIES - 3, &lcd_task_handle, 0);

    return ESP_OK;
}

void app_lcd_draw_wallpaper()
{
    scr_info_t lcd_info;
    g_lcd.get_info(&lcd_info);

    uint16_t *pixels = (uint16_t *) heap_caps_malloc((logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t),
                                                     MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (NULL == pixels) {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
        return;
    }
    memcpy(pixels, logo_en_240x240_lcd, (logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t));
    g_lcd.draw_bitmap(0, 0, logo_en_240x240_lcd_width, logo_en_240x240_lcd_height, (uint16_t *)pixels);
    heap_caps_free(pixels);
}

void app_lcd_colour_for_inference(int colour, int countdown, int action)
{
    colour_det = colour;
    countdown_det = countdown;
    action_det = action;
}

void app_lcd_set_colour(int colour)
{
    scr_info_t lcd_info;
    g_lcd.get_info(&lcd_info);
    uint16_t *buffer = (uint16_t *)malloc(lcd_info.width * sizeof(uint16_t));
    if (NULL == buffer) {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
    } else {
        for (size_t i = 0; i < lcd_info.width; i++) {
            buffer[i] = colour;
        }

        for (int y = 0; y < lcd_info.height; y++) {
            g_lcd.draw_bitmap(0, y, lcd_info.width, 1, buffer);
        }

        free(buffer);
    }
}
