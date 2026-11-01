/*
 * DAA Lab 11 - Q03: Bidirectional BFS for Shortest Transformation Sequence in a Word Graph
 *
 * Description:
 * Computes the minimum number of single-character mutations k required to
 * transform a starting word A into a target word B using valid intermediate
 * words from a dictionary D of N unique lowercase words of length L.
 * Respects constraints: N <= 10^5, L <= 10, lowercase letters ('a'-'z').
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define MAX_N 100000
#define MAX_L 10
#define WORD_BUF_SIZE 16
#define HASH_TABLE_SIZE 262144 /* 2^18 (> 2 * 10^5 for low load factor <= 0.38) */
#define EMPTY_SLOT -1

/* Word Graph Dictionary and Problem Instance */
typedef struct {
    int n;                              /* Number of dictionary words (N <= 10^5) */
    int length;                         /* Length of each word (L <= 10) */
    char start_word[WORD_BUF_SIZE];     /* Starting word A */
    char target_word[WORD_BUF_SIZE];    /* Target word B */
    char (*words)[WORD_BUF_SIZE];       /* Array of N+1 words (index N reserved if A not in D) */
    int *hash_Slot;                     /* Open-addressed hash table mapping word -> index */
} WordDictionary;

/* --- Hash Table & Validation Helpers --- */

static uint32_t hash_word(const char *str, int len) {
    uint32_t h = 2166136261u; /* FNV-1a 32-bit offset basis */
    for (int i = 0; i < len; i++) {
        h ^= (uint8_t)str[i];
        h *= 16777619u;
    }
    return h & (HASH_TABLE_SIZE - 1);
}

bool is_valid_lowercase_word(const char *w, int expected_len) {
    int len = (int)strlen(w);
    if (len != expected_len) return false;
    for (int i = 0; i < len; i++) {
        if (w[i] < 'a' || w[i] > 'z') return false;
    }
    return true;
}

WordDictionary *create_dictionary(int n, int length) {
    WordDictionary *dict = (WordDictionary *)malloc(sizeof(WordDictionary));
    if (!dict) {
        perror("Failed to allocate WordDictionary");
        exit(EXIT_FAILURE);
    }
    dict->n = n;
    dict->length = length;
    /* Allocate n + 1 slots so index n can store start_word A if A is not initially in D */
    dict->words = malloc((n + 1) * sizeof(*dict->words));
    dict->hash_Slot = (int *)malloc(HASH_TABLE_SIZE * sizeof(int));
    if (!dict->words || !dict->hash_Slot) {
        perror("Failed to allocate dictionary storage");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        dict->hash_Slot[i] = EMPTY_SLOT;
    }
    return dict;
}

bool insert_word_into_hash(WordDictionary *dict, const char *word, int index) {
    uint32_t slot = hash_word(word, dict->length);
    while (dict->hash_Slot[slot] != EMPTY_SLOT) {
        int existing_idx = dict->hash_Slot[slot];
        if (strcmp(dict->words[existing_idx], word) == 0) {
            return false; /* Duplicate word */
        }
        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
    }
    strcpy(dict->words[index], word);
    dict->hash_Slot[slot] = index;
    return true;
}

int lookup_word_index(const WordDictionary *dict, const char *word) {
    uint32_t slot = hash_word(word, dict->length);
    while (dict->hash_Slot[slot] != EMPTY_SLOT) {
        int idx = dict->hash_Slot[slot];
        if (strcmp(dict->words[idx], word) == 0) {
            return idx;
        }
        slot = (slot + 1) & (HASH_TABLE_SIZE - 1);
    }
    return -1;
}

void free_dictionary(WordDictionary *dict) {
    if (!dict) return;
    free(dict->words);
    free(dict->hash_Slot);
    free(dict);
}

void print_dictionary_summary(const WordDictionary *dict) {
    printf("\n--- Word Graph Instance Summary ---\n");
    printf("Dictionary Size (N): %d | Word Length (L): %d\n", dict->n, dict->length);
    printf("Start Word (A): \"%s\" | Target Word (B): \"%s\"\n",
           dict->start_word, dict->target_word);
    if (dict->n <= 30) {
        printf("Dictionary Words D = { ");
        for (int i = 0; i < dict->n; i++) {
            printf("\"%s\"%s", dict->words[i], (i + 1 < dict->n) ? ", " : " ");
        }
        printf("}\n");
    } else {
        printf("(Dictionary listing omitted for N > 30; showing first 10 words: ");
        for (int i = 0; i < 10; i++) {
            printf("\"%s\"%s", dict->words[i], (i < 9) ? ", " : "...)\n");
        }
    }
}

