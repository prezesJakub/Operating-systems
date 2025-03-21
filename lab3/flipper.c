#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

unsigned long checksum(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Nie można otworzyć pliku");
        return 0;
    }
    unsigned long sum = 0;
    int ch;
    while((ch = fgetc(file)) != EOF) {
        sum += ch;
    }
    fclose(file);
    return sum;
}

void reverse_lines(const char *input_path, const char *output_path) {
    FILE *input = fopen(input_path, "r");
    FILE *output = fopen(output_path, "w");

    if (!input || !output) {
        perror("Błąd otwierania plików");
        return;
    }

    char line[1024];
    while(fgets(line, sizeof(line), input)) {
        size_t len = strlen(line);
        if(len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        for(size_t i = 0; i < len/2; i++) {
            char temp = line[i];
            line[i] = line[len-i-1];
            line[len-i-1] = temp;
        }
        fprintf(output, "%s\n", line);
    }

    fclose(input);
    fclose(output);
}

void process_directory(const char *source_dir, const char *output_dir) {
    DIR *dir = opendir(source_dir);
    if(!dir) {
        perror("Nie można otworzyć katalogu");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir))) {
        char input_path[512], output_path[512];
        snprintf(input_path, sizeof(input_path), "%s/%s", source_dir, entry->d_name);
        snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, entry->d_name);

        printf("Przetwarzanie pliku: %s\n", entry->d_name);

        unsigned long checksum_before = checksum(input_path);
        reverse_lines(input_path, output_path);
        unsigned long checksum_after = checksum(output_path);

        printf("Suma kontrolna przed: %lu\n", checksum_before);
        printf("Suma kontrolna po: %lu\n", checksum_after);
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Użycie: %s <katalog źródłowy> <katalog wynikowy>\n", argv[0]);
        return 1;
    }
    process_directory(argv[1], argv[2]);
    return 0;
}