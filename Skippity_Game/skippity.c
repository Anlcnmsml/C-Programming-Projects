#include <stdio.h> //Anýlcan Muþmul 23011622 
#include <stdlib.h> 	// video linki: https://1drv.ms/v/c/0dc605af07f1e3e6/Efpj-agD73xIgEYQzrEPWGIB1-R_zkV-fFbIy3mKURmEFg?e=NL04Df 
#include <time.h>
#include <string.h>

#define MAX_SIZE 20
#define MIN_SIZE 3
#define MAX_MOVES 1000 // Ýzlenecek maksimum hamle sayýsý

// Oyun durumu yapýsý
typedef struct {
    char **board;
    int size;
    int player1Pieces[5];
    int player2Pieces[5];
    int currentPlayer;
} GameState;

// Hamle yapýsý
typedef struct {
    int fromX, fromY, toX, toY;
    char capturedPiece;
    int midX, midY;
} Move;

// Hamle geçmiþi yapýsý
typedef struct {
    Move moves[MAX_MOVES];
    int moveCount;
    int currentMove;
} MoveHistory;

// Tahtayý baþlatma fonksiyonu
void initializeBoard(GameState *game, int size) {
    if (size % 2 != 0) {
        printf("Board size must be even for the middle squares to be empty.\n");
        exit(1);
    }
    game->size = size;
    game->board = (char **)malloc(size * sizeof(char *));
    
    char colors[] = {'A', 'B', 'C', 'D', 'E'}; // Harflerle temsil edilen renkler
    int colorCount = sizeof(colors) / sizeof(colors[0]);
    int totalCells = size * size - 4; // Dört orta boþ kare hariç
    int colorIndex = 0;
    int i, j;

    // Tüm tahtadaki pozisyonlarý tutmak için bir dizi oluþtur
    char *shuffledColors = (char *)malloc(totalCells * sizeof(char));
    for (i = 0; i < totalCells; i++) {
        shuffledColors[i] = colors[colorIndex];
        colorIndex = (colorIndex + 1) % colorCount;
    }

    // Diziyi karýþtýr
    for (i = totalCells - 1; i > 0; i--) {
        j = rand() % (i + 1);
        char temp = shuffledColors[i];
        shuffledColors[i] = shuffledColors[j];
        shuffledColors[j] = temp;
    }

    int shuffledIndex = 0;
    for (i = 0; i < size; i++) {
        game->board[i] = (char *)malloc(size * sizeof(char));
        for (j = 0; j < size; j++) {
            if ((i == size / 2 - 1 || i == size / 2) && (j == size / 2 - 1 || j == size / 2)) {
                game->board[i][j] = '.';  // Ortadaki boþ kareler
            } else {
                game->board[i][j] = shuffledColors[shuffledIndex++];
            }
        }
    }

    free(shuffledColors);

    for (i = 0; i < 5; i++) {
        game->player1Pieces[i] = 0;
        game->player2Pieces[i] = 0;
    }
    game->currentPlayer = 1;
}

// Tahtayý serbest býrakma fonksiyonu
void freeBoard(GameState *game) {
    int i;
    for (i = 0; i < game->size; i++) {
        free(game->board[i]);
    }
    free(game->board);
}

// Tahtayý yazdýrma fonksiyonu
void printBoard(GameState *game) {
    int i, j;
    printf(" ");
    for (j = 0; j < game->size; j++) {
        printf("___ ");
    }
    printf("\n");

    for (i = 0; i < game->size; i++) {
        printf("|");
        for (j = 0; j < game->size; j++) {
            printf(" %c |", game->board[i][j]);
        }
        printf("\n");

        printf(" ");
        for (j = 0; j < game->size; j++) {
            printf("___ ");
        }
        printf("\n");
    }
}

// Puanlarý yazdýrma fonksiyonu
void printScores(GameState *game) {
    printf("Player 1 Pieces: A: %d, B: %d, C: %d, D: %d, E: %d\n",
           game->player1Pieces[0], game->player1Pieces[1],
           game->player1Pieces[2], game->player1Pieces[3], game->player1Pieces[4]);
    printf("Player 2 Pieces: A: %d, B: %d, C: %d, D: %d, E: %d\n",
           game->player2Pieces[0], game->player2Pieces[1],
           game->player2Pieces[2], game->player2Pieces[3], game->player2Pieces[4]);
}

