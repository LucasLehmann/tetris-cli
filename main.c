#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <windows.h>

HANDLE hStdin;
DWORD fdwSaveOldMode;

VOID ErrorExit(LPCSTR);
VOID KeyEventProc(KEY_EVENT_RECORD);
VOID MouseEventProc(MOUSE_EVENT_RECORD);

typedef struct {
    int x;
    int y;
} term_size_t;

enum BOARD { BOARD_WIDTH = 10, BOARD_HEIGHT = 22, BOARD_SIZE = BOARD_WIDTH*BOARD_HEIGHT };
enum PIECES { I_PIECE, O_PIECE, T_PIECE, S_PIECE, Z_PIECE, J_PIECE, L_PIECE, PIECE_COUNT };
const int PIECE_COLOURS[] = { 36, 33, 35, 32, 31, 34, 37 };

typedef int board_t[BOARD_SIZE/(sizeof(int)*8)+1];

typedef struct {
    int x;
    int y;
    int rotation;
    int type;
} piece_t;

typedef struct {
    int start;
    int end;
    char content[10];
} queue_t;

int set_bit(board_t board, int x, int y) {
    if(x < 0) return -1;
    if(y < 0) return -1;
    if(x >= BOARD_WIDTH) return -1;
    if(y >= BOARD_HEIGHT) return -1;
    int i = x + y * BOARD_WIDTH;
    board[i/(sizeof(*board)*8)] |= 1 << (i%(sizeof(*board)*8));
    return 0;
}

int clear_bit(board_t board, int x, int y) {
    if(x < 0) return -1;
    if(y < 0) return -1;
    if(x >= BOARD_WIDTH) return -1;
    if(y >= BOARD_HEIGHT) return -1;
    int i = x + y * BOARD_WIDTH;
    board[i/(sizeof(*board)*8)] &= ~(1 << (i%(sizeof(*board)*8)));
    return 0;
}

int get_bit(const board_t board, int x, int y) {
    if(x < 0) return -1;
    if(y < 0) return -1;
    if(x >= BOARD_WIDTH) return -1;
    if(y >= BOARD_HEIGHT) return -1;
    int i = x + y * BOARD_WIDTH;
    return (board[i/(sizeof(*board)*8)] >> (i%(sizeof(*board)*8))) & 1;
}

int is_invalid(const board_t board, piece_t piece) {
    switch(piece.type) {
        case I_PIECE:
            switch(piece.rotation) {
                case 0:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x+2, piece.y)
                           );
                case 1:
                    return(get_bit(board, piece.x+1, piece.y-1) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x+1, piece.y+1) |
                           get_bit(board, piece.x+1, piece.y+2)
                           );
                case 2:
                    return(get_bit(board, piece.x-1, piece.y+1) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x+1, piece.y+1) |
                           get_bit(board, piece.x+2, piece.y+1)
                           );
                case 3:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x, piece.y+2)
                           );
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        case O_PIECE:
            switch(piece.rotation) {
                case 0:
                case 1:
                case 2:
                case 3:
                    return(get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x+1, piece.y+1)
                           );
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        case T_PIECE:
            switch(piece.rotation) {
                case 0:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x, piece.y-1)
                           );
                case 1:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x+1, piece.y)
                           );
                case 2:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x, piece.y+1)
                           );
                case 3:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x-1, piece.y)
                           );
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        case S_PIECE:
            switch(piece.rotation) {
                case 0:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x+1, piece.y-1)
                           );
                case 1:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x+1, piece.y+1)
                           );
                case 2:
                    return(get_bit(board, piece.x-1, piece.y+1) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y)
                           );
                case 3:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x-1, piece.y+1)
                           );
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        case Z_PIECE:
            switch(piece.rotation) {
                case 0:
                    return(get_bit(board, piece.x-1, piece.y-1) |
                           get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y)
                           );
                case 1:
                    return(get_bit(board, piece.x+1, piece.y-1) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y-1)
                           );
                case 2:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x+1, piece.y+1)
                           );
                case 3:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x-1, piece.y+1)
                           );
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        case J_PIECE:
            switch(piece.rotation) {
                case 0:
                    return(get_bit(board, piece.x-1, piece.y-1) |
                           get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y)
                           );
                case 1:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x+1, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1)
                           );
                case 2:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x+1, piece.y+1)
                           );
                case 3:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x-1, piece.y+1)
                           );
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        case L_PIECE:
            switch(piece.rotation) {
                case 0:
                    return(get_bit(board, piece.x+1, piece.y-1) |
                           get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y)
                           );
                case 1:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x+1, piece.y+1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1)
                           );
                case 2:
                    return(get_bit(board, piece.x-1, piece.y) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x+1, piece.y) |
                           get_bit(board, piece.x-1, piece.y+1)
                           );
                case 3:
                    return(get_bit(board, piece.x, piece.y-1) |
                           get_bit(board, piece.x, piece.y) |
                           get_bit(board, piece.x, piece.y+1) |
                           get_bit(board, piece.x-1, piece.y-1)
                           );
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
        default:
            printf("Piece: %d\n", piece.type);
            ErrorExit("UnknownPiece");
    }
    return 1;
}

