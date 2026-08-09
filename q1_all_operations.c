/*
 * DAA Lab-02, Q1: Full implementation of all 7 dictionary operations
 * (Search, Insert, Delete, Max, Min, Predecessor, Successor) on all 6
 * structures (unsorted/sorted array, singly/doubly linked unsorted/sorted).
 *
 * Each section is self-contained. main() demonstrates every operation on
 * every structure so correctness can be checked by inspection.
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define NOT_FOUND -1
#define NO_VALUE  INT_MIN   /* sentinel meaning "no such element" */

/* =====================================================================
   1. UNSORTED ARRAY
   Search O(n) | Insert O(1) | Delete O(1)* | Max/Min O(n) | Pred/Succ O(n)
   *Delete given an index: swap with last element, order doesn't matter.
   ===================================================================== */
typedef struct { int *data; int n; } UArray;

int ua_search(UArray *a, int key) {                 /* O(n) */
    for (int i = 0; i < a->n; i++) if (a->data[i] == key) return i;
    return NOT_FOUND;
}
void ua_insert(UArray *a, int key) {                 /* O(1) */
    a->data[a->n++] = key;
}
void ua_delete(UArray *a, int index) {               /* O(1) */
    a->data[index] = a->data[a->n - 1];
    a->n--;
}
int ua_max(UArray *a) {                              /* O(n) */
    int m = a->data[0];
    for (int i = 1; i < a->n; i++) if (a->data[i] > m) m = a->data[i];
    return m;
}
int ua_min(UArray *a) {                              /* O(n) */
    int m = a->data[0];
    for (int i = 1; i < a->n; i++) if (a->data[i] < m) m = a->data[i];
    return m;
}
int ua_predecessor(UArray *a, int key) {             /* O(n) */
    int best = NO_VALUE;
    for (int i = 0; i < a->n; i++)
        if (a->data[i] < key && (best == NO_VALUE || a->data[i] > best)) best = a->data[i];
    return best;
}
int ua_successor(UArray *a, int key) {               /* O(n) */
    int best = NO_VALUE;
    for (int i = 0; i < a->n; i++)
        if (a->data[i] > key && (best == NO_VALUE || a->data[i] < best)) best = a->data[i];
    return best;
}

/* =====================================================================
   2. SORTED ARRAY
   Search O(log n) | Insert O(n) | Delete O(n) | Max/Min O(1) | Pred/Succ O(1)
   ===================================================================== */
typedef struct { int *data; int n; } SArray;

int sa_search(SArray *a, int key) {                  /* O(log n) */
    int lo = 0, hi = a->n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a->data[mid] == key) return mid;
        else if (a->data[mid] < key) lo = mid + 1;
        else hi = mid - 1;
    }
    return NOT_FOUND;
}
void sa_insert(SArray *a, int key) {                 /* O(n) */
    int i = a->n - 1;
    while (i >= 0 && a->data[i] > key) { a->data[i + 1] = a->data[i]; i--; }
    a->data[i + 1] = key;
    a->n++;
}
void sa_delete(SArray *a, int index) {               /* O(n) */
    for (int i = index; i < a->n - 1; i++) a->data[i] = a->data[i + 1];
    a->n--;
}
int sa_max(SArray *a) { return a->data[a->n - 1]; }  /* O(1) */
int sa_min(SArray *a) { return a->data[0]; }         /* O(1) */
int sa_predecessor(SArray *a, int index) {           /* O(1), given index of x */
    return index > 0 ? a->data[index - 1] : NO_VALUE;
}
int sa_successor(SArray *a, int index) {             /* O(1), given index of x */
    return index < a->n - 1 ? a->data[index + 1] : NO_VALUE;
}

/* =====================================================================
   3. SINGLY LINKED LIST, UNSORTED
   Search O(n) | Insert O(1) | Delete O(n)* | Max/Min O(n) | Pred/Succ O(n)
   *Delete needs the predecessor (no back-pointer), so must rescan from head.
   ===================================================================== */
typedef struct SNode { int key; struct SNode *next; } SNode;

SNode* sllu_search(SNode *head, int key) {           /* O(n) */
    for (SNode *p = head; p; p = p->next) if (p->key == key) return p;
    return NULL;
}
SNode* sllu_insert(SNode *head, int key) {           /* O(1), insert at head */
    SNode *n = malloc(sizeof(SNode));
    n->key = key; n->next = head;
    return n;
}
SNode* sllu_delete(SNode *head, SNode *target) {     /* O(n) */
    if (head == target) { SNode *nxt = head->next; free(head); return nxt; }
    SNode *p = head;
    while (p->next && p->next != target) p = p->next;
    if (p->next == target) { p->next = target->next; free(target); }
    return head;
}
int sllu_max(SNode *head) {                          /* O(n) */
    int m = head->key;
    for (SNode *p = head->next; p; p = p->next) if (p->key > m) m = p->key;
    return m;
}
int sllu_min(SNode *head) {                          /* O(n) */
    int m = head->key;
    for (SNode *p = head->next; p; p = p->next) if (p->key < m) m = p->key;
    return m;
}
int sllu_predecessor(SNode *head, int key) {         /* O(n) */
    int best = NO_VALUE;
    for (SNode *p = head; p; p = p->next)
        if (p->key < key && (best == NO_VALUE || p->key > best)) best = p->key;
    return best;
}
int sllu_successor(SNode *head, int key) {           /* O(n) */
    int best = NO_VALUE;
    for (SNode *p = head; p; p = p->next)
        if (p->key > key && (best == NO_VALUE || p->key < best)) best = p->key;
    return best;
}

