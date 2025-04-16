#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <math.h>

double f(double x) {
    return 4.0 / (x*x + 1);
}

double time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <step_size> <max_processes>\n", argv[0]);
        exit(1);
    }

    double step = atof(argv[1]);
    int max_procs = atoi(argv[2]);

    if(step <= 0 || max_procs <= 0) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    int total_steps = (int)(1.0 / step);

    for(int k=1; k<=max_procs; k++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);

        int pipes[k][2];
        pid_t pids[k];

        int chunk = total_steps / k;
        int remainder = total_steps % k;

        for(int i=0; i<k; i++) {
            if(pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(1);
            }

            pid_t pid = fork();
            if(pid == -1) {
                perror("fork");
                exit(1);
            }

            if(pid == 0) {
                close(pipes[i][0]);

                int start_idx = i * chunk + (i < remainder ? i : remainder);
                int local_steps = chunk + (i < remainder ? 1 : 0);

                double local_sum = 0.0;
                for(int j=0; j < local_steps; j++) {
                    double x = (start_idx + j) * step;
                    local_sum += f(x) * step;
                }

                if (write(pipes[i][1], &local_sum, sizeof(double)) != sizeof(double)) {
                    perror("write failed");
                    exit(1);
                }
                close(pipes[i][1]);
                exit(0);
            } else {
                pids[i] = pid;
                close(pipes[i][1]);
            }
        }

        double total_sum = 0.0;
        for(int i=0; i<k; i++) {
            double partial = 0.0;
            if (read(pipes[i][0], &partial, sizeof(double)) != sizeof(double)) {
                perror("read failed");
                exit(1);
            }
            total_sum += partial;
            close(pipes[i][0]);
            waitpid(pids[i], NULL, 0);
        }

        gettimeofday(&end, NULL);
        double duration = time_diff(start, end);

        printf("Procesy: %d, Wynik: %.24f, Czas: %.4fs\n", k, total_sum, duration);
    }

    return 0;
}