#include <stdio.h>
struct test {
    const char *test1;
    int a;
};
int main(int argc, char **argv) {
    char buf[1024] = "asdadasdsa";
    struct test test1 = { &buf, sizeof(buf) };
    struct test* ptr = &test1;
    char buf2[1024] = "asdadasds";
    ptr->test1 = &buf2;
    (ptr->test1)[0] = '\0';
    
    printf("%s\n", ptr->test1);
    return 0;
}