/* =====================================================================
   4. SINGLY LINKED LIST, SORTED
   Search O(n) | Insert O(n) | Delete O(n) | Max O(n) | Min O(1)
   Predecessor O(n) | Successor O(1)
   ===================================================================== */
SNode* slls_search(SNode *head, int key) {           /* O(n) */
    for (SNode *p = head; p; p = p->next) {
        if (p->key == key) return p;
        if (p->key > key) break;                     /* sorted -> can stop early */
    }
    return NULL;
}
SNode* slls_insert(SNode *head, int key) {           /* O(n): find position */
    SNode *n = malloc(sizeof(SNode));
    n->key = key;
    if (!head || head->key >= key) { n->next = head; return n; }
    SNode *p = head;
    while (p->next && p->next->key < key) p = p->next;
    n->next = p->next; p->next = n;
    return head;
}
SNode* slls_delete(SNode *head, SNode *target) {     /* O(n): find predecessor */
    return sllu_delete(head, target);                 /* identical logic */
}
int slls_max(SNode *head) {                          /* O(n): walk to end */
    SNode *p = head;
    while (p->next) p = p->next;
    return p->key;
}
int slls_min(SNode *head) { return head->key; }      /* O(1): head is smallest */
int slls_predecessor(SNode *head, int key) {         /* O(n): must scan from head */
    int best = NO_VALUE;
    for (SNode *p = head; p && p->key < key; p = p->next) best = p->key;
    return best;
}
int slls_successor(SNode *head, SNode *x) {          /* O(1): given pointer to x */
    return x->next ? x->next->key : NO_VALUE;
}

/* =====================================================================
   5. DOUBLY LINKED LIST, UNSORTED
   Search O(n) | Insert O(1) | Delete O(1) | Max/Min O(n) | Pred/Succ O(n)
   ===================================================================== */
typedef struct DNode { int key; struct DNode *next, *prev; } DNode;

DNode* dllu_search(DNode *head, int key) {           /* O(n) */
    for (DNode *p = head; p; p = p->next) if (p->key == key) return p;
    return NULL;
}
DNode* dllu_insert(DNode *head, int key) {           /* O(1), insert at head */
    DNode *n = malloc(sizeof(DNode));
    n->key = key; n->next = head; n->prev = NULL;
    if (head) head->prev = n;
    return n;
}
DNode* dllu_delete(DNode *head, DNode *x) {          /* O(1): both neighbors known */
    if (x->prev) x->prev->next = x->next; else head = x->next;
    if (x->next) x->next->prev = x->prev;
    free(x);
    return head;
}
int dllu_max(DNode *head) {                          /* O(n) */
    int m = head->key;
    for (DNode *p = head->next; p; p = p->next) if (p->key > m) m = p->key;
    return m;
}
int dllu_min(DNode *head) {                          /* O(n) */
    int m = head->key;
    for (DNode *p = head->next; p; p = p->next) if (p->key < m) m = p->key;
    return m;
}
int dllu_predecessor(DNode *head, int key) {         /* O(n) */
    int best = NO_VALUE;
    for (DNode *p = head; p; p = p->next)
        if (p->key < key && (best == NO_VALUE || p->key > best)) best = p->key;
    return best;
}
int dllu_successor(DNode *head, int key) {           /* O(n) */
    int best = NO_VALUE;
    for (DNode *p = head; p; p = p->next)
        if (p->key > key && (best == NO_VALUE || p->key < best)) best = p->key;
    return best;
}

/* =====================================================================
   6. DOUBLY LINKED LIST, SORTED   (head AND tail pointers maintained)
   Search O(n) | Insert O(n) | Delete O(1) | Max/Min O(1) | Pred/Succ O(1)
   ===================================================================== */
typedef struct { DNode *head, *tail; } DListSorted;

DNode* dlls_search(DListSorted *L, int key) {        /* O(n) */
    for (DNode *p = L->head; p; p = p->next) {
        if (p->key == key) return p;
        if (p->key > key) break;
    }
    return NULL;
}
void dlls_insert(DListSorted *L, int key) {          /* O(n): find position */
    DNode *n = malloc(sizeof(DNode));
    n->key = key; n->next = NULL; n->prev = NULL;
    if (!L->head) { L->head = L->tail = n; return; }
    if (L->head->key >= key) {
        n->next = L->head; L->head->prev = n; L->head = n; return;
    }
    DNode *p = L->head;
    while (p->next && p->next->key < key) p = p->next;
    n->next = p->next;
    if (p->next) p->next->prev = n; else L->tail = n;
    p->next = n; n->prev = p;
}
void dlls_delete(DListSorted *L, DNode *x) {         /* O(1): direct unlink */
    if (x->prev) x->prev->next = x->next; else L->head = x->next;
    if (x->next) x->next->prev = x->prev; else L->tail = x->prev;
    free(x);
}
int dlls_max(DListSorted *L) { return L->tail->key; }/* O(1): tail pointer */
int dlls_min(DListSorted *L) { return L->head->key; }/* O(1): head pointer */
int dlls_predecessor(DNode *x) {                     /* O(1): given pointer to x */
    return x->prev ? x->prev->key : NO_VALUE;
}
int dlls_successor(DNode *x) {                       /* O(1): given pointer to x */
    return x->next ? x->next->key : NO_VALUE;
}

