#include "config.h"

uint16_t port = 0;
uint16_t backlog = 0; 
static bool handle_config_line(char *buffer){
    if(!strncmp("echo ", buffer, 5))printf("%s\n", buffer + 5);  
    else if(!strncmp("port ", buffer, 5))port = atoi(buffer + 5);
    else if(!strncmp("backlog ", buffer, 8))backlog = atoi(buffer + 8);
    else{printf("invalid config argument: %s\n", buffer); return false;}
    return true;
}

static bool check_config(void){
    if(port == 0){printf("no port selected in config.txt. aborting. . .\n"); return false;}
    if(backlog == 0){printf("no backlog size selected in config.txt. aborting. . .\n"); return false;}
    return true;
}

bool init_config(void){
    char config_path[512];
    const char *home = getenv("HOME");
    if (!home) home = "/root";
    snprintf(config_path, sizeof(config_path), "%s/cweb/config.txt", home);
    FILE *config  = fopen(config_path, "r");
    if(config == NULL){printf("config file not found. . .\n"); return false;}
    printf("loading setings. . .\n");
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), config) != NULL) {
       if(!handle_config_line(buffer))return false;
    }

    if(!check_config())return false;

    return true;
}
