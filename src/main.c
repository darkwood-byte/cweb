#include "main.h"

// Functie om MIME type te bepalen
const char* get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".svg") == 0) return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    
    return "application/octet-stream";
}

// Functie om bestandssize te krijgen
long get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

// Functie om URL te decoderen (basisimplementatie)
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if (*src == '%' && (a = src[1]) && (b = src[2]) && 
            isxdigit(a) && isxdigit(b)) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Functie om HTTP request te parseren en pad te extraheren
int parse_request(const char *request, char *filepath, size_t max_len) {
    // Zoek naar GET /file HTTP
    if (strncmp(request, "GET ", 4) != 0 &&
        strncmp(request, "HEAD ", 5) != 0) {
        return -1; // Alleen GET en HEAD ondersteund
    }
    
    const char *url_start = request + 4;
    if (request[0] == 'H') url_start = request + 5; // Voor HEAD
    
    const char *url_end = strchr(url_start, ' ');
    if (!url_end) return -1;
    
    size_t url_len = url_end - url_start;
    if (url_len >= max_len) url_len = max_len - 1;
    
    // Kopieer URL
    strncpy(filepath, url_start, url_len);
    filepath[url_len] = '\0';
    
    // URL decoderen
    char decoded[MAX_PATH];
    url_decode(decoded, filepath);
    strcpy(filepath, decoded);
    
    // Als het root is, geef index.html
    if (strcmp(filepath, "/") == 0) {
        strcpy(filepath, "/index.html");
    }
    
    return 0;
}

// Functie om een bestand te serveren
void serve_file(int client_fd, const char *filepath) {
    char fullpath[MAX_PATH];
    char buffer[BUFFER_SIZE];
    
    // Beveiliging: verwijder ".." om directory traversal te voorkomen
    if (strstr(filepath, "..")) {
        const char *response = 
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<h1>403 Forbidden</h1>";
        send(client_fd, response, strlen(response), 0);
        return;
    }
    
    // Maak lokaal pad (verwijder leading slash)
    const char *home = getenv("HOME");
    if (!home) home = "/root";
    if (filepath[0] == '/') {
        snprintf(fullpath, sizeof(fullpath), "%s/cweb/websrc/%s", home, filepath + 1);
    } else {
        snprintf(fullpath, sizeof(fullpath), "%s/cweb/websrc/%s", home, filepath);
    }
    
    // Probeer bestand te openen
    FILE *file = fopen(fullpath, "rb");
    if (!file) {
        // 404 Not Found
        const char *response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<h1>404 Not Found</h1>"
            "<p>File not found: %s</p>";
        char error_response[BUFFER_SIZE];
        snprintf(error_response, sizeof(error_response), response, filepath);
        send(client_fd, error_response, strlen(error_response), 0);
        return;
    }
    
    // Bepaal bestandsgrootte
    long file_size = get_file_size(file);
    
    // Stuur HTTP headers
    const char *mime_type = get_mime_type(filepath);
    char headers[512];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n",
             mime_type, file_size);
    
    send(client_fd, headers, strlen(headers), 0);
    
    // Stuur bestandsinhoud
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_fd, buffer, bytes_read, 0);
    }
    
    fclose(file);
}

 int server_fd, client_fd;
struct sockaddr_in address;
int addrlen = sizeof(address);

void handle_client(void){
 // Accepteer nieuwe client
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address,  (socklen_t*)&addrlen)) < 0) {
        perror("accept");
    }
   else{

   }     
    // Lees HTTP request
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        
    if (bytes_read > 0) {
        // Parse request
        char filepath[MAX_PATH];
        if (parse_request(buffer, filepath, sizeof(filepath)) == 0) {
            printf("Request: %s -> Serving: %s\n", strtok(buffer, "\r\n"), filepath);
            serve_file(client_fd, filepath);
        } else {
             // Bad request
            const char *response = 
                "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/html\r\n\r\n"
                "<h1>400 Bad Request</h1>";
            send(client_fd, response, strlen(response), 0);
        }
    }  
    close(client_fd);   
}

void init_socket(void){
    // Maak socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Configureer socket opties
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // Bind adres
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Luister naar connections
    if (listen(server_fd, backlog) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int main(void){
    if(!init_config())return 1;

    printf("config done. . .\n");

    init_socket();    
    
    printf("Server draait op http://localhost:%d/\n", port);
    printf("Druk op Ctrl+C om te stoppen\n\n");
    
    // Hoofdlus
    while (1) {
       handle_client();
    }
    
    close(server_fd);
    return 0;
}