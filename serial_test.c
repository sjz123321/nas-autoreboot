#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD_RATE B9600

// 设置串口参数
int set_serial_attributes(int fd) {
    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        perror("Error getting serial attributes");
        return -1;
    }

    // 设置波特率
    cfsetispeed(&options, BAUD_RATE);
    cfsetospeed(&options, BAUD_RATE);

    // 8位数据，1位停止位，无校验
    options.c_cflag &= ~PARENB;  // 无校验
    options.c_cflag &= ~CSTOPB;  // 1位停止位
    options.c_cflag &= ~CSIZE;   // 清除数据位
    options.c_cflag |= CS8;      // 8位数据
    options.c_cflag |= CLOCAL;   // 本地连接，不用控制调制解调器
    options.c_cflag |= CREAD;    // 启用接收

    // 禁用流控制
    options.c_cflag &= ~CRTSCTS;

    // 设置读取模式（非规范模式）
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    // 设置新的串口属性
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("Error setting serial attributes");
        return -1;
    }

    return 0;
}

int main() {
    int fd;
    char buffer[256];
    ssize_t bytesRead;

    // 打开串口
    fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        perror("Error opening serial port");
        return -1;
    }

    // 设置串口参数
    if (set_serial_attributes(fd) != 0) {
        close(fd);
        return -1;
    }

    printf("Listening on %s...\n", SERIAL_PORT);

    // 持续监听串口
    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // 读取串口数据
        bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';  // 确保字符串结束

            // 判断是否包含 "monitor"
            if (strstr(buffer, "monitor") != NULL) {
                printf("Received: %s\n", buffer);
                // 回复 "OK!"
                write(fd, "OK!\n", 4);
            }
        } else if (bytesRead < 0) {
            perror("Error reading from serial port");
        }

        usleep(100000);  // 避免占用过多CPU资源
    }

    close(fd);
    return 0;
}

