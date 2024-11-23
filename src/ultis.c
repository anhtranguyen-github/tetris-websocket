
#include <stdio.h>
#include <time.h>
#include <string.h>  // For strlen
#include "ultis.h"   // Include the header file

void write_to_log(const char *message) {
    // Open the log file in append mode
    FILE *file = fopen("log.txt", "a");

    // Check if the file was opened successfully
    if (file == NULL) {
        printf("Error opening log file\n");
        return;
    }

    // Get the current time
    time_t current_time = time(NULL);
    char *time_str = ctime(&current_time);
    // Remove the newline character at the end of the time string
    time_str[strlen(time_str) - 1] = '\0';

    // Write the timestamp and message to the log file
    fprintf(file, "[%s] %s\n", time_str, message);

    // Close the file
    fclose(file);
}

void write_to_log_int(int value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%d", value);
    write_to_log(buffer); 
}
