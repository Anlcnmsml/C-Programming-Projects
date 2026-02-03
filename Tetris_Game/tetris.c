#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>

/* GLOBAL DEÐÝÞKENLER */
int WIDTH, HEIGHT;
int **board;
int score = 0;
int highScore = 0;

/* TETROMINO YAPISI */
struct Tetromino {
    int shape[4][4];
    int color; // 0-6 arasý þekil ID'si
};

struct Tetromino currentPiece;
int currentX, currentY;

/* STANDART TETRIS ÞEKÝLLERÝ (7 ADET)
   I, J, L, O, S, T, Z
*/
int shapes[7][4][4] = {
    // I Þekli
    {{0, 0, 0, 0},
     {1, 1, 1, 1},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
    // J Þekli
    {{1, 0, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
    // L Þekli (Düzeltildi)
    {{0, 0, 1, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
    // O Þekli (Kare)
    {{1, 1, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
    // S Þekli
    {{0, 1, 1, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
    // T Þekli
    {{0, 1, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},
    // Z Þekli
    {{1, 1, 0, 0},
     {0, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}}
};

/* EKRAN TÝTREMESÝNÝ ENGELLEMEK ÝÇÝN YARDIMCI FONKSÝYONLAR */
void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void setCursorPosition(int x, int y) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (short)x, (short)y };
    SetConsoleCursorPosition(hOut, coord);
}

/* OYUN MANTIÐI */
int checkCollision(int x, int y, int rotation) {
    // Not: rotation parametresi þu an kullanýlmýyor çünkü
    // döndürme iþlemi currentPiece üzerinde yapýlýyor.
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (currentPiece.shape[i][j] != 0) {
                int newX = x + j;
                int newY = y + i;

                // Sýnýr kontrolü
                if (newX < 0 || newX >= WIDTH || newY >= HEIGHT) {
                    return 1;
                }
                
                // Zemin veya baþka parça kontrolü
                if (newY >= 0 && board[newY][newX] != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void placePiece() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (currentPiece.shape[i][j] != 0) {
                int boardX = currentX + j;
                int boardY = currentY + i;

                if (boardX >= 0 && boardX < WIDTH && boardY >= 0 && boardY < HEIGHT) {
                    // Renk bilgisi yerine basitçe 1 atýyoruz, 
                    // istersen piece.color + 1 yapabilirsin.
                    board[boardY][boardX] = 1; 
                }
            }
        }
    }
}

void clearLines() {
    int i, j, k;
    // Aþaðýdan yukarýya doðru kontrol ediyoruz
    for (i = HEIGHT - 1; i >= 0; i--) {
        int lineFilled = 1;
        for (j = 0; j < WIDTH; j++) {
            if (board[i][j] == 0) {
                lineFilled = 0;
                break;
            }
        }

        if (lineFilled) {
            // Satýrý sil ve üsttekileri aþaðý kaydýr
            for (k = i; k > 0; k--) {
                for (j = 0; j < WIDTH; j++) {
                    board[k][j] = board[k - 1][j];
                }
            }
            // En üst satýrý sýfýrla
            for (j = 0; j < WIDTH; j++) {
                board[0][j] = 0;
            }
            
            // Ayný satýrý tekrar kontrol etmek için i'yi artýrýyoruz 
            // (döngü i-- yaptýðý için ayný indexte kalmalý)
            i++; 
            
            score += 10;
            if (score > highScore) {
                highScore = score;
            }
        }
    }
}

void initTetromino(struct Tetromino *piece, int color) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            piece->shape[i][j] = shapes[color][i][j];
        }
    }
    piece->color = color;
}

void rotatePieceInternal() {
    struct Tetromino tempPiece = currentPiece;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            // Saat yönünde döndürme formülü
            currentPiece.shape[i][j] = tempPiece.shape[3 - j][i];
        }
    }
}

/* Akýllý Döndürme (Wall Kick) */
void rotate() {
    rotatePieceInternal();
    
    // Eðer döndürdükten sonra çarpýþma varsa kurtarmayý dene
    if (checkCollision(currentX, currentY, 0)) {
        // 1 birim saða dene
        if (!checkCollision(currentX + 1, currentY, 0)) {
            currentX++;
        } 
        // 1 birim sola dene
        else if (!checkCollision(currentX - 1, currentY, 0)) {
            currentX--;
        }
        // 2 birim sola dene (Uzun çubuk kenardaysa)
        else if (!checkCollision(currentX - 2, currentY, 0)) {
            currentX -= 2;
        }
        else {
            // Kurtaramadýk, eski haline geri getir (3 kere döndür = -90 derece)
            rotatePieceInternal();
            rotatePieceInternal();
            rotatePieceInternal();
        }
    }
}

void drawBoard() {
    // system("cls") YERÝNE setCursorPosition kullanýyoruz
    setCursorPosition(0, 0);

    int i, j;
    
    // Üst çerçeve
    for (j = 0; j < WIDTH + 2; j++) printf("#");
    printf("\n");

    for (i = 0; i < HEIGHT; i++) {
        printf("#"); // Sol duvar
        for (j = 0; j < WIDTH; j++) {
            // Önce düþen parçayý kontrol et
            int pieceX = j - currentX;
            int pieceY = i - currentY;
            int isCurrentPiece = 0;

            if (pieceX >= 0 && pieceX < 4 && pieceY >= 0 && pieceY < 4) {
                if (currentPiece.shape[pieceY][pieceX] != 0) {
                    isCurrentPiece = 1;
                }
            }

            if (isCurrentPiece) {
                printf("O"); // Düþen parça
            } else if (board[i][j] != 0) {
                printf("X"); // Yerleþmiþ parça
            } else {
                printf(" "); // Boþluk
            }
        }
        printf("#"); // Sað duvar
        printf("\n");
    }

    // Alt çerçeve
    for (j = 0; j < WIDTH + 2; j++) printf("#");
    printf("\n");

    printf("Score: %d\tHigh Score: %d\n", score, highScore);
    printf("Controls: A (Left), D (Right), S (Down), W (Rotate), P (Pause), Q (Quit)\n");
}

void initializeBoard() {
    int i, j;
    board = (int **)malloc(HEIGHT * sizeof(int *));
    for (i = 0; i < HEIGHT; i++) {
        board[i] = (int *)malloc(WIDTH * sizeof(int));
        for (j = 0; j < WIDTH; j++) {
            board[i][j] = 0;
        }
    }
}

void resetBoard() {
    int i, j;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            board[i][j] = 0;
        }
    }
}

void freeBoard() {
    int i;
    for (i = 0; i < HEIGHT; i++) {
        free(board[i]);
    }
    free(board);
}

void showMenu() {
    system("cls"); // Menüde temizleyebiliriz, oyun içi deðil
    printf("\n-------- TETRIS MENU --------\n");
    printf("1. Start Game\n");
    printf("2. How to Play\n");
    printf("3. Quit\n");
    printf("----------------------------\n");
}

void printHowToPlay() {
    system("cls");
    printf("\n-------- HOW TO PLAY --------\n");
    printf("Move Left: 'a'\n");
    printf("Move Right: 'd'\n");
    printf("Move Down: 's'\n");
    printf("Rotate: 'w'\n");
    printf("Pause/Resume: 'p'\n");
    printf("Quit: 'q'\n");
    printf("----------------------------\n");
    printf("Press any key to return menu...");
    getch();
}

int main() {
    srand((unsigned int)time(NULL));
    hideCursor(); // Ýmleci gizle

    int gravityDelay = 1000; // Baþlangýç düþme hýzý (ms)
    
    printf("Press Enter to start configuration...\n");
    getchar();

    printf("Enter width (e.g. 10): ");
    scanf("%d", &WIDTH);
    printf("Enter height (e.g. 20): ");
    scanf("%d", &HEIGHT);
    
    // Minimum boyut kontrolü
    if(WIDTH < 6) WIDTH = 10;
    if(HEIGHT < 10) HEIGHT = 20;

    initializeBoard();

    while (1) {
        int menuChoice;
        showMenu();
        printf("Enter your choice: ");
        scanf("%d", &menuChoice);

        switch (menuChoice) {
            case 1: {
                // OYUN BAÞLIYOR
                int gameOver = 0;
                resetBoard();
                score = 0;
                
                // Ýlk parçayý oluþtur
                currentX = WIDTH / 2 - 2;
                currentY = 0;
                initTetromino(&currentPiece, rand() % 7);

                clock_t lastGravityTime = clock();

                system("cls"); // Oyun baþlamadan önce ekraný bir kez temizle

                while (!gameOver) {
                    // 1. ZAMANLAMA VE YERÇEKÝMÝ
                    clock_t currentTime = clock();

                    if ((currentTime - lastGravityTime) > gravityDelay) {
                        if (!checkCollision(currentX, currentY + 1, 0)) {
                            currentY++;
                        } else {
                            // Parça yere deðdi
                            placePiece();
                            clearLines();
                            
                            // Yeni parça
                            currentX = WIDTH / 2 - 2;
                            currentY = 0;
                            initTetromino(&currentPiece, rand() % 7);
                            
                            // Yeni parça hemen çarpýþýyorsa oyun biter
                            if (checkCollision(currentX, currentY, 0)) {
                                drawBoard();
                                printf("\nGAME OVER!\n");
                                printf("Press any key to return to menu.\n");
                                getch();
                                gameOver = 1;
                            }
                        }
                        lastGravityTime = currentTime;
                    }

                    // 2. ÇÝZÝM
                    drawBoard();

                    // 3. KULLANICI GÝRÝÞÝ (Input)
                    if (kbhit()) {
                        char key = getch();
                        switch (key) {
                            case 'a':
                                if (!checkCollision(currentX - 1, currentY, 0)) currentX--;
                                break;
                            case 'd':
                                if (!checkCollision(currentX + 1, currentY, 0)) currentX++;
                                break;
                            case 's': // Hýzlý düþürme
                                if (!checkCollision(currentX, currentY + 1, 0)) {
                                    currentY++;
                                    // Hýzlý düþerken skoru artýrmak istersen: score++;
                                    lastGravityTime = clock(); // Süreyi sýfýrla ki hemen bir daha düþmesin
                                }
                                break;
                            case 'w':
                                rotate();
                                break;
                            case 'p':
                                setCursorPosition(0, HEIGHT + 4);
                                printf("PAUSED. Press 'p' to continue.");
                                while (getch() != 'p') {}
                                system("cls"); // Pause sonrasý temizlik
                                break;
                            case 'q':
                                gameOver = 1;
                                break;
                        }
                    }

                    Sleep(50); // Ýþlemciyi yormamak için kýsa bekleme
                }
                break;
            }
            case 2:
                printHowToPlay();
                break;
            case 3:
                freeBoard();
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice.\n");
                Sleep(1000);
        }
    }
    return 0;
}
