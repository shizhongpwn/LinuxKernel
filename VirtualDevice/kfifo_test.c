#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEMO_DEV_NAME "/dev/demo_dev"

int main(int argc, char **argv) {
    char buf[64];
    int fd;
    int ret;
    size_t len;
    char message[] = "Testing the virtual FIFO device";
    char *read_buffer;
    len = sizeof(message);
    fd = open(DEMO_DEV_NAME, O_RDWR);
    printf("the fd is %d\n", fd);
    if (fd < 0) {
        printf("open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    ret = write(fd, message, len);
    if (ret != len) {
        printf("cat't write on device %d, ret = %d\n", fd, ret);
        return -1;
    }
    read_buffer = (char *)malloc(len*2);
    memset(read_buffer, 0, len*2);
    ret = read(fd, read_buffer, len*2);
    printf("read %d bytes\n", ret);
    printf("read buffer is %s\n", read_buffer);
    close(fd);
    return 0;
}