/* --- Input Handling Functions --- */

WordDictionary *handle_manual_input(void) {
    int n, l;
    char buf[64];

    printf("\n[Manual Input Mode]\n");
    printf("Enter number of dictionary words N (1 to %d): ", MAX_N);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_N) {
        fprintf(stderr, "Invalid N. Must satisfy 1 <= N <= %d.\n", MAX_N);
        exit(EXIT_FAILURE);
    }

    printf("Enter length of each word L (1 to %d): ", MAX_L);
    if (scanf("%d", &l) != 1 || l < 1 || l > MAX_L) {
        fprintf(stderr, "Invalid L. Must satisfy 1 <= L <= %d.\n", MAX_L);
        exit(EXIT_FAILURE);
    }

    WordDictionary *dict = create_dictionary(n, l);

    printf("Enter starting word A (length %d, lowercase 'a'-'z'): ", l);
    if (scanf("%63s", buf) != 1 || !is_valid_lowercase_word(buf, l)) {
        fprintf(stderr, "Invalid starting word A. Must be %d lowercase letters.\n", l);
        free_dictionary(dict);
        exit(EXIT_FAILURE);
    }
    strcpy(dict->start_word, buf);

    printf("Enter target word B (length %d, lowercase 'a'-'z'): ", l);
    if (scanf("%63s", buf) != 1 || !is_valid_lowercase_word(buf, l)) {
        fprintf(stderr, "Invalid target word B. Must be %d lowercase letters.\n", l);
        free_dictionary(dict);
        exit(EXIT_FAILURE);
    }
    strcpy(dict->target_word, buf);

    printf("Enter %d unique dictionary words of length %d:\n", n, l);
    for (int i = 0; i < n; i++) {
        if (scanf("%63s", buf) != 1 || !is_valid_lowercase_word(buf, l)) {
            fprintf(stderr, "Invalid word at index %d. Must be %d lowercase letters.\n", i + 1, l);
            free_dictionary(dict);
            exit(EXIT_FAILURE);
        }
        if (!insert_word_into_hash(dict, buf, i)) {
            fprintf(stderr, "Duplicate dictionary word \"%s\" detected. Dictionary must contain unique words.\n", buf);
            free_dictionary(dict);
            exit(EXIT_FAILURE);
        }
    }

    return dict;
}

WordDictionary *generate_random_data(void) {
    int n, l;

    printf("\n[Randomized Data Generation Mode]\n");
    printf("Enter number of dictionary words N (1 to %d): ", MAX_N);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_N) {
        fprintf(stderr, "Invalid N. Must satisfy 1 <= N <= %d.\n", MAX_N);
        exit(EXIT_FAILURE);
    }

    printf("Enter length of each word L (1 to %d): ", MAX_L);
    if (scanf("%d", &l) != 1 || l < 1 || l > MAX_L) {
        fprintf(stderr, "Invalid L. Must satisfy 1 <= L <= %d.\n", MAX_L);
        exit(EXIT_FAILURE);
    }

    /* Check that 26^L >= N so N unique words can exist */
    long long max_possible = 1;
    for (int i = 0; i < l && max_possible <= MAX_N; i++) {
        max_possible *= 26;
    }
    if (n > max_possible) {
        fprintf(stderr, "Cannot generate %d unique words of length %d (max possible is %lld).\n",
                n, l, max_possible);
        exit(EXIT_FAILURE);
    }

    srand((unsigned int)time(NULL));
    WordDictionary *dict = create_dictionary(n, l);

    /* Generate starting word A */
    for (int j = 0; j < l; j++) {
        dict->start_word[j] = (char)('a' + (rand() % 26));
    }
    dict->start_word[l] = '\0';

    /* Plant a valid mutation chain of length chain_len = min(n, 3..8) so random tests
     * frequently exhibit interesting multi-step shortest paths, then fill remaining
     * slots with random unique words of length L. */
    int chain_len = (n >= 6) ? (3 + (rand() % 5)) : n;
    char curr[WORD_BUF_SIZE];
    strcpy(curr, dict->start_word);

    int inserted = 0;
    for (int step = 0; step < chain_len && inserted < n; step++) {
        int pos = rand() % l;
        int delta = 1 + (rand() % 25);
        curr[pos] = (char)('a' + ((curr[pos] - 'a' + delta) % 26));
        if (strcmp(curr, dict->start_word) != 0 && insert_word_into_hash(dict, curr, inserted)) {
            inserted++;
        }
    }

    /* Set target word B to the end of the planted chain (or a dictionary word) */
    if (inserted > 0) {
        strcpy(dict->target_word, dict->words[inserted - 1]);
    } else {
        for (int j = 0; j < l; j++) {
            dict->target_word[j] = (char)('a' + (rand() % 26));
        }
        dict->target_word[l] = '\0';
        insert_word_into_hash(dict, dict->target_word, inserted++);
    }

    /* Fill remaining dictionary slots with uniformly random unique lowercase words */
    while (inserted < n) {
        char candidate[WORD_BUF_SIZE];
        for (int j = 0; j < l; j++) {
            candidate[j] = (char)('a' + (rand() % 26));
        }
        candidate[l] = '\0';
        if (insert_word_into_hash(dict, candidate, inserted)) {
            inserted++;
        }
    }

    return dict;
}