void make_tangible(board_t board, piece_t piece) {
    switch(piece.type) {
        case I_PIECE:
            switch(piece.rotation) {
                case 0:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x+2, piece.y);
                    break;
                case 1:
                    set_bit(board, piece.x+1, piece.y-1);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x+1, piece.y+1);
                    set_bit(board, piece.x+1, piece.y+2);
                    break;
                case 2:
                    set_bit(board, piece.x-1, piece.y+1);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x+1, piece.y+1);
                    set_bit(board, piece.x+2, piece.y+1);
                    break;
                case 3:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x, piece.y+2);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        case O_PIECE:
            switch(piece.rotation) {
                case 0:
                case 1:
                case 2:
                case 3:
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x+1, piece.y+1);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        case T_PIECE:
            switch(piece.rotation) {
                case 0:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x, piece.y-1);
                    break;
                case 1:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x+1, piece.y);
                    break;
                case 2:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    break;
                case 3:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x-1, piece.y);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        case S_PIECE:
            switch(piece.rotation) {
                case 0:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x+1, piece.y-1);
                    break;
                case 1:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x+1, piece.y+1);
                    break;
                case 2:
                    set_bit(board, piece.x-1, piece.y+1);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    break;
                case 3:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x-1, piece.y+1);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        case Z_PIECE:
            switch(piece.rotation) {
                case 0:
                    set_bit(board, piece.x-1, piece.y-1);
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    break;
                case 1:
                    set_bit(board, piece.x+1, piece.y-1);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y-1);
                    break;
                case 2:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x+1, piece.y+1);
                    break;
                case 3:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x-1, piece.y+1);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        case J_PIECE:
            switch(piece.rotation) {
                case 0:
                    set_bit(board, piece.x-1, piece.y-1);
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    break;
                case 1:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x+1, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    break;
                case 2:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x+1, piece.y+1);
                    break;
                case 3:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x-1, piece.y+1);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        case L_PIECE:
            switch(piece.rotation) {
                case 0:
                    set_bit(board, piece.x+1, piece.y-1);
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    break;
                case 1:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x+1, piece.y+1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    break;
                case 2:
                    set_bit(board, piece.x-1, piece.y);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x+1, piece.y);
                    set_bit(board, piece.x-1, piece.y+1);
                    break;
                case 3:
                    set_bit(board, piece.x, piece.y-1);
                    set_bit(board, piece.x, piece.y);
                    set_bit(board, piece.x, piece.y+1);
                    set_bit(board, piece.x-1, piece.y-1);
                    break;
                default:
                    printf("Rotation: %d\n", piece.rotation);
                    ErrorExit("InvalidRotation");
            }
            break;
        default:
            printf("Piece: %d\n", piece.type);
            ErrorExit("UnknownPiece");
    }
}

void reset(piece_t *piece) {
    static int rng;
    if(rng == 0) rng = clock();
    rng ^= rng << 6;
    rng ^= rng >> 9;
    *piece = (piece_t) { .x = 4, .y = 4, .rotation = 0, .type = rng%PIECE_COUNT };
}

void push(queue_t *queue, char key_code) {
        queue->content[queue->end] = key_code;
        queue->end = (queue->end+1) % 8;
        if(queue->end == queue->start) queue->start = (queue->start+1) % 8;
}

void clear(queue_t *queue) {
    queue->start = 0;
    queue->end = 0;
}

char pop(queue_t *queue) {
    if(queue->start == queue->end) return 0;
    queue->end = (queue->end-1+8) % 8;
    return queue->content[queue->end];
}

int contains(const queue_t *queue, char key_code) {
    for(int i = queue->start; i != queue->end ; i = (i+1)%8) {
        if(queue->content[i] == key_code) return 1;
    }
    return 0;
}

int empty(const queue_t *queue) { return queue->start == queue->end; }

int move_line(board_t board, int line) {
    if(line <= 0) return -1;
    if(line >= BOARD_HEIGHT) return -1;
    for(int x = 0; x < BOARD_WIDTH; ++x) {
        if(get_bit(board, x, line-1)) {
            set_bit(board, x, line);
        } else {
            clear_bit(board, x, line);
        }
    }
    return 0;
}

