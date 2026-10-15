

#include <stdio.h>
#include <string.h>
#include <math.h>

#define DIGIT_NPM 10
#define JML_PROSES 5          
#define EPS 1e-9

static const char *DAFTAR_NPM[] = {
    "2517052047",  
    "2517052040", 
    "2517052038",
    "2517052048"
};

typedef struct {
    int       no;       /* nomor proses (1..5) */
    long long at;
    long long bt;
    int       priority;
    long long ct;
    long long tat;      /* CT - AT  */
    long long wt;       /* TAT - BT */
} Proses;

typedef struct {
    const char *nama;
    Proses      p[JML_PROSES];
    int         urutan[JML_PROSES];   /* indeks proses sesuai urutan eksekusi */
    double      avgWT;
    double      avgTAT;
    long long   selesai;              /* waktu selesai seluruh proses */
} HasilAlgoritma;

static int npmValid(const char *s) {
    if (strlen(s) != DIGIT_NPM) return 0;
    for (int i = 0; i < DIGIT_NPM; i++)
        if (s[i] < '0' || s[i] > '9') return 0;
    return 1;
}

/* mode 0 = FCFS, 1 = SJF, 2 = Priority (semua non-preemptive).
 * Tie-break: AT lebih kecil, lalu nomor proses lebih kecil. */
static int pilihProses(const Proses *p, const int *selesai,
                       long long waktu, int mode) {
    int best = -1;
    for (int i = 0; i < JML_PROSES; i++) {
        if (selesai[i] || p[i].at > waktu) continue;
        if (best == -1) { best = i; continue; }

        long long a, b;
        if (mode == 0)      { a = p[i].at;       b = p[best].at; }
        else if (mode == 1) { a = p[i].bt;       b = p[best].bt; }
        else                { a = p[i].priority; b = p[best].priority; }

        if (a < b || (a == b && p[i].at < p[best].at))
            best = i;
    }
    return best;
}

static void jalankan(HasilAlgoritma *h, const Proses *asal,
                     int mode, const char *nama) {
    int selesai[JML_PROSES] = {0};
    long long waktu = 0, totalWT = 0, totalTAT = 0;
    int sudah = 0;

    h->nama = nama;
    h->selesai = 0;
    for (int i = 0; i < JML_PROSES; i++) h->p[i] = asal[i];

    while (sudah < JML_PROSES) {
        int k = pilihProses(h->p, selesai, waktu, mode);
        if (k == -1) { /* CPU idle: lompat ke kedatangan terdekat */
            long long minAT = -1;
            for (int i = 0; i < JML_PROSES; i++)
                if (!selesai[i] && (minAT == -1 || h->p[i].at < minAT))
                    minAT = h->p[i].at;
            waktu = minAT;
            continue;
        }
        waktu += h->p[k].bt;
        h->p[k].ct  = waktu;
        h->p[k].tat = h->p[k].ct - h->p[k].at;
        h->p[k].wt  = h->p[k].tat - h->p[k].bt;
        selesai[k] = 1;
        h->urutan[sudah++] = k;
    }

    for (int i = 0; i < JML_PROSES; i++) {
        totalWT  += h->p[i].wt;
        totalTAT += h->p[i].tat;
        if (h->p[i].ct > h->selesai) h->selesai = h->p[i].ct;
    }
    h->avgWT  = (double)totalWT  / JML_PROSES;
    h->avgTAT = (double)totalTAT / JML_PROSES;
}

/* Satu tabel untuk satu algoritma (berisi 5 proses dari NPM tersebut) */
static void cetakTabel(const HasilAlgoritma *h) {
    printf("\n--- %s ---\n", h->nama);
    printf("Urutan eksekusi: ");
    for (int i = 0; i < JML_PROSES; i++) {
        printf("P%d", h->p[h->urutan[i]].no);
        if (i < JML_PROSES - 1) printf(" -> ");
    }
    printf("\n");
    printf("%-6s %-4s %-4s %-4s %-6s %-6s %-6s\n",
           "Proses", "AT", "BT", "Pri", "CT", "TAT", "WT");
    printf("------------------------------------------\n");
    /* baris tabel diurutkan sesuai urutan eksekusi algoritma */
    for (int i = 0; i < JML_PROSES; i++) {
        const Proses *p = &h->p[h->urutan[i]];
        printf("P%-5d %-4lld %-4lld %-4d %-6lld %-6lld %-6lld\n",
               p->no, p->at, p->bt, p->priority, p->ct, p->tat, p->wt);
    }
    printf("------------------------------------------\n");
    printf("Average Waiting Time    : %.2f\n", h->avgWT);
    printf("Average Turnaround Time : %.2f\n", h->avgTAT);
    printf("Waktu selesai keseluruhan: %lld\n", h->selesai);
}