// Geçerli hamleyi kontrol etme fonksiyonu
int isValidMove(GameState *game, int fromX, int fromY, int toX, int toY) {
    // Koordinatlarýn tahta sýnýrlarý içinde olup olmadýðýný kontrol et
    if (fromX < 0 || fromX >= game->size || fromY < 0 || fromY >= game->size ||
        toX < 0 || toX >= game->size || toY < 0 || toY >= game->size) {
        return 0;
    }

    // Baþlangýç noktasýndaki pulun boþ olup olmadýðýný kontrol et
    if (game->board[fromX][fromY] == '.') {
        return 0;
    }

    // Hedef noktasýnýn boþ olup olmadýðýný kontrol et
    if (game->board[toX][toY] != '.') {
        return 0;
    }

    // Taþlarý sadece dikey veya yatay olarak hareket ettirme kontrolü ve bir kare atlama
    if ((abs(fromX - toX) == 2 && fromY == toY) || (fromX == toX && abs(fromY - toY) == 2)) {
        int midX = (fromX + toX) / 2;
        int midY = (fromY + toY) / 2;
        if (game->board[midX][midY] == '.') {
            return 0;
        }
        return 1;
    }

    // Geçerli bir hamle deðil
    return 0;
}

// Daha fazla atlama olup olmadýðýný kontrol etme fonksiyonu
int hasMoreJumps(GameState *game, int fromX, int fromY) {
    int dx[] = {-2, 2, 0, 0};
    int dy[] = {0, 0, -2, 2};
    int i;
    for (i = 0; i < 4; i++) {
        int toX = fromX + dx[i];
        int toY = fromY + dy[i];
        if (isValidMove(game, fromX, fromY, toX, toY)) {
            return 1;
        }
    }
    return 0;
}

// Hamleyi kaydetme fonksiyonu
void saveMove(MoveHistory *history, int fromX, int fromY, int toX, int toY, char capturedPiece, int midX, int midY) {
    if (history->moveCount < MAX_MOVES) {
        history->moves[history->moveCount].fromX = fromX;
        history->moves[history->moveCount].fromY = fromY;
        history->moves[history->moveCount].toX = toX;
        history->moves[history->moveCount].toY = toY;
        history->moves[history->moveCount].capturedPiece = capturedPiece;
        history->moves[history->moveCount].midX = midX;
        history->moves[history->moveCount].midY = midY;
        history->moveCount++;
        history->currentMove = history->moveCount; // Geri alma yýðýnýný sýfýrla
    }
}

// Hamleyi geri alma fonksiyonu
void undoMove(GameState *game, MoveHistory *history) {
    if (history->currentMove > 0) {
        history->currentMove--;
        Move move = history->moves[history->currentMove];
        game->board[move.fromX][move.fromY] = game->board[move.toX][move.toY]; // Taþý orijinal konumuna geri getir
        game->board[move.toX][move.toY] = '.';
        if (move.capturedPiece != '.') {
            game->board[move.midX][move.midY] = move.capturedPiece;
            if (game->currentPlayer == 2) { // Mevcut oyuncu 2 ise, önceki hamle oyuncu 1 tarafýndan yapýlmýþtýr
                game->player1Pieces[move.capturedPiece - 'A']--;
            } else { // Mevcut oyuncu 1 ise, önceki hamle oyuncu 2 tarafýndan yapýlmýþtýr
                game->player2Pieces[move.capturedPiece - 'A']--;
            }
        }
        game->currentPlayer = 3 - game->currentPlayer; // Önceki oyuncuya geri dön
        printBoard(game); // Geri alma sonrasý tahtayý yazdýr
        printScores(game); // Geri alma sonrasý puanlarý yazdýr
    }
}

