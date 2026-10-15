#include "server.h"

int main(int argc, char *argv[]) {
    int port = 8080;
    const char *static_dir = "frontend";
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--dir") == 0 && i + 1 < argc) {
            static_dir = argv[++i];
        }
    }
    
    server_init(port, static_dir);
    return server_run();
}