/* --- Core Algorithmic Logic: Bidirectional BFS on Implicit Word Graph --- */

/* Expands one full BFS level from the given queue (`q`) and checks if any
 * neighbor has already been visited by the opposite search direction (`dist_other`).
 * Returns the meeting word index if the two frontiers intersect, or -1 otherwise. */
static int expand_one_level(const WordDictionary *dict,
                            int *q, int *front, int *back,
                            int *dist_self, const int *dist_other,
                            int *parent_self) {
    int level_size = *back - *front;
    int len = dict->length;

    for (int i = 0; i < level_size; i++) {
        int u = q[(*front)++];
        char mutated[WORD_BUF_SIZE];
        strcpy(mutated, dict->words[u]);

        for (int pos = 0; pos < len; pos++) {
            char orig_char = mutated[pos];
            for (char c = 'a'; c <= 'z'; c++) {
                if (c == orig_char) continue;
                mutated[pos] = c;

                int v = lookup_word_index(dict, mutated);
                if (v != -1) {
                    if (dist_other[v] != -1) {
                        /* Frontiers meet! Record parent if not yet visited on this side */
                        if (dist_self[v] == -1) {
                            dist_self[v] = dist_self[u] + 1;
                            parent_self[v] = u;
                        }
                        return v;
                    }
                    if (dist_self[v] == -1) {
                        dist_self[v] = dist_self[u] + 1;
                        parent_self[v] = u;
                        q[(*back)++] = v;
                    }
                }
            }
            mutated[pos] = orig_char;
        }
    }
    return -1;
}

static void print_transformation_sequence(const WordDictionary *dict,
                                          int meet_idx,
                                          const int *parent_fwd,
                                          const int *parent_bwd) {
    int total_nodes = dict->n + 1;
    int *fwd_chain = (int *)malloc(total_nodes * sizeof(int));
    int fwd_len = 0;

    int curr = meet_idx;
    while (curr != -1) {
        fwd_chain[fwd_len++] = curr;
        curr = parent_fwd[curr];
    }

    printf("Shortest Transformation Sequence: ");
    for (int i = fwd_len - 1; i >= 0; i--) {
        printf("%s", dict->words[fwd_chain[i]]);
        if (i > 0 || parent_bwd[meet_idx] != -1) {
            printf(" -> ");
        }
    }

    curr = parent_bwd[meet_idx];
    while (curr != -1) {
        printf("%s", dict->words[curr]);
        curr = parent_bwd[curr];
        if (curr != -1) printf(" -> ");
    }
    printf("\n");

    free(fwd_chain);
}