// Hamle yapma fonksiyonu
void makeMove(GameState *game, MoveHistory *history, int fromX, int fromY, int toX, int toY) {
    char piece = game->board[fromX][fromY];
    game->board[fromX][fromY] = '.';
    game->board[toX][toY] = piece;

    // Atlanan taþlarý ele geçir ve puanlarý güncelle
    int midX = (fromX + toX) / 2;
    int midY = (fromY + toY) / 2;
    char capturedPiece = game->board[midX][midY];
    game->board[midX][midY] = '.';
    if (capturedPiece != '.') {
        if (game->currentPlayer == 1) {
            game->player1Pieces[capturedPiece - 'A']++;
        } else {
            game->player2Pieces[capturedPiece - 'A']++;
        }
    }

    saveMove(history, fromX, fromY, toX, toY, capturedPiece, midX, midY);

    printBoard(game); // Hamle sonrasý tahtayý yazdýr
    printScores(game); // Hamle sonrasý puanlarý yazdýr
}

// Bilgisayarýn hamlesi fonksiyonu
void computerMove(GameState *game, MoveHistory *history) {
    int fromX, fromY, toX, toY;
    int validMoveFound = 0;
    for (fromX = 0; fromX < game->size; fromX++) {
        for (fromY = 0; fromY < game->size; fromY++) {
            if (game->board[fromX][fromY] != '.') {
                for (toX = 0; toX < game->size; toX++) {
                    for (toY = 0; toY < game->size; toY++) {
                        if (isValidMove(game, fromX, fromY, toX, toY)) {
                            makeMove(game, history, fromX, fromY, toX, toY);
                            printf("Computer moved from (%d, %d) to (%d, %d)\n", fromX, fromY, toX, toY);
                            validMoveFound = 1;
                            break;
                        }
                    }
                    if (validMoveFound) break;
                }
            }
            if (validMoveFound) break;
        }
        if (validMoveFound) break;
    }
    game->currentPlayer = 1; // Bilgisayar hamlesinden sonra sýrayý oyuncu 1'e geçir
}

// Hamleyi yeniden yapma fonksiyonu
void redoMove(GameState *game, MoveHistory *history, int mode) {
    if (history->currentMove < history->moveCount) {
        Move move = history->moves[history->currentMove];
        game->board[move.toX][move.toY] = game->board[move.fromX][move.fromY];
        game->board[move.fromX][move.fromY] = '.';
        if (move.capturedPiece != '.') {
            game->board[move.midX][move.midY] = '.';
            if (game->currentPlayer == 1) {
                game->player1Pieces[move.capturedPiece - 'A']++;
            } else {
                game->player2Pieces[move.capturedPiece - 'A']++;
            }
        }
        history->currentMove++;
        printBoard(game); // Yeniden yapma sonrasý tahtayý yazdýr
        printScores(game); // Yeniden yapma sonrasý puanlarý yazdýr

        // Sýrayý deðiþtir
        game->currentPlayer = 3 - game->currentPlayer;

        // Eðer bilgisayara karþý oynanýyorsa ve sýra bilgisayara geçerse, bilgisayarýn hamlesi
        if (mode == 2 && game->currentPlayer == 2) {
            computerMove(game, history);
        }
    }
}

