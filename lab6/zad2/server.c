#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>
#include <string.h>

double f(double x) {
    return 4.0 / (x*x + 1);
}

double integrate(double a, double b, double step) {
    double result = 0.0;
    int n = (int)((b-a) / step);
    for(int i=0; i<n; i++) {
        double x = a + i*step;
        result += f(x) * step;
    }
    return result;
}

int main() {
    const char *fifo_req = "fifo_request";
    const char *fifo_res = "fifo_response";
    if (mkfifo(fifo_req, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo fifo_request");
        return 1;
    }
    if (mkfifo(fifo_res, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo fifo_response");
        return 1;
    }

    while (1) {
        double a, b;
        int fd_in = open(fifo_req, O_RDONLY);
        if(fd_in == -1) {
            perror("open fifo_request");
            continue;
        }

        ssize_t r1 = read(fd_in, &a, sizeof(double));
        ssize_t r2 = read(fd_in, &b, sizeof(double));
        close(fd_in);

        if(r1 != sizeof(double) || r2 != sizeof(double)) {
            fprintf(stderr, "Błąd odczytu danych wejściowych\n");
            continue;
        }

        double result = integrate(a, b, 1e-6);

        int fd_out = open(fifo_res, O_WRONLY);
        if(fd_out == -1) {
            perror("open fifo_response");
            continue;
        }

        ssize_t written = write(fd_out, &result, sizeof(double));
        if(written != sizeof(double)) {
            fprintf(stderr, "Błąd zapisu danych wyjściowych\n");
        }

        close(fd_out);
    }

    return 0;
}