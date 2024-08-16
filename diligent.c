#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdbool.h>
#include<assert.h>
#include<sys/wait.h>

#define STATE_FILE "last_valid_state.txt"

void write_last_valid_board(int board[3][3]) {
    char temp_filename[] = "last_valid_state_temp.txt";
    int fd = open(temp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        perror("Unable to open temporary state file");
        sync();
        exit(EXIT_FAILURE);
    }

    FILE *temp_file = fdopen(fd, "w");
    if (temp_file == NULL) {
        perror("Unable to fdopen temporary state file");
        close(fd);
        sync();
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            fprintf(temp_file, "%d ", board[i][j]);
        }
        fprintf(temp_file, "\n");
    }

    fflush(temp_file); // Ensure data is flushed to the file
    if (fsync(fd) != 0) {
        perror("Unable to sync temporary state file");
        fclose(temp_file);
        sync();
        exit(EXIT_FAILURE);
    }
    fclose(temp_file);

    if (rename(temp_filename, STATE_FILE) != 0) {
        perror("Unable to rename temporary state file");
        sync();
        exit(EXIT_FAILURE);
    }
}


int winner(int board[3][3]){
    //check rows
    for(int i = 0; i < 3; i++){
        if(board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != 0){
            return board[i][0];
        }
    }
    //check columns
    for(int i = 0; i < 3; i++){
        if(board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != 0){
            return board[0][i];
        }
    }
    //check diagonals
    if(board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != 0){
        return board[0][0];
    }
    if(board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != 0){
        return board[0][2];
    }
    //check if there is any empty cell
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(board[i][j] == 0){
                return -1; // game not finished
            }
        }
    }
    return 0; // draw
}

void read_last_valid_board(int last_board[3][3]){
    FILE *file = fopen(STATE_FILE, "r");
    if(file == NULL){
        perror("Unable to read the last valid state");
        sync();
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            fscanf(file, "%d", &last_board[i][j]);
        }
    }
    fclose(file);
}

void copy_board(int dest[3][3], int src[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

bool detect_cheating(int last_board[3][3], int current_board[3][3]) {

    int change = 0;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(last_board[i][j] != current_board[i][j]){
                if(last_board[i][j] != 0){
                    return true;
                } else {
                    change++;
                }
                if(change > 1){
                    return true;
                }
            }
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid == 0) {
        assert(argc == 10);

        int last_board[3][3] = { {0, 0, 0}, {0, 0, 0}, {0, 0, 0} };
        int current_board[3][3];

        // Initialize the current board from arguments
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                current_board[i][j] = atoi(argv[i*3 + j + 1]);
            }
        }

        // Read the last valid board state from the file
        read_last_valid_board(last_board);

        // Check for cheating
        if(detect_cheating(last_board, current_board)){
            printf("cheating detected\n");

            exit(EXIT_SUCCESS);
        }
        copy_board(last_board, current_board);
        // Check if the game is over
        int result = winner(current_board);
        if(result == 0){
            printf("Draw\n");
        }else if(result == 1){
            printf("P1 won\n");
        }else if(result == 2){
            printf("P2 won\n");
        }
        if(result != -1){
            exit(EXIT_SUCCESS);
        }

        // Make a move
        int move = -1;
        while(1){
            move = rand() % 9;
            if(current_board[move/3][move%3] == 0){
                current_board[move/3][move%3] = 1;
                break;
            }
        }

        // Write the new valid board state to the file
        write_last_valid_board(current_board);

        // Call fairgame_p2 with the new arguments
        char* args[10];
        args[0] = "./fairgame_p2";
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                args[i*3 + j + 1] = (char*)malloc(2);
                sprintf(args[i*3 + j + 1], "%d", current_board[i][j]);
            }
        }
        args[10] = NULL;

        sync(); 
        execvp(args[0], args);
        perror("Unable to execute\n");
        sync();
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process: Wait for the child to finish
        int status;
        waitpid(pid, &status, 0);

        FILE *file = fopen(STATE_FILE, "w"); // Open the file in write mode
        if (file == NULL) {
            perror("Unable to clear the state file");
            exit(EXIT_FAILURE);
        }
        fclose(file); // Close the file, which effectively clears its contents
    return 0;
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
