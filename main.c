#include <stdio.h>

#define N 5   /* jumlah proses per NPM */

/* Data proses disimpan dalam array (indeks 0 = P1, 1 = P2, dst) */
int at[N], bt[N], pri[N];     /* data awal */
int ct[N], tat[N], wt[N];     /* hasil perhitungan */
int urutan[N];                /* urutan eksekusi */

/* Hasil rata-rata tiap algoritma (0 = FCFS, 1 = SJF, 2 = Priority) */
double avgWT[3], avgTAT[3];

/* Mengambil nilai yang dipakai untuk memilih proses, sesuai algoritma */
int nilai(int i, int mode) {
    if (mode == 0) return at[i];    /* FCFS     : yang datang duluan */
    if (mode == 1) return bt[i];    /* SJF      : burst time terkecil */
    return pri[i];                  /* Priority : angka prioritas terkecil */
}

/* Menjalankan satu algoritma */
void jalankan(int mode, char nama[]) {
    int selesai[N] = {0};   /* 0 = belum selesai, 1 = sudah selesai */
    int waktu = 0;          /* waktu sekarang */
    int jumlah = 0;         /* jumlah proses yang sudah selesai */

    while (jumlah < N) {
        /* Cari proses yang sudah datang dan nilainya paling kecil */
        int pilih = -1;
        for (int i = 0; i < N; i++) {
            if (selesai[i] == 1 || at[i] > waktu) continue;

            if (pilih == -1 || nilai(i, mode) < nilai(pilih, mode)) {
                pilih = i;
            } else if (nilai(i, mode) == nilai(pilih, mode) && at[i] < at[pilih]) {
                pilih = i;   /* kalau seri, pilih yang datang lebih dulu */
            }
        }

        /* Belum ada proses yang datang: CPU menunggu (idle) */
        if (pilih == -1) {
            waktu++;
            continue;
        }

        /* Jalankan proses yang dipilih */
        waktu = waktu + bt[pilih];
        ct[pilih]  = waktu;                  /* Completion Time */
        tat[pilih] = ct[pilih] - at[pilih];  /* Turnaround Time */
        wt[pilih]  = tat[pilih] - bt[pilih]; /* Waiting Time */
        selesai[pilih] = 1;
        urutan[jumlah] = pilih;
        jumlah++;
    }

    /* Hitung rata-rata */
    int totalWT = 0, totalTAT = 0;
    for (int i = 0; i < N; i++) {
        totalWT  += wt[i];
        totalTAT += tat[i];
    }
    avgWT[mode]  = (double)totalWT / N;
    avgTAT[mode] = (double)totalTAT / N;

    /* Cetak tabel hasil */
    printf("\n--- %s ---\n", nama);
    printf("Urutan: ");
    for (int i = 0; i < N; i++) {
        printf("P%d", urutan[i] + 1);
        if (i < N - 1) printf(" -> ");
    }
    printf("\n");

    printf("Proses  AT  BT  Pri  CT  TAT  WT\n");
    for (int i = 0; i < N; i++) {
        int p = urutan[i];
        printf("P%-6d %-3d %-3d %-4d %-3d %-4d %d\n",
               p + 1, at[p], bt[p], pri[p], ct[p], tat[p], wt[p]);
    }
    printf("Rata-rata WT  : %.2f\n", avgWT[mode]);
    printf("Rata-rata TAT : %.2f\n", avgTAT[mode]);
}

int main(void) {
    /* Daftar NPM kelompok (jangan lupa koma di antara NPM) */
    char *daftarNPM[] = {
        "2517052047"
        // "2517052040",
        // "2517052038",
        // "2517052048"
    };
    int jumlahNPM = 4;

    for (int k = 0; k < jumlahNPM; k++) {
        char *npm = daftarNPM[k];

        printf("\n==============================\n");
        printf("NPM %s\n", npm);
        printf("==============================\n");

        /* Pecah NPM menjadi AT dan BT.
         * npm[i] - '0' mengubah karakter angka menjadi bilangan. */
        for (int i = 0; i < N; i++) {
            at[i]  = npm[i] - '0';        /* digit 1-5  */
            bt[i]  = npm[i + N] - '0';    /* digit 6-10 */
            pri[i] = i + 1;
        }

        printf("\nProses  AT  BT  Pri\n");
        for (int i = 0; i < N; i++)
            printf("P%-6d %-3d %-3d %d\n", i + 1, at[i], bt[i], pri[i]);

        jalankan(0, "FCFS");
        jalankan(1, "SJF");
        jalankan(2, "Priority");

        printf("\n=== PERBANDINGAN ===\n");
        printf("Algoritma   Avg WT   Avg TAT\n");
        printf("FCFS        %-8.2f %.2f\n", avgWT[0], avgTAT[0]);
        printf("SJF         %-8.2f %.2f\n", avgWT[1], avgTAT[1]);
        printf("Priority    %-8.2f %.2f\n", avgWT[2], avgTAT[2]);
    }

    return 0;
}