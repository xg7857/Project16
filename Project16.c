#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_LEN 32

typedef struct {
    int  id;
    char name[NAME_LEN];
    char gender;
    int  korean, english, math, total;
} Student;

static Student *orig = NULL;
static Student *sorted = NULL;
static int n = 0;

static long long cmp_count = 0;

static int cmp_student(const Student *a, const Student *b) {
    int r = strcmp(a->name, b->name);
    cmp_count++;
    if (r != 0) return r;
    return (a->id > b->id) - (a->id < b->id);
}

static int load_csv(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    char line[256];
    int cap = 1024;
    orig = malloc(sizeof(Student) * cap);
    fgets(line, sizeof line, fp);
    while (fgets(line, sizeof line, fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (!line[0]) continue;
        if (n == cap) { cap *= 2; orig = realloc(orig, sizeof(Student) * cap); }
        Student *s = &orig[n];
        char *t = strtok(line, ",");           s->id = atoi(t);
        t = strtok(NULL, ",");                 strncpy(s->name, t, NAME_LEN - 1); s->name[NAME_LEN - 1] = '\0';
        t = strtok(NULL, ",");                 s->gender = t[0];
        t = strtok(NULL, ",");                 s->korean = atoi(t);
        t = strtok(NULL, ",");                 s->english = atoi(t);
        t = strtok(NULL, ",");                 s->math = atoi(t);
        s->total = s->korean + s->english + s->math;
        n++;
    }
    fclose(fp);
    return 1;
}

static void swap_st(Student *a, Student *b) { Student t = *a; *a = *b; *b = t; }

static void quick_sort(Student *a, int lo, int hi) {
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;

        if (cmp_student(&a[mid], &a[lo]) < 0) swap_st(&a[mid], &a[lo]);
        if (cmp_student(&a[hi],  &a[lo]) < 0) swap_st(&a[hi],  &a[lo]);
        if (cmp_student(&a[hi],  &a[mid]) < 0) swap_st(&a[hi], &a[mid]);
        Student pivot = a[mid];
        int i = lo - 1, j = hi + 1;
        for (;;) {
            do { i++; } while (cmp_student(&a[i], &pivot) < 0);
            do { j--; } while (cmp_student(&a[j], &pivot) > 0);
            if (i >= j) break;
            swap_st(&a[i], &a[j]);
        }

        if (j - lo < hi - (j + 1)) { quick_sort(a, lo, j); lo = j + 1; }
        else                        { quick_sort(a, j + 1, hi); hi = j; }
    }
}

static int sequential_search(const char *key, int *res) {
    int cnt = 0;
    for (int i = 0; i < n; i++) {
        cmp_count++;
        if (strcmp(key, orig[i].name) == 0) res[cnt++] = i;
    }
    return cnt;
}

static int binary_search(const char *key, int *res) {
    int lo = 0, hi = n - 1, found = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int r = strcmp(key, sorted[mid].name);
        cmp_count++;
        if (r == 0) { found = mid; break; }
        if (r < 0) hi = mid - 1; else lo = mid + 1;
    }
    if (found < 0) return 0;
    int l = found, h = found;
    while (l - 1 >= 0) { cmp_count++; if (strcmp(key, sorted[l - 1].name) == 0) l--; else break; }
    while (h + 1 < n)  { cmp_count++; if (strcmp(key, sorted[h + 1].name) == 0) h++; else break; }
    int cnt = 0;
    for (int i = l; i <= h; i++) res[cnt++] = i;
    return cnt;
}

static void print_result(const Student *s) {
    printf("Search Result\n");
    printf("ID      : %d\n", s->id);
    printf("Name    : %s\n", s->name);
    printf("Korean  : %d\n", s->korean);
    printf("English : %d\n", s->english);
    printf("Math    : %d\n", s->math);
    printf("Total   : %d\n\n", s->total);
}

static unsigned long long rng_state = 20261126ULL;
static unsigned rnd(void) {
    rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned)(rng_state >> 33);
}

