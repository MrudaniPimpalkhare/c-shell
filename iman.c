#include "iman.h"


void strip_html_tags(char *text)
{
    char *src = text;
    char *dst = text;
    int in_tag = 0;

    while (*src)
    {
        if (*src == '<')
        {
            in_tag = 1;
        }
        else if (*src == '>')
        {
            in_tag = 0;
            src++;
            continue;
        }

        if (!in_tag)
        {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}


void print_after_doctype(char *buffer, size_t length) {
    char *doctype = "<html";
    char *body_start = buffer;
    
    // Find the start of the `DOCTYPE` declaration
    char *doctype_pos = strstr(buffer, doctype);
    if (doctype_pos != NULL) {
        // Print everything after the `DOCTYPE` declaration
        body_start = doctype_pos + strlen(doctype);
    }
    
    // Print the content starting from the `DOCTYPE` position
    strip_html_tags(body_start);
    printf("%s", body_start);
}

void fetch_man_page(char *command)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[16384];     // Buffer to hold the response
    char request[1024];    // Buffer to hold the HTTP GET request
    int header_skipped = 0; // Flag to indicate if headers have been skipped
    int body_started = 0;
    char *patp = "\r\n\r\n";
    char *body;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    // Get server info
    server = gethostbyname("man.he.net");
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(80);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    // Create HTTP GET request
    snprintf(request, sizeof(request),
             "GET /?topic=%s&section=all HTTP/1.0\r\nHost: man.he.net\r\n\r\n", command);

    // Send request
    if (write(sockfd, request, strlen(request)) < 0)
    {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }

    // Read response
    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0'; // Null-terminate buffer

        if (!header_skipped) {
            char *header_end = strstr(buffer, patp);
            if (header_end != NULL) {
                printf("%s header haha", header_end);
                header_end += strlen(patp); // Skip the "\r\n\r\n"
                // Print body content after header
                print_after_doctype(header_end, strlen(header_end));
                header_skipped = 1;
            } else {
                // Print the buffer as part of the header
                print_after_doctype(buffer, n);
            }
        } else {
            // Print the rest of the body after headers and `DOCTYPE`
            print_after_doctype(buffer, n);
        }
    }

    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
}