// Oyuncunun hamlesi fonksiyonu
void playerMove(GameState *game, MoveHistory *history, int *undoFlag, int mode) {
    int fromX, fromY, toX, toY;
    char choice;
    while (1) {
        printBoard(game);
        printf("Player %d's turn. Enter move (fromX fromY toX toY) or -1 to end: ", game->currentPlayer);
        scanf("%d %d %d %d", &fromX, &fromY, &toX, &toY);
        if (fromX == -1 && fromY == -1 && toX == -1 && toY == -1) {
            exit(0); // Oyunu bitir
        }
        if (isValidMove(game, fromX, fromY, toX, toY)) {
            makeMove(game, history, fromX, fromY, toX, toY);
            while (hasMoreJumps(game, toX, toY)) {
                printf("Multiple jumps possible. Enter next move (fromX fromY toX toY) or -1 to stop: ");
                scanf("%d %d %d %d", &fromX, &fromY, &toX, &toY);
                if (fromX == -1 || fromY == -1 || toX == -1 || toY == -1) {
                    break;
                }
                if (isValidMove(game, fromX, fromY, toX, toY)) {
                    makeMove(game, history, fromX, fromY, toX, toY);
                } else {
                    printf("Invalid move. Multiple jumps stopped.\n");
                    break;
                }
            }
            *undoFlag = 0; // Yeni bir hamleden sonra geri alma bayraðýný sýfýrla
            printf("Do you want to undo (u) the move, or continue (c)? ");
            scanf(" %c", &choice);
            if (choice == 'u') {
                game->currentPlayer = 3 - game->currentPlayer; // Geri almadan önce sýrayý deðiþtir
                undoMove(game, history);
                *undoFlag = 1; // Geri almadan sonra geri alma bayraðýný ayarla
                printf("Do you want to redo (r) the move, or continue (c)? ");
                scanf(" %c", &choice);
                if (choice == 'r') {
                    redoMove(game, history, mode);
                    *undoFlag = 0; // Yeniden yaptýktan sonra geri alma bayraðýný sýfýrla
                    break;
                }
            } else {
                game->currentPlayer = 3 - game->currentPlayer; // Sýrayý bir sonraki oyuncuya geçir
                break;
            }
        } else {
            printf("Invalid move. Try again.\n");
        }
    }
}


// Oyunun bitip bitmediðini kontrol etme fonksiyonu
void checkGameEnd(GameState *game) {
    int fromX, fromY, toX, toY;
    int noMoreMoves = 1;
    for (fromX = 0; fromX < game->size; fromX++) {
        for (fromY = 0; fromY < game->size; fromY++) {
            if (game->board[fromX][fromY] != '.') {
                for (toX = 0; toX < game->size; toX++) {
                    for (toY = 0; toY < game->size; toY++) {
                        if (isValidMove(game, fromX, fromY, toX, toY)) {
                            noMoreMoves = 0;
                            break;
                        }
                    }
                    if (!noMoreMoves) break;
                }
            }
            if (!noMoreMoves) break;
        }
        if (!noMoreMoves) break;
    }

    if (noMoreMoves) {
        printf("No valid moves left! Game over!\n");
        int player1Sets = game->player1Pieces[0];
        int player2Sets = game->player2Pieces[0];
        int i;
        for (i = 1; i < 5; i++) {
            if (game->player1Pieces[i] < player1Sets) {
                player1Sets = game->player1Pieces[i];
            }
            if (game->player2Pieces[i] < player2Sets) {
                player2Sets = game->player2Pieces[i];
            }
        }

        if (player1Sets > player2Sets) {
            printf("Player 1 wins with %d complete sets!\n", player1Sets);
        } else if (player2Sets > player1Sets) {
            printf("Player 2 wins with %d complete sets!\n", player2Sets);
        } else {
            // Tam setlerde beraberlik, toplam taþlara bak
            int player1TotalPieces = 0;
            int player2TotalPieces = 0;
            for (i = 0; i < 5; i++) {
                player1TotalPieces += game->player1Pieces[i];
                player2TotalPieces += game->player2Pieces[i];
            }

            if (player1TotalPieces > player2TotalPieces) {
                printf("Player 1 wins with %d total pieces!\n", player1TotalPieces);
            } else if (player2TotalPieces > player1TotalPieces) {
                printf("Player 2 wins with %d total pieces!\n", player2TotalPieces);
            } else {
                printf("It's a tie!\n");
            }
        }
        exit(0); // Oyunu bitir
    }
}

// Oyunu kaydetme fonksiyonu
void saveGame(GameState *game, const char *filename, int mode) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Failed to save game.\n");
        return;
    }

    int i, j;
    fprintf(file, "%d\n", game->size);
    for (i = 0; i < game->size; i++) {
        for (j = 0; j < game->size; j++) {
            fprintf(file, "%c", game->board[i][j]);
        }
        fprintf(file, "\n");
    }
    for (i = 0; i < 5; i++) {
        fprintf(file, "%d ", game->player1Pieces[i]);
    }
    fprintf(file, "\n");
    for (i = 0; i < 5; i++) {
        fprintf(file, "%d ", game->player2Pieces[i]);
    }
    fprintf(file, "\n");
    fprintf(file, "%d\n", game->currentPlayer);
    fprintf(file, "%d\n", mode); 

    fclose(file);
}

