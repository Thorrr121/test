#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define THREAD_COUNT 30  // Number of threads
#define CONNECTION_COUNT 1000  // Connections per thread

typedef struct {
    char *ip;
    int port;
    int interval;  // Time interval in seconds
} thread_data_t;

// Function for each thread to establish multiple connections
void *establish_connections(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int connection_attempts = 0;

    for (int i = 0; i < CONNECTION_COUNT; i++) {
        int sock;
        struct sockaddr_in server_addr;

        // Create a socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            continue;
        }

        // Configure server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(data->port);
        if (inet_pton(AF_INET, data->ip, &server_addr.sin_addr) <= 0) {
            perror("Invalid address or Address not supported");
            close(sock);
            continue;
        }

        // Attempt to connect to the server
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            printf("Connection established by thread %ld (attempt %d)\n", pthread_self(), ++connection_attempts);
        } else {
            perror("Connection failed");
        }

        // Close the connection
        close(sock);

        // Wait for the specified interval before the next connection
        sleep(data->interval);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <INTERVAL>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int interval = atoi(argv[3]);

    pthread_t threads[THREAD_COUNT];
    thread_data_t data = {ip, port, interval};

    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&threads[i], NULL, establish_connections, &data) != 0) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads completed.\n");
    return EXIT_SUCCESS;
}
