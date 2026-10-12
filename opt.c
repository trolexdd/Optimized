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
} Rebus;

static void parse(const char *s, Rebus *r) {
    bool seen[26] = {false};
    int len = 0;
    r->n_words = 0;
    r->n_letters =0;
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
                r->n_words++;
                len = 0;
            }
            if (*p == '\0') break;
        }
    }
}

unsigned long long word_to_num(const char* w, const int* digit) {
    unsigned long long num = 0;
    for (; *w != '\0'; w++) {
        num = num * 10 + digit[*w - 'A'];
    }
    return num;
}

bool check(const Rebus* r, const int* digit) {
    for (int i = 0; i < r->n_words; i++) {
        char first = r->words[i][0];
        if (digit[first - 'A'] == 0) return false;
    }

    unsigned long long sum = 0;
    for (int i = 0; i < r->n_words - 1; i++)
        sum += word_to_num(r->words[i], digit);

    unsigned long long result = word_to_num(r->words[r->n_words - 1], digit);

    return sum == result;

}

static bool permute(const Rebus* r, int pos) {
    if (pos == r->n_letters) return check(r, digit);

    for (int d = 0; d <= 9; d++) {
        if (used[d]) continue; 

        used[d] = true;
        digit[r->letter[pos] - 'A'] = d;

        if (permute(r, pos+1)) return true;

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

    if (!permute(&r, 0)) return false;

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