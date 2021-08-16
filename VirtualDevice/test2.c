#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define DEMO_DEV_NAME  "/dev/demo_dev"
int main(int argc, char *argv[]) {
    char buffer[64];
    int fd;
    int ret;
    size_t len;
    char message[] = "Testing the virtual FIFO device";
    char *read_buffer = NULL;
    len = sizeof(message);
    fd = open(DEMO_DEV_NAME, O_RDWR | O_NONBLOCK); //非阻塞模式读取
    if (fd < 0) { 
        printf("open device failed\n");
        return -1;
    }
    ret = write(fd, message, len);
    if (ret != len) {
        printf("cant't writet device \n");
    }
    read_buffer = malloc(2 * len);
    memset(read_buffer, 0, 2 * len); 
    close(fd);
    fd  = open(DEMO_DEV_NAME, O_RDWR);
    if (fd < 0) { 
        printf("open device failed\n");
        return -1;
    }
    ret = read(fd, read_buffer, 2 * len);
    printf("read %d bytes\n", ret);
    printf("read buffer=%s\n", read_buffer);
    close(fd);
    return 0;
}