int solve_bidirectional_word_bfs(WordDictionary *dict) {
    /* Rule 2 check: Target word B must always exist in dictionary D */
    int target_idx = lookup_word_index(dict, dict->target_word);
    if (target_idx == -1) {
        printf("\n--- Bidirectional BFS Result ---\n");
        printf("Target word B = \"%s\" is NOT in dictionary D.\n", dict->target_word);
        printf("Minimum number of mutations k: -1 (No valid path exists)\n");
        return -1;
    }

    /* Trivial case: A == B */
    if (strcmp(dict->start_word, dict->target_word) == 0) {
        printf("\n--- Bidirectional BFS Result ---\n");
        printf("Start word A and Target word B are identical (\"%s\").\n", dict->start_word);
        printf("Minimum number of mutations k: 0\n");
        return 0;
    }

    /* Check if A is already in D; if not, insert A into virtual slot index N
     * so both forward and backward frontiers can reference it seamlessly. */
    int start_idx = lookup_word_index(dict, dict->start_word);
    if (start_idx == -1) {
        start_idx = dict->n;
        insert_word_into_hash(dict, dict->start_word, start_idx);
    }

    int total_slots = dict->n + 1;
    int *dist_fwd = (int *)malloc(total_slots * sizeof(int));
    int *dist_bwd = (int *)malloc(total_slots * sizeof(int));
    int *parent_fwd = (int *)malloc(total_slots * sizeof(int));
    int *parent_bwd = (int *)malloc(total_slots * sizeof(int));
    int *q_fwd = (int *)malloc(total_slots * sizeof(int));
    int *q_bwd = (int *)malloc(total_slots * sizeof(int));

    if (!dist_fwd || !dist_bwd || !parent_fwd || !parent_bwd || !q_fwd || !q_bwd) {
        perror("Failed to allocate Bidirectional BFS buffers");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < total_slots; i++) {
        dist_fwd[i] = -1;
        dist_bwd[i] = -1;
        parent_fwd[i] = -1;
        parent_bwd[i] = -1;
    }

    int front_fwd = 0, back_fwd = 0;
    int front_bwd = 0, back_bwd = 0;

    dist_fwd[start_idx] = 0;
    q_fwd[back_fwd++] = start_idx;

    dist_bwd[target_idx] = 0;
    q_bwd[back_bwd++] = target_idx;

    int min_mutations = -1;
    int meet_node = -1;

    /* Bidirectional BFS loop: always expand the smaller active frontier */
    while (front_fwd < back_fwd && front_bwd < back_bwd) {
        int size_fwd = back_fwd - front_fwd;
        int size_bwd = back_bwd - front_bwd;

        if (size_fwd <= size_bwd) {
            meet_node = expand_one_level(dict, q_fwd, &front_fwd, &back_fwd,
                                         dist_fwd, dist_bwd, parent_fwd);
        } else {
            meet_node = expand_one_level(dict, q_bwd, &front_bwd, &back_bwd,
                                         dist_bwd, dist_fwd, parent_bwd);
        }

        if (meet_node != -1) {
            min_mutations = dist_fwd[meet_node] + dist_bwd[meet_node];
            break;
        }
    }

    printf("\n--- Bidirectional BFS Result ---\n");
    if (min_mutations != -1) {
        printf("Minimum number of mutations k: %d\n", min_mutations);
        print_transformation_sequence(dict, meet_node, parent_fwd, parent_bwd);
    } else {
        printf("Minimum number of mutations k: -1 (No valid transformation sequence exists)\n");
    }

    free(dist_fwd);
    free(dist_bwd);
    free(parent_fwd);
    free(parent_bwd);
    free(q_fwd);
    free(q_bwd);

    return min_mutations;
}

/* --- Menu-Driven Entry Point --- */

int main(void) {
    int choice;
    printf("===========================================================================\n");
    printf("  Q03: Bidirectional BFS for Shortest Transformation Sequence in Word Graph\n");
    printf("===========================================================================\n");
    printf("Select Input Method:\n");
    printf("  1. Manual user input\n");
    printf("  2. Randomized data generation\n");
    printf("Enter choice (1 or 2): ");

    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) {
        fprintf(stderr, "Invalid choice. Please enter 1 or 2.\n");
        return EXIT_FAILURE;
    }

    WordDictionary *dict = NULL;
    if (choice == 1) {
        dict = handle_manual_input();
    } else {
        dict = generate_random_data();
    }

    print_dictionary_summary(dict);
    solve_bidirectional_word_bfs(dict);
    free_dictionary(dict);

    return EXIT_SUCCESS;
}
