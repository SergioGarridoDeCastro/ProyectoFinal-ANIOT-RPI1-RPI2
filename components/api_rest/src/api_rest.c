 /*
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_log.h"
//#include "esp_vfs.h"
#include "cJSON.h"
//#include "cbor.h"

#include "api_rest.h"
#include <arch.h>

static const char *REST_TAG = "server-rest";
#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

 // Set HTTP response content type according to file extension  
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}


static esp_err_t rest_common_get_handler(httpd_req_t *req){
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *) req->user_ctx;     
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    if(req->uri[strlen(req->uri) - 1] == '/'){
        strlcat(filepath, "/index.html", sizeof(filepath));
    }
    else{
        strlcat(filepath, req->uri, sizeof(filepath));
    }

    int fd = open(filepath, O_RDONLY, 0);
    if(fd == -1){
        ESP_LOGE(REST_TAG, "Error al abrir el fichero: %s", filepath);
         // Respond with 500 Internal Server Error  
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error al leer fichero existente");
        return ESP_FAIL;
    }
   
    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;

    do {
         // Read file in chunks into the scratch buffer  
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Error al leer el fichero : %s", filepath);
        } else if (read_bytes > 0) {
             // Send the buffer contents as HTTP response chunk  
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "Error al enviar el fichero");
                 // Abort sending file  
                httpd_resp_sendstr_chunk(req, NULL);
                 // Respond with 500 Internal Server Error  
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error al enviar el fichero");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    // Close file after sending complete 
    close(fd);
    ESP_LOGI(REST_TAG, "Envio de fichero completado");
    // Respond with an empty chunk to signal HTTP response completion 
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
    
}
 //
esp_err_t start_rest_server(const char *base_path){
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    URI handler for fetching system info 
    httpd_uri_t system_info_get_uri = {
        .uri = "/api/v1/system/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &system_info_get_uri);

     return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
} */