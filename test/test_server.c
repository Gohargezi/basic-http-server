#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "unity.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PORT 8080
#define BUFFER 2048

void setUp(void) {}
void tearDown(void) {}

/* ---------------------------------------------
   Helper: send HTTP request, return response
--------------------------------------------- */
int http_request(const char *req, char *response_out, size_t max_len) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK)
    };

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    if (send(sock, req, strlen(req), 0) < 0) {
        perror("send");
        close(sock);
        return -1;
    }

    int n = recv(sock, response_out, max_len - 1, 0);
    if (n < 0) {
        perror("recv");
        close(sock);
        return -1;
    }

    response_out[n] = '\0';
    close(sock);
    return 0;
}

/* ---------------------------------------------
   Test 1: GET /
--------------------------------------------- */
void test_get_root() {
    char buf[BUFFER];

    const char *req =
        "GET / HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    TEST_ASSERT_EQUAL_INT(0, http_request(req, buf, BUFFER));
    TEST_ASSERT_NOT_NULL(strstr(buf, "200 OK"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "index.html"));
}

/* ---------------------------------------------
   Test 2: GET missing file
--------------------------------------------- */
void test_get_missing() {
    char buf[BUFFER];

    const char *req =
        "GET /missing.txt HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";

    TEST_ASSERT_EQUAL_INT(0, http_request(req, buf, BUFFER));

    TEST_ASSERT_NOT_NULL(strstr(buf, "404 Not Found"));
}

/* ---------------------------------------------
   Test 3: POST /
--------------------------------------------- */
void test_post_not_implemented() {
    char buf[BUFFER];

    const char *req =
        "POST / HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";

    TEST_ASSERT_EQUAL_INT(0, http_request(req, buf, BUFFER));

    TEST_ASSERT_NOT_NULL(strstr(buf, "501 Not Implemented"));
}

/* ---------------------------------------------
   Test runner
--------------------------------------------- */
int main() {
    UNITY_BEGIN();

    RUN_TEST(test_get_root);
    RUN_TEST(test_get_missing);
    RUN_TEST(test_post_not_implemented);

    return UNITY_END();
}