/* ===================================================================== */

void print_val(const char *label, int v) {
    if (v == NO_VALUE) printf("%-14s: (none)\n", label);
    else printf("%-14s: %d\n", label, v);
}

int main(void) {
    /* ---------------- 1. Unsorted array ---------------- */
    printf("=== Unsorted Array ===\n");
    int buf1[10] = {40, 10, 30, 20, 50};
    UArray ua = { buf1, 5 };
    print_val("search(30)", ua_search(&ua, 30));
    ua_insert(&ua, 60);
    print_val("max", ua_max(&ua));
    print_val("min", ua_min(&ua));
    print_val("pred(30)", ua_predecessor(&ua, 30));
    print_val("succ(30)", ua_successor(&ua, 30));
    ua_delete(&ua, 2);   /* delete element at index 2 */
    printf("after delete, n=%d\n\n", ua.n);

    /* ---------------- 2. Sorted array ---------------- */
    printf("=== Sorted Array ===\n");
    int buf2[10] = {10, 20, 30, 40, 50};
    SArray sa = { buf2, 5 };
    int idx = sa_search(&sa, 30);
    print_val("search(30)->idx", idx);
    print_val("max", sa_max(&sa));
    print_val("min", sa_min(&sa));
    print_val("pred(idx of 30)", sa_predecessor(&sa, idx));
    print_val("succ(idx of 30)", sa_successor(&sa, idx));
    sa_insert(&sa, 25);
    sa_delete(&sa, 0);
    printf("after insert+delete, n=%d\n\n", sa.n);

    /* ---------------- 3. SLL unsorted ---------------- */
    printf("=== Singly Linked, Unsorted ===\n");
    SNode *sllu = NULL;
    int vals[] = {50, 10, 40, 20, 30};
    for (int i = 0; i < 5; i++) sllu = sllu_insert(sllu, vals[i]);
    SNode *found = sllu_search(sllu, 40);
    print_val("search(40) found", found ? found->key : NO_VALUE);
    print_val("max", sllu_max(sllu));
    print_val("min", sllu_min(sllu));
    print_val("pred(30)", sllu_predecessor(sllu, 30));
    print_val("succ(30)", sllu_successor(sllu, 30));
    sllu = sllu_delete(sllu, found);
    printf("deleted node with key 40\n\n");

    /* ---------------- 4. SLL sorted ---------------- */
    printf("=== Singly Linked, Sorted ===\n");
    SNode *slls = NULL;
    for (int i = 0; i < 5; i++) slls = slls_insert(slls, vals[i]);
    found = slls_search(slls, 30);
    print_val("search(30) found", found ? found->key : NO_VALUE);
    print_val("max", slls_max(slls));
    print_val("min", slls_min(slls));
    print_val("pred(30)", slls_predecessor(slls, 30));
    print_val("succ(ptr to 30)", slls_successor(slls, found));
    slls = slls_delete(slls, found);
    printf("deleted node with key 30\n\n");

    /* ---------------- 5. DLL unsorted ---------------- */
    printf("=== Doubly Linked, Unsorted ===\n");
    DNode *dllu = NULL;
    for (int i = 0; i < 5; i++) dllu = dllu_insert(dllu, vals[i]);
    DNode *dfound = dllu_search(dllu, 40);
    print_val("search(40) found", dfound ? dfound->key : NO_VALUE);
    print_val("max", dllu_max(dllu));
    print_val("min", dllu_min(dllu));
    print_val("pred(30)", dllu_predecessor(dllu, 30));
    print_val("succ(30)", dllu_successor(dllu, 30));
    dllu = dllu_delete(dllu, dfound);   /* O(1) */
    printf("deleted node with key 40 (O(1))\n\n");

    /* ---------------- 6. DLL sorted ---------------- */
    printf("=== Doubly Linked, Sorted ===\n");
    DListSorted dlls = { NULL, NULL };
    for (int i = 0; i < 5; i++) dlls_insert(&dlls, vals[i]);
    dfound = dlls_search(&dlls, 30);
    print_val("search(30) found", dfound ? dfound->key : NO_VALUE);
    print_val("max", dlls_max(&dlls));
    print_val("min", dlls_min(&dlls));
    print_val("pred(ptr to 30)", dlls_predecessor(dfound));
    print_val("succ(ptr to 30)", dlls_successor(dfound));
    dlls_delete(&dlls, dfound);         /* O(1) */
    printf("deleted node with key 30 (O(1))\n");

    return 0;
}
