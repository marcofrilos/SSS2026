import signal
import sys
import time

# Riferimenti globali alle liste originali per garantire
# che la visualizzazione mostri sempre la Torre A a sinistra,
# la Torre B al centro e la Torre C a destra.
orig_A = None
orig_B = None
orig_C = None

total_moves_count = 0
total_disks_count = 0
frame_lines = 0
is_first_move = True
delay_ms = 0
start_time = 0.0


# Ripristino del cursore in caso di interruzione (Ctrl+C)
def handle_signal(sig, frame):
    sys.stdout.write("\033[?25h\n")
    sys.stdout.flush()
    sys.exit(0)


# Funzione di visualizzazione unificata in loco:
# 1. Vettori numerici
# 2. Grafica ASCII art
# 3. Contatore mosse dinamico
# 4. Tempo di runtime al millisecondo
def stamp(A, B, C, n, moves, elapsed_ms):
    # 1. Vettori numerici
    sys.stdout.write("A: " + " ".join(str(x) for x in A) + " \033[K\n")
    sys.stdout.write("B: " + " ".join(str(x) for x in B) + " \033[K\n")
    sys.stdout.write("C: " + " ".join(str(x) for x in C) + " \033[K\n\n")

    # 2. Rappresentazione grafica ASCII art
    peg_width = 2 * n + 3

    # Helper per disegnare disco o piolo vuoto
    def get_disk_or_rod(disk):
        if disk == 0:
            pad = (peg_width - 1) // 2
            return " " * pad + "|" + " " * pad
        else:
            # Disco con larghezza proporzionale al valore numerico
            disk_len = 2 * disk + 1
            disk_str = "[" + "=" * (2 * disk - 1) + "]"
            pad = (peg_width - disk_len) // 2
            return " " * pad + disk_str + " " * (peg_width - disk_len - pad)

    # Puntale superiore dei pioli
    pad = (peg_width - 1) // 2
    tip = " " * pad + "|" + " " * pad
    sys.stdout.write(f"{tip}   {tip}   {tip}\033[K\n")

    # Livelli da n-1 (cima) fino a 0 (terra)
    for lvl in range(n - 1, -1, -1):
        sys.stdout.write(
            f"{get_disk_or_rod(A[lvl])}   {get_disk_or_rod(B[lvl])}   {get_disk_or_rod(C[lvl])}\033[K\n"
        )

    # Basamento
    base = "=" * peg_width
    sys.stdout.write(f"{base}   {base}   {base}\033[K\n")

    # Etichette dei pioli
    def get_label(name):
        pad = (peg_width - len(name)) // 2
        return " " * pad + name + " " * (peg_width - len(name) - pad)

    sys.stdout.write(
        f"{get_label('A')}   {get_label('B')}   {get_label('C')}\033[K\n\n"
    )

    # 3. Contatore mosse dinamico
    sys.stdout.write(f"Mosse: {moves}\033[K\n")

    # 4. Tempo di runtime dinamico al millisecondo
    sys.stdout.write(f"Tempo di runtime: {elapsed_ms} ms\033[K\n")
    sys.stdout.flush()


# Sposta il disco in cima da src al primo alloggiamento libero su dst
def move_disk(src, dst, n):
    top_src = -1
    for i in range(n - 1, -1, -1):
        if src[i] != 0:
            top_src = i
            break
    top_dst = -1
    for i in range(n):
        if dst[i] == 0:
            top_dst = i
            break
    if top_src != -1 and top_dst != -1:
        dst[top_dst] = src[top_src]
        src[top_src] = 0


# Esegue una singola mossa, aggiorna i contatori e ridisegna in loco
def execute_move(src, dst):
    global is_first_move, total_moves_count

    if not is_first_move:
        sys.stdout.write(f"\033[{frame_lines}A\r")
        sys.stdout.flush()
    else:
        is_first_move = False

    move_disk(src, dst, total_disks_count)
    total_moves_count += 1

    elapsed_ms = int((time.perf_counter() - start_time) * 1000)

    stamp(orig_A, orig_B, orig_C, total_disks_count, total_moves_count, elapsed_ms)

    if delay_ms > 0:
        time.sleep(delay_ms / 1000.0)


# Algoritmo ricorsivo della Torre di Hanoi
def hanoi_solver(n, A, B, C):
    if n == 1:
        execute_move(A, C)
        return
    hanoi_solver(n - 1, A, C, B)
    execute_move(A, C)
    hanoi_solver(n - 1, B, A, C)


def main():
    global total_disks_count, orig_A, orig_B, orig_C, frame_lines, start_time

    signal.signal(signal.SIGINT, handle_signal)

    try:
        raw = input("numero dischi: ")
        n = int(raw)
        if n <= 0:
            print("Numero di dischi non valido.")
            return 1
    except (ValueError, EOFError):
        print("Numero di dischi non valido.")
        return 1

    total_disks_count = n

    # Allocazione e inizializzazione dei vettori
    # Indice 0 = terra, Indice n-1 = cima
    sys.stdout.write("\n\n")
    A = [
        n - i for i in range(n)
    ]  # A[0] = disco più grande (terra), A[n-1] = disco più piccolo (cima)
    B = [0] * n
    C = [0] * n

    orig_A = A
    orig_B = B
    orig_C = C

    # Calcolo righe totali occupate dal frame a video:
    # 3 (vettori A, B, C) + 1 (linea vuota) + 1 (punta pioli) + n (livelli)
    # + 1 (base) + 1 (etichette) + 1 (linea vuota) + 1 (mosse) + 1 (tempo runtime)
    frame_lines = n + 10

    # Visualizzazione dello stato iniziale
    stamp(A, B, C, n, 0, 0)

    # Richiesta di conferma per avviare la risoluzione
    try:
        answer = input("\nPremi 'y' per risolvere: ")
    except EOFError:
        answer = ""

    if answer.strip() not in ("y", "Y"):
        print("Risoluzione annullata.")
        return 0

    # Nasconde il cursore durante l'animazione
    sys.stdout.write("\033[?25l")
    sys.stdout.flush()

    # Risale per sovrascrivere lo stato iniziale ed eliminare la riga del prompt
    # frame_lines + 1 (linea vuota prima del prompt) + 1 (riga del prompt con newline)
    sys.stdout.write(f"\033[{frame_lines + 2}A\r\033[J")
    sys.stdout.flush()

    # Avvio cronometro
    start_time = time.perf_counter()

    # Risoluzione della torre di Hanoi
    hanoi_solver(n, A, B, C)

    # Ripristino del cursore a fine esecuzione
    sys.stdout.write("\033[?25h\n")
    sys.stdout.flush()

    return 0


if __name__ == "__main__":
    sys.exit(main())