// Oyunu oynama fonksiyonu
void playGame(GameState *game, MoveHistory *history, int mode) {
    int paused = 0; // Oyun duraklatýldý mý?
    int undoFlag = 0; // Undo iþlemi yapýldý mý?

    while (1) {
        if (!paused) {
            if (mode == 1) { // Oyuncu vs Oyuncu
                playerMove(game, history, &undoFlag, mode);
            } else if (mode == 2) { // Oyuncu vs Bilgisayar
                if (game->currentPlayer == 1) {
                    playerMove(game, history, &undoFlag, mode);
                } else {
                    computerMove(game, history);
                }
            }
        }

        // Oyunun duraklatýlmasýný ve devam ettirilmesini yönet
        int userInput;
        do {
            printf("Press 0 to continue, 1 to pause: ");
            scanf("%d", &userInput);
        } while (userInput != 0 && userInput != 1);

        if (userInput == 1) {
            printf("Game paused. Returning to Main Menu.\n");
            saveGame(game, "saved_game.dat", mode);
            break;
        } else {
            paused = 0; // Oyuna devam et
        }

        // Geçerli hamle kalmadýðýný kontrol et
        checkGameEnd(game);
    }
}

// Oyunu yükleme fonksiyonu
void loadGame(GameState *game, const char *filename, int *mode) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to load game.\n");
        return;
    }

    int i, j;
    if (fscanf(file, "%d", &game->size) != 1) {
        printf("Error reading game size.\n");
        fclose(file);
        return;
    }

    game->board = (char **)malloc(game->size * sizeof(char *));
    for (i = 0; i < game->size; i++) {
        game->board[i] = (char *)malloc(game->size * sizeof(char));
        for (j = 0; j < game->size; j++) {
            if (fscanf(file, " %c", &game->board[i][j]) != 1) {
                printf("Error reading board data.\n");
                fclose(file);
                return;
            }
        }
    }

    for (i = 0; i < 5; i++) {
        if (fscanf(file, "%d", &game->player1Pieces[i]) != 1) {
            printf("Error reading player 1 pieces.\n");
            fclose(file);
            return;
        }
    }

    for (i = 0; i < 5; i++) {
        if (fscanf(file, "%d", &game->player2Pieces[i]) != 1) {
            printf("Error reading player 2 pieces.\n");
            fclose(file);
            return;
        }
    }

    if (fscanf(file, "%d", &game->currentPlayer) != 1) {
        printf("Error reading current player.\n");
        fclose(file);
        return;
    }

    
    if (fscanf(file, "%d", mode) != 1) {
        printf("Error reading game mode.\n");
        fclose(file);
        return;
    }

    fclose(file);
     // Yükleme sonrasý tahtayý yazdýr
    printScores(game); // Yükleme sonrasý puanlarý yazdýr
}

// Ana fonksiyon
int main() {
    srand(time(NULL));
    GameState game;
    MoveHistory history = {{0}, 0, 0};
    int boardSize;
    char filename[50];
    int choice, mode;

    printf("Welcome to Skippity!\n");
    while (1) {
        printf("Main Menu:\n1. New Game\n2. Load Game\n3. Exit\nYour choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Enter board size (min %d, max %d): ", MIN_SIZE, MAX_SIZE);
            scanf("%d", &boardSize);
            if (boardSize < MIN_SIZE || boardSize > MAX_SIZE) {
                printf("Invalid board size. Exiting...\n");
                break;
            }
            initializeBoard(&game, boardSize);

            printf("Choose mode:\n1. Player vs Player\n2. Player vs Computer\n3. Main Menu\nYour choice: ");
            scanf("%d", &mode);

            playGame(&game, &history, mode);
            freeBoard(&game);
        } else if (choice == 2) {
            printf("Enter filename to load: ");
            scanf("%s", filename);
            loadGame(&game, filename, &mode); 
            playGame(&game, &history, mode); 
            freeBoard(&game);
        } else if (choice == 3) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