int check_for_full_lines(board_t board) {
    int clear_line;
    for(int y = 0; y < BOARD_HEIGHT; ++y) {
        clear_line = 1;
        for(int x = 0; x < BOARD_WIDTH; ++x) {
            if(!get_bit(board, x, y)) {
                clear_line = 0;
                break;
            }
        }
        if(clear_line) {
            for(int line = y; line > 0; --line) {
                move_line(board, line);
            }
            for(int x = 0; x < BOARD_WIDTH; ++x) {
                clear_bit(board, x, 0);
            }
        }
        if(clear_line && y >= BOARD_HEIGHT-2 ) { // TODO: Fail match after stuff above 20
        }
    }
    return 0;
}

void render(const board_t board, piece_t piece, term_size_t terminal_size) {
    for(int y = 0; y < BOARD_HEIGHT; ++y) {
        printf("\033[%d;%dH|", y+1, terminal_size.x/2-6+1);
        char line[BOARD_WIDTH+1] = {0};
        line[BOARD_WIDTH] = 0;
        for(int x = 0; x < BOARD_WIDTH; ++x) {
            line[x] = ' ';
            if(get_bit(board, x, y)) {
                line[x] = '#';
            }
        }
        printf("%s|", line);
    }
    // A up, B down, C right, D left
    printf("\033[%d;%dH", piece.y+1, terminal_size.x/2-5+piece.x+1);
    switch(piece.type) {
        case I_PIECE:
            printf("\033[%dm", PIECE_COLOURS[I_PIECE]); //  Cyan
            switch(piece.rotation) {
                case 2:
                    printf("\033[B");
                case 0:
                    printf("\033[D####");
                    break;
                case 1:
                    printf("\033[C");
                case 3:
                    printf("\033[A#\033[D\033[B#\033[D\033[B#\033[D\033[B#\033[D");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        case O_PIECE:
            printf("\033[%dm", PIECE_COLOURS[O_PIECE]); //  Yellow
            switch(piece.rotation) {
                case 0:
                case 1:
                case 2:
                case 3:
                    printf("##\033[2D\033[B##");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        case T_PIECE:
            printf("\033[%dm", PIECE_COLOURS[T_PIECE]); // Purple
            switch(piece.rotation) {
                case 0:
                    printf("\033[D###\033[2D\033[A#");
                    break;
                case 1:
                    printf("##\033[2D\033[A#\033[D\033[2B#");
                    break;
                case 2:
                    printf("\033[D###\033[2D\033[B#");
                    break;
                case 3:
                    printf("\033[D##\033[D\033[A#\033[D\033[2B#");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        case S_PIECE:
            printf("\033[%dm", PIECE_COLOURS[S_PIECE]); // Green
            switch(piece.rotation) {
                case 0:
                    printf("\033[D##\033[A\033[D##");
                    break;
                case 1:
                    printf("##\033[D\033[B#\033[2D\033[2A#");
                    break;
                case 2:
                    printf("##\033[3D\033[B##");
                    break;
                case 3:
                    printf("\033[D##\033[2D\033[A#\033[2B#");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        case Z_PIECE:
            printf("\033[%dm", PIECE_COLOURS[Z_PIECE]); // Red
            switch(piece.rotation) {
                case 0:
                    printf("\033[A\033[D##\033[D\033[B##");
                    break;
                case 1:
                    printf("##\033[D\033[A#\033[2D\033[2B#");
                    break;
                case 2:
                    printf("\033[D##\033[D\033[B##");
                    break;
                case 3:
                    printf("\033[D##\033[D\033[A#\033[2D\033[2B#");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        case J_PIECE:
            printf("\033[%dm", PIECE_COLOURS[J_PIECE]); // Blue
            switch(piece.rotation) {
                case 0:
                    printf("\033[D###\033[3D\033[A#");
                    break;
                case 1:
                    printf("\033[A##\033[B\033[2D#\033[B\033[D#");
                    break;
                case 2:
                    printf("\033[D###\033[D\033[B#");
                    break;
                case 3:
                    printf("\033[A#\033[B\033[D#\033[B\033[2D##");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        case L_PIECE:
            printf("\033[%dm", PIECE_COLOURS[L_PIECE]); // Pretend like its Orange, (White)
            switch(piece.rotation) {
                case 0:
                    printf("\033[D###\033[D\033[A#");
                    break;
                case 1:
                    printf("\033[A#\033[B\033[D#\033[B\033[D##");
                    break;
                case 2:
                    printf("\033[D###\033[3D\033[B#");
                    break;
                case 3:
                    printf("\033[D\033[A##\033[B\033[D#\033[B\033[D#");
                    break;
                default:
                    ErrorExit("InvalidRotation");
            }
            break;
        default:
            ErrorExit("UnknownPiece");
        }
    printf("\033[0m");
    printf("\033[1;1H");
}

queue_t queue = {0};

int main(VOID) {
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    // Get the standard input handle.
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if(hStdin == INVALID_HANDLE_VALUE) ErrorExit("GetStdHandle");

    // Save the current input mode, to be restored on exit.
    if(!GetConsoleMode(hStdin, &fdwSaveOldMode)) ErrorExit("GetConsoleMode");

    HANDLE hStdOut = NULL;
    CONSOLE_CURSOR_INFO curInfo;
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hStdOut, &curInfo);
    curInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hStdOut, &curInfo);

    // Enable the window and mouse input events.
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if(!SetConsoleMode(hStdin, fdwMode)) ErrorExit("SetConsoleMode");

    board_t board = {0};
    piece_t piece = {0};
    reset(&piece);
    term_size_t term_dimensions = { .x = 120, .y = 75};
    clock_t start_time = clock();
    int changed = 0;
    // Loop to read and handle the next 100 input events.
    while(1) {
        if(GetNumberOfConsoleInputEvents(hStdin, &cNumRead) && cNumRead > 0) {
            // Wait for the events.
            if(!ReadConsoleInput(hStdin,      // input buffer handle
                                 irInBuf,     // buffer to read into
                                 128,         // size of read buffer
                                 &cNumRead))  // number of records read
                             ErrorExit("ReadConsoleInput");

            // Dispatch the events to the appropriate handler.
            for(i = 0; i < cNumRead; i++) {
                switch(irInBuf[i].EventType) {
                    case KEY_EVENT: // keyboard input
                        KeyEventProc(irInBuf[i].Event.KeyEvent);
                        break;
                    case MOUSE_EVENT: // mouse input
                        MouseEventProc(irInBuf[i].Event.MouseEvent);
                        break;
                    case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                        WINDOW_BUFFER_SIZE_RECORD wbsr = irInBuf[i].Event.WindowBufferSizeEvent;
                        term_dimensions.x = wbsr.dwSize.X;
                        term_dimensions.y = wbsr.dwSize.Y;
                        break;
                    case FOCUS_EVENT:  // disregard focus events
                    case MENU_EVENT:   // disregard menu events
                        break;
                    default:
                        ErrorExit("Unknown event type");
                        break;
                }
            }
        }

        if(clock() > start_time + 500) {
            start_time = clock();
            ++piece.y; changed = 1;
            if(is_invalid(board, piece)) {
                --piece.y;
                make_tangible(board, piece);
                reset(&piece);
            }
        }
        for(char key_code = pop(&queue); key_code != 0; key_code = pop(&queue)) {
            printf("%c", key_code);
            switch(key_code) {
                case VK_LEFT:
                    --piece.x; changed = 1;
                    if(is_invalid(board, piece)) {
                        ++piece.x; changed = 0;
                    }
                    break;
                case VK_RIGHT:
                    ++piece.x; changed = 1;
                    if(is_invalid(board, piece)) {
                        --piece.x; changed = 0;
                    }
                    break;
                case VK_UP:
                    piece.rotation = (piece.rotation+1)%4; changed = 1;
                    if(is_invalid(board, piece)) {
                        piece.rotation = (piece.rotation-1+4)%4; changed = 0;
                    }
                    break;
                case VK_DOWN:
                    ++piece.y;
                    if(is_invalid(board, piece)) {
                        --piece.y;
                        make_tangible(board, piece);
                        reset(&piece);
                    }
            }
        }
        if(changed) {
            check_for_full_lines(board);
            render(board, piece, term_dimensions);
        }
        changed = 0;
    }

    // Restore input mode on exit.
    SetConsoleMode(hStdin, fdwSaveOldMode);
    return 0;
}

VOID ErrorExit(LPCSTR lpszMessage) {
    fprintf(stderr, "%s\n", lpszMessage);
    // Restore input mode on exit.
    SetConsoleMode(hStdin, fdwSaveOldMode);
    ExitProcess(0);
}

VOID KeyEventProc(KEY_EVENT_RECORD ker) {
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    char c = ker.wVirtualKeyCode;

    if(ker.bKeyDown) {
        if(c == 'Q') exit(0);
        push(&queue, c);
    }// else printf("key released\n");
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer) {
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
    printf("Mouse event: ");

    switch(mer.dwEventFlags) {
        case 0:
            if(mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                printf("left button press \n");
            } else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED) {
                printf("right button press \n");
            } else {
                printf("button press\n");
            } break;
        case DOUBLE_CLICK:
            printf("double click\n");
            break;
        case MOUSE_HWHEELED:
            printf("horizontal mouse wheel\n");
            break;
        case MOUSE_MOVED:
            printf("mouse moved\n");
            break;
        case MOUSE_WHEELED:
            printf("vertical mouse wheel\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
}

#ifdef NEVER
/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main() {
  enableRawMode();

  while (1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }

  return 0;
}
#endif
