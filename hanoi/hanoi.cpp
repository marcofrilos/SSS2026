#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <csignal>

using namespace std;

// Puntatori globali ai vettori originali per garantire
// che la visualizzazione mostri sempre la Torre A a sinistra,
// la Torre B al centro e la Torre C a destra.
int* origA = nullptr;
int* origB = nullptr;
int* origC = nullptr;

int totalMovesCount = 0;
int totalDisksCount = 0;
int frameLines = 0;
bool isFirstMove = true;
int delayMs = 0;
chrono::steady_clock::time_point startTime;

// Ripristino del cursore in caso di interruzione (Ctrl+C)
void handleSignal(int) {
    cout << "\033[?25h" << endl;
    exit(0);
}

// Funzione di visualizzazione unificata in loco:
// 1. Vettori numerici
// 2. Grafica ASCII art
// 3. Contatore mosse dinamico
// 4. Tempo di runtime al millisecondo
void stamp(int* A, int* B, int* C, int n, int moves, long long elapsedMs) {
    // 1. Vettori numerici
    cout << "A: ";
    for (int i = 0; i < n; i++) cout << A[i] << " ";
    cout << "\033[K\n";

    cout << "B: ";
    for (int i = 0; i < n; i++) cout << B[i] << " ";
    cout << "\033[K\n";

    cout << "C: ";
    for (int i = 0; i < n; i++) cout << C[i] << " ";
    cout << "\033[K\n\n";

    // 2. Rappresentazione grafica ASCII art
    int pegWidth = 2 * n + 3;

    // Helper per disegnare disco o piolo vuoto
    auto getDiskOrRod = [pegWidth](int disk) -> string {
        if (disk == 0) {
            int pad = (pegWidth - 1) / 2;
            return string(pad, ' ') + "|" + string(pad, ' ');
        } else {
            // Disco con larghezza proporzionale al valore numerico
            int diskLen = 2 * disk + 1;
            string diskStr = "[" + string(2 * disk - 1, '=') + "]";
            int pad = (pegWidth - diskLen) / 2;
            return string(pad, ' ') + diskStr + string(pegWidth - diskLen - pad, ' ');
        }
    };

    // Puntale superiore dei pioli
    int pad = (pegWidth - 1) / 2;
    string tip = string(pad, ' ') + "|" + string(pad, ' ');
    cout << tip << "   " << tip << "   " << tip << "\033[K\n";

    // Livelli da n-1 (cima) fino a 0 (terra)
    for (int lvl = n - 1; lvl >= 0; lvl--) {
        cout << getDiskOrRod(A[lvl]) << "   "
             << getDiskOrRod(B[lvl]) << "   "
             << getDiskOrRod(C[lvl]) << "\033[K\n";
    }

    // Basamento
    string base(pegWidth, '=');
    cout << base << "   " << base << "   " << base << "\033[K\n";

    // Etichette dei pioli
    auto getLabel = [pegWidth](const string& name) -> string {
        int pad = (pegWidth - (int)name.length()) / 2;
        return string(pad, ' ') + name + string(pegWidth - (int)name.length() - pad, ' ');
    };
    cout << getLabel("A") << "   " << getLabel("B") << "   " << getLabel("C") << "\033[K\n\n";

    // 3. Contatore mosse dinamico
    cout << "Mosse: " << moves << "\033[K\n";

    // 4. Tempo di runtime dinamico al millisecondo
    cout << "Tempo di runtime: " << elapsedMs << " ms\033[K\n";
    cout << flush;
}

// Sposta il disco in cima da src al primo alloggiamento libero su dst
void moveDisk(int* src, int* dst, int n) {
    int topSrc = -1;
    for (int i = n - 1; i >= 0; i--) {
        if (src[i] != 0) {
            topSrc = i;
            break;
        }
    }
    int topDst = -1;
    for (int i = 0; i < n; i++) {
        if (dst[i] == 0) {
            topDst = i;
            break;
        }
    }
    if (topSrc != -1 && topDst != -1) {
        dst[topDst] = src[topSrc];
        src[topSrc] = 0;
    }
}

// Esegue una singola mossa, aggiorna i contatori e ridisegna in loco
void executeMove(int* src, int* dst) {
    if (!isFirstMove) {
        cout << "\033[" << frameLines << "A\r" << flush;
    } else {
        isFirstMove = false;
    }

    moveDisk(src, dst, totalDisksCount);
    totalMovesCount++;

    auto now = chrono::steady_clock::now();
    long long elapsedMs = chrono::duration_cast<chrono::milliseconds>(now - startTime).count();

    stamp(origA, origB, origC, totalDisksCount, totalMovesCount, elapsedMs);

    if (delayMs > 0) {
        this_thread::sleep_for(chrono::milliseconds(delayMs));
    }
}

// Algoritmo ricorsivo della Torre di Hanoi
void hanoiSolver(int n, int* A, int* B, int* C) {
    if (n == 1) {
        executeMove(A, C);
        return;
    }
    hanoiSolver(n - 1, A, C, B);
    executeMove(A, C);
    hanoiSolver(n - 1, B, A, C);
}

int main() {
    signal(SIGINT, handleSignal);

    int n;
    cout << "numero dischi: ";
    if (!(cin >> n) || n <= 0) {
        cout << "Numero di dischi non valido." << endl;
        return 1;
    }

    totalDisksCount = n;

    // Allocazione e inizializzazione dei vettori
    // Indice 0 = terra, Indice n-1 = cima
    cout << "\n" << endl;
    int* A = new int[n];
    int* B = new int[n]{};
    int* C = new int[n]{};
    for (int i = 0; i < n; i++) {
        A[i] = n - i; // A[0] = disco più grande (terra), A[n-1] = disco più piccolo (cima)
    }

    origA = A;
    origB = B;
    origC = C;

    // Calcolo righe totali occupate dal frame a video:
    // 3 (vettori A, B, C) + 1 (linea vuota) + 1 (punta pioli) + n (livelli)
    // + 1 (base) + 1 (etichette) + 1 (linea vuota) + 1 (mosse) + 1 (tempo runtime)
    frameLines = n + 10;

    // Visualizzazione dello stato iniziale
    stamp(A, B, C, n, 0, 0);

    // Richiesta di conferma per avviare la risoluzione
    cout << "\nPremi 'y' per risolvere: ";
    char answer;
    if (!(cin >> answer) || (answer != 'y' && answer != 'Y')) {
        cout << "Risoluzione annullata." << endl;
        delete[] A;
        delete[] B;
        delete[] C;
        return 0;
    }

    // Nasconde il cursore durante l'animazione
    cout << "\033[?25l" << flush;

    // Risale per sovrascrivere lo stato iniziale ed eliminare la riga del prompt
    // frameLines + 1 (linea vuota prima del prompt) + 1 (riga del prompt con newline)
    cout << "\033[" << (frameLines + 2) << "A\r\033[J" << flush;

    // Avvio cronometro
    startTime = chrono::steady_clock::now();

    // Risoluzione della torre di Hanoi
    hanoiSolver(n, A, B, C);

    // Ripristino del cursore a fine esecuzione
    cout << "\033[?25h" << endl;

    // Deallocazione memoria
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