static double nilai(const HasilAlgoritma *h, int metrik) {
    if (metrik == 0) return h->avgWT;
    if (metrik == 1) return h->avgTAT;
    return (double)h->selesai;
}

/* Cetak algoritma terbaik (terkecil) / terburuk (terbesar); tangani seri */
static void cetakExtrem(const HasilAlgoritma *h, int metrik,
                        int cariTerkecil, const char *label) {
    double target = nilai(&h[0], metrik);
    for (int i = 1; i < 3; i++) {
        double v = nilai(&h[i], metrik);
        if (cariTerkecil ? (v < target) : (v > target)) target = v;
    }
    printf("%-48s: ", label);
    int pertama = 1;
    for (int i = 0; i < 3; i++) {
        if (fabs(nilai(&h[i], metrik) - target) < EPS) {
            printf("%s%s", pertama ? "" : ", ", h[i].nama);
            pertama = 0;
        }
    }
    printf(" (%.2f)\n", target);
}

int main(void) {
    int jumlahNPM = (int)(sizeof(DAFTAR_NPM) / sizeof(DAFTAR_NPM[0]));

    printf("=== SIMULASI CPU SCHEDULING BERDASARKAN NPM ===\n");

    for (int k = 0; k < jumlahNPM; k++) {
        const char *npm = DAFTAR_NPM[k];
        Proses data[JML_PROSES];

        if (!npmValid(npm)) {
            printf("NPM tidak valid (harus 10 digit angka): %s\n", npm);
            return 1;
        }

        /* Pecah NPM: digit 0-4 = AT, digit 5-9 = BT, dipasangkan per digit */
        for (int i = 0; i < JML_PROSES; i++) {
            data[i].no       = i + 1;
            data[i].at       = npm[i] - '0';
            data[i].bt       = npm[JML_PROSES + i] - '0';
            data[i].priority = i + 1;
        }

        printf("\n############################################################\n");
        printf("NPM %s\n", npm);
        printf("############################################################\n");

        /* Tampilan vertikal digit NPM */
        printf("\nAT (5 digit pertama):\n");
        for (int i = 0; i < JML_PROSES; i++) printf("%lld\n", data[i].at);
        printf("\nBT (5 digit terakhir):\n");
        for (int i = 0; i < JML_PROSES; i++) printf("%lld\n", data[i].bt);

        printf("\nProses yang terbentuk (data sama untuk ketiga algoritma):\n");
        printf("%-6s %-4s %-4s %-4s\n", "Proses", "AT", "BT", "Pri");
        for (int i = 0; i < JML_PROSES; i++)
            printf("P%-5d %-4lld %-4lld %-4d\n",
                   data[i].no, data[i].at, data[i].bt, data[i].priority);
        printf("(Priority: angka lebih kecil = prioritas lebih tinggi)\n");

        HasilAlgoritma hasil[3];
        jalankan(&hasil[0], data, 0, "FCFS");
        jalankan(&hasil[1], data, 1, "SJF");
        jalankan(&hasil[2], data, 2, "Priority");

        for (int i = 0; i < 3; i++) cetakTabel(&hasil[i]);

        printf("\n=== PERBANDINGAN (NPM %s) ===\n", npm);
        printf("%-10s %-20s %-22s %-20s\n", "Algoritma",
               "Avg Waiting Time", "Avg Turnaround Time", "Selesai Keseluruhan");
        for (int i = 0; i < 3; i++)
            printf("%-10s %-20.2f %-22.2f %-20lld\n", hasil[i].nama,
                   hasil[i].avgWT, hasil[i].avgTAT, hasil[i].selesai);
        printf("\n");
        cetakExtrem(hasil, 0, 1, "Terbaik  berdasarkan Average Waiting Time");
        cetakExtrem(hasil, 0, 0, "Terburuk berdasarkan Average Waiting Time");
        cetakExtrem(hasil, 1, 1, "Terbaik  berdasarkan Average Turnaround Time");
        cetakExtrem(hasil, 1, 0, "Terburuk berdasarkan Average Turnaround Time");
        cetakExtrem(hasil, 2, 1, "Penyelesaian keseluruhan tercepat");
        cetakExtrem(hasil, 2, 0, "Penyelesaian keseluruhan paling lambat");
    }

    return 0;
}