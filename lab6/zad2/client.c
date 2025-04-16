#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    const char *fifo_req = "fifo_request";
    const char *fifo_res = "fifo_response";

    double a, b;
    printf("Podaj początek przedziału: ");
    if (scanf("%lf", &a) != 1) {
        fprintf(stderr, "Błąd podczas wczytywania a\n");
        return 1;
    }
    printf("Podaj koniec przedziału: ");
    if (scanf("%lf", &b) != 1) {
        fprintf(stderr, "Błąd podczas wczytywania b\n");
        return 1;
    }

    int fd_out = open(fifo_req, O_WRONLY);
    if (fd_out == -1) {
        perror("open fifo_request");
        exit(1);
    }
    if (write(fd_out, &a, sizeof(double)) != sizeof(double) ||
        write(fd_out, &b, sizeof(double)) != sizeof(double)) {
        perror("write to fifo_request");
        close(fd_out);
        return 1;
    }
    close(fd_out);

    int fd_in = open(fifo_res, O_RDONLY);
    if (fd_in == -1) {
        perror("open fifo_response");
        exit(1);
    }

    double result;
    if (read(fd_in, &result, sizeof(double)) != sizeof(double)) {
        perror("read from fifo_response");
        close(fd_in);
        return 1;
    }
    close(fd_in);

    printf("Wynik całkowania w przedziale [%.4f, %.4f] wynosi %.12f\n", a, b, result);

    return 0;
}