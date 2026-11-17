#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>

#define MAX_WORDS 8
#define MAX_LEN 32

static int digit[26];
static bool used[10];

typedef struct {
    char words[MAX_WORDS][MAX_LEN];
    int n_words;
    char letter[10];
    int n_letters;
    bool leading[26];
    long long coef[26];
    int n_cols;                 /* max word length */
    int col_end[MAX_LEN];       /* index in letter[] after which column c is fully assigned */
} Rebus;

static void parse(const char *s, Rebus *r) {
    bool seen[26] = {false};
    int len = 0;
    r->n_words = 0;
    r->n_letters = 0;
    memset(r->leading, 0, sizeof r->leading);
    memset(r->coef, 0, sizeof r->coef);
    for (const char *p = s; ; p++) {
        if (*p >= 'A' && *p <= 'Z') {
            r->words[r->n_words][len++] = *p;
            if (!seen[*p - 'A']) {
                seen[*p - 'A'] = true;
                r->letter[r->n_letters++] = *p;
            }
        } else if (*p == '+' || *p == '=' || *p == '\0') {
            if (len > 0) {
                r->words[r->n_words][len] = '\0';
                r->leading[r->words[r->n_words][0] - 'A'] = true;
                r->n_words++;
                len = 0;
            }
            if (*p == '\0') break;
        }
    }
    /* letters ordered by column, starting from the units column */
    int maxlen = 0;
    for (int i = 0; i < r->n_words; i++) {
        int L = (int)strlen(r->words[i]);
        if (L > maxlen) maxlen = L;
    }
    r->n_cols = maxlen;
    bool placed[26] = {false};
    int cnt = 0;
    for (int c = 0; c < maxlen; c++) {
        for (int i = 0; i < r->n_words; i++) {
            int L = (int)strlen(r->words[i]);
            if (c < L) {
                int ch = r->words[i][L - 1 - c] - 'A';
                if (!placed[ch]) { placed[ch] = true; r->letter[cnt++] = (char)('A' + ch); }
            }
        }
        r->col_end[c] = cnt;
    }
    for (int i = 0; i < r->n_words; i++) {
        long long w = 1;
        int L = (int)strlen(r->words[i]);
        for (int j = L - 1; j >= 0; j--, w *= 10)
            r->coef[r->words[i][j] - 'A'] += (i == r->n_words - 1) ? -w : w;
    }
}

static int col_carry[MAX_LEN + 1];

/* checks column c, given the carry from column c-1 */
static bool column_ok(const Rebus* r, int c) {
    int sum = col_carry[c];
    for (int i = 0; i < r->n_words - 1; i++) {
        int L = (int)strlen(r->words[i]);
        if (c < L) sum += digit[r->words[i][L - 1 - c] - 'A'];
    }
    const char* res = r->words[r->n_words - 1];
    int L = (int)strlen(res);
    int need = (c < L) ? digit[res[L - 1 - c] - 'A'] : 0;
    if (sum % 10 != need) return false;
    col_carry[c + 1] = sum / 10;
    return true;
}

static bool permute(const Rebus* r, int pos, int col) {
    if (pos == r->n_letters) return col == r->n_cols && col_carry[r->n_cols] == 0;

    for (int d = 0; d <= 9; d++) {
        if (used[d]) continue;
        if (d == 0 && r->leading[r->letter[pos] - 'A']) continue;

        used[d] = true;
        digit[r->letter[pos] - 'A'] = d;

        /* check every column that became complete (a column may add no new letters) */
        int c = col;
        bool ok = true;
        while (c < r->n_cols && pos + 1 == r->col_end[c]) {
            if (!column_ok(r, c)) { ok = false; break; }
            c++;
        }
        if (ok && permute(r, pos + 1, c)) return true;
        used[d] = false;
    }
    return false;
}

bool solve(const char* rebus, char* out, size_t out_size) {
    memset(digit, 0, sizeof digit);
    memset(used, 0, sizeof used);
    
    Rebus r;
    parse(rebus, &r); 

    memset(digit, 0, sizeof digit);
    memset(used, 0, sizeof used);

    col_carry[0] = 0;
    if (!permute(&r, 0, 0)) return false;

    size_t i = 0;
    for (const char* p = rebus; *p != '\0' && i + 1 < out_size; p++) {
        if (*p >= 'A' && *p <= 'Z')
            out[i++] = '0' + digit[*p - 'A'];
        else
            out[i++] = *p;
    }
    out[i] = '\0';
    return true;
}

int main(int argc, char** argv) {
    char buf[512];
    const char* path;

    if (argc >= 2) {
        path = argv[1];                    
    }
    else {
        printf("Enter file name: ");           
        if (!fgets(buf, sizeof buf, stdin)) return 1;
        buf[strcspn(buf, "\r\n")] = '\0';      
        path = buf;
    }

    FILE* f = fopen(path, "r");
    if (!f) { perror("fopen"); return 1; }
    
    static char lines[100][256];
    int n = 0;
    while (n < 100 && fgets(lines[n], sizeof lines[n], f)) {
        lines[n][strcspn(lines[n], "\r\n")] = '\0';
        if (lines[n][0] != '\0') n++;
    }
    fclose(f);

    int repeats = 20; 
    char answer[256];
    double total = 0;
    for (int i = 0; i < n; i++) {
        bool ok = false;
        clock_t t0 = clock();
        for (int k = 0; k < repeats; k++)
            ok = solve(lines[i], answer, sizeof answer);
        double ms = 1000.0 * (clock() - t0) / CLOCKS_PER_SEC / repeats;
        total += ms;
        printf("%-45s %8.2f ms  %s\n", lines[i], ms, ok ? answer : "no solution");
    }
    printf("TOTAL: %.2f ms\n", total);
    return 0;
}