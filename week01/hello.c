/* First program: compile with gcc, run, and time it. */
#include <stdio.h>
#include <unistd.h>

int main(void) {
    char host[256];
    gethostname(host, sizeof host);
    printf("Hello from %s\n", host);
    return 0;
}