static void make_missing_name(char *buf) { sprintf(buf, "Zz%u", rnd() % 100000); }

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "dataset_id_ascending.csv";
    if (!load_csv(path)) { fprintf(stderr, "Cannot open CSV file: %s\n", path); return 1; }

    sorted = malloc(sizeof(Student) * n);
    int *res_a = malloc(sizeof(int) * n), *res_b = malloc(sizeof(int) * n);

    memcpy(sorted, orig, sizeof(Student) * n);
    cmp_count = 0;
    quick_sort(sorted, 0, n - 1);
    long long sort_cmp = cmp_count;

    const char *demo[] = { "Liam", "Ryan", "Nobody" };
    int dn = (argc > 2) ? argc - 2 : 3;
    printf("[Data] %d students loaded, Quick Sort comparisons = %lld\n\n", n, sort_cmp);
    for (int d = 0; d < dn; d++) {
        const char *key = (argc > 2) ? argv[2 + d] : demo[d];
        int ca = sequential_search(key, res_a), cb = binary_search(key, res_b);
        int same = (ca == cb);
        for (int i = 0; same && i < ca; i++)
            if (orig[res_a[i]].id != sorted[res_b[i]].id) same = 0;
        printf("Query: \"%s\"  (matches: sequential=%d, binary=%d, identical=%s)\n",
               key, ca, cb, same ? "yes" : "NO");
        if (ca == 0) printf("Not found\n\n");
        for (int i = 0; i < ca; i++) print_result(&orig[res_a[i]]);
    }

    int Kmax = 20000;
    char (*list)[NAME_LEN] = malloc(NAME_LEN * Kmax);
    for (int i = 0; i < Kmax; i++) {
        if (i % 2 == 0) strcpy(list[i], orig[rnd() % n].name);
        else            make_missing_name(list[i]);
    }

    long long *cum_seq = malloc(sizeof(long long) * (Kmax + 1));
    long long *cum_bin = malloc(sizeof(long long) * (Kmax + 1));
    cum_seq[0] = 0; cum_bin[0] = sort_cmp;
    for (int i = 0; i < Kmax; i++) {
        cmp_count = 0; int ca = sequential_search(list[i], res_a); cum_seq[i + 1] = cum_seq[i] + cmp_count;
        cmp_count = 0; int cb = binary_search(list[i], res_b);     cum_bin[i + 1] = cum_bin[i] + cmp_count;
        if (ca != cb) { fprintf(stderr, "Result mismatch: %s\n", list[i]); return 1; }
    }

    int Ks[] = {1, 5, 10, 15, 20, 25, 30, 40, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    printf("Comparison Results\n\n");
    printf("%-8s %-16s %-16s %s\n", "K", "Sequential", "Quick + Binary", "Winner");
    printf("------------------------------------------------------\n");
    for (size_t i = 0; i < sizeof Ks / sizeof *Ks; i++) {
        int K = Ks[i];
        const char *w = (cum_seq[K] < cum_bin[K]) ? "Sequential" : (cum_seq[K] > cum_bin[K]) ? "Quick+Binary" : "Tie";
        printf("%-8d %-16lld %-16lld %s\n", K, cum_seq[K], cum_bin[K], w);
    }

    for (int K = 1; K <= Kmax; K++) {
        if (cum_bin[K] < cum_seq[K]) {
            printf("\nCrossover: Quick + Binary becomes cheaper at K = %d (Seq=%lld, Bin=%lld)\n",
                   K, cum_seq[K], cum_bin[K]);
            printf("At K = %d: Seq=%lld, Bin=%lld (Sequential still cheaper)\n",
                   K - 1, cum_seq[K - 1], cum_bin[K - 1]);
            break;
        }
    }
    printf("Average per search: Sequential = %.1f, Binary = %.2f (K=%d)\n",
           (double)cum_seq[Kmax] / Kmax, (double)(cum_bin[Kmax] - sort_cmp) / Kmax, Kmax);
    return 0;
}
