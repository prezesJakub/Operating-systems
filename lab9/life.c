#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS 100

typedef struct {
	int id;
	int start_row;
	int end_row;
	char *src;
	char *dst;
	volatile sig_atomic_t ready;
} thread_data;

thread_data thread_args[MAX_THREADS];
pthread_t threads[MAX_THREADS];
int num_threads;

void handler(int signum) {

}

void *worker(void *arg) {
	thread_data *data = (thread_data *)arg;
	signal(SIGUSR1, handler);

	while(1) {
		pause();

		for(int i = data->start_row; i < data->end_row; i++) {
			for(int j = 0; j < grid_width; j++) {
				data->dst[i * grid_width + j] = is_alive(i, j, data->src);
			}
		}

		data->ready = 1;
	}

	return NULL;
}

void wait_for_workers() {
	for(int i=0; i<num_threads; ++i) {
		while(!thread_args[i].ready)
			usleep(100);
		thread_args[i].ready = 0;
	}
}

int main(int argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
		return 1;
	}

	num_threads = atoi(argv[1]);
	if(num_threads < 1 || num_threads > MAX_THREADS) {
		fprintf(stderr, "Invalid number of threads (1-%d)\n", MAX_THREADS);
		return 1;
	}

	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr();

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	init_grid(foreground);

	int rows_per_thread = grid_height / num_threads;
	int extra = grid_height % num_threads;

	for(int i=0, row=0; i<num_threads; ++i) {
		int span = rows_per_thread + (i < extra ? 1 : 0);
		thread_args[i] = (thread_data){
			.id = i,
			.start_row = row,
			.end_row = row + span,
			.src = foreground,
			.dst = background,
			.ready = 0
		};
		pthread_create(&threads[i], NULL, worker, &thread_args[i]);
		row += span;
	}

	while (1) {
		draw_grid(foreground);
		usleep(500 * 1000);

		for(int i=0; i<num_threads; ++i) {
			thread_args[i].src = foreground;
			thread_args[i].dst = background;
		}

		for(int i=0; i<num_threads; ++i) {
			pthread_kill(threads[i], SIGUSR1);
		}

		wait_for_workers();

		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin();
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}