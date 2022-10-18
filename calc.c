/** @file
  Implementacja kalkulatora działającego na wielomianach i stosującego odwrotną notację polską.
  @author Wiktoria Walczak
  @date 2021
*/

#include "poly.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include "additional_functions.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define ADD "ADD"
#define ZERO "ZERO"
#define IS_COEFF "IS_COEFF"
#define IS_ZERO "IS_ZERO"
#define CLONE "CLONE"
#define MUL "MUL"
#define NEG "NEG"
#define SUB "SUB"
#define IS_EQ "IS_EQ"
#define DEG "DEG"
#define DEG_BY "DEG_BY"
#define AT "AT"
#define PRINT "PRINT"
#define POP "POP"
#define COMPOSE "COMPOSE"

#define INITIAL_LENGTH 8
#define FIRST_NUMBER 48
#define LAST_NUMBER 57
#define FIRST_SMALL_LETTER 97
#define LAST_SMALL_LETTER 122
#define FIRST_BIG_LETTER 65
#define LAST_BIG_LETTER 90

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * To jest struktura reprezentująca stos tablicowy.
 */
typedef struct {
    Poly *array; ///< tablica przechowywująca wartości ze stosu
    size_t top; ///< indeks pierwszego wolnego miejsca na stosie
    size_t length; ///< rozmiar stosu
} Stack;

/**
 * Tworzy nowy, pusty stos.
 * @return pusty stos
 */
Stack newPolyStack(void) {
    Stack stack;
    stack.top = 0;
    stack.length = INITIAL_LENGTH;
    stack.array = malloc(stack.length * sizeof *stack.array);
    return stack;
}

/**
 * Sprawdza, czy stos @f$stack@f$ jest pusty.
 * @param[in] stack : stos
 * @return Czy stos jest pusty?
 */
bool emptyPoly(Stack *stack) {
    return stack->top == 0;
}

/**
 * Daje wielomian z wierzchu stosu @f$stack@f$, nie zdejumując go.
 * @param[in] stack: stos
 * @return wielomian z góry stosu
 */
Poly topPoly(Stack *stack) {
    return stack->array[stack->top - 1];
}

/**
 * Zdejmuje wielomian z wierzchu stosu @f$stack@f$ i zwraca go.
 * @param[in] stack: stos
 * @return wielomian z góry stosu
 */
Poly popPoly(Stack *stack) {
    --stack->top;
    return stack->array[stack->top];
}

/**
 * Usuwa stos @f$stack@f$ z pamięci.
 * @param[in] stack : stos
 */
void freeStack(Stack *stack) {
    while (!emptyPoly(stack)) {
        Poly p = popPoly(stack);
        PolyDestroy(&p);
    }
    stack->length = 0;
    free(stack->array);
}

/**
 * Wkłada wielomain @f$poly@f$ na stos @f$stack@f$.
 * @param[in,out] stack : stos
 * @param[in] poly : wielomian @f$q@f$
 */
void pushPoly(Stack *stack, Poly poly) {
    if (stack->top == stack->length) {
        stack->length = more(stack->length);
        stack->array = realloc(stack->array, stack->length * sizeof *stack->array);
        CheckReallocOutcome(stack->array);
    }
    stack->array[stack->top] = poly;
    ++stack->top;
}

/**
 * Wczytuje znaki z wejścia, dopóki nie wczyta znaku końca linii.
 */
static void readRestOfLine(void) {
    int c = getchar();
    while ((c != EOF) && (c != '\n')) {
        c = getchar();
    }
}

/**
 * Zwraca wartość true, jeśli na zmiennej @f$c@f$ zapisana jest mała lub wielka litera alfabetu angielskiego.
 * Zwraca wartość false w przeciwnym przypadku.
 * @param[in] c : znak
 * @return Czy @c to litera?
 */
static bool isLetter(int c) {
    if (((c <= LAST_BIG_LETTER) && (c >= FIRST_BIG_LETTER)) ||
        ((c >= FIRST_SMALL_LETTER) && (c <= LAST_SMALL_LETTER))) {
        return true;
    } else {
        return false;
    }
}

/**
 * Wczytuje z wejścia znaki i konwertuje je do postaci liczby typu poly_coeff_t.
 * Jeśli wczyta niedozwolony znak
 * lub nastąpi przekroczenie zakresu typu poly_coeff_t,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] number : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość buffera
 * @return wczytana wartość
 */
static poly_coeff_t readCoeff(bool *correct, char **number, size_t *length) {
    int c = getchar();
    size_t i = 0;
    if (c == '-') {
        lengthenIfNecessary(number, length, i);
        (*number)[i] = c;
        c = getchar();
        ++i;
    }
    while ((c != EOF) && (c != ',') && (c != '\n') && *correct) {
        if ((c >= FIRST_NUMBER) && (c <= LAST_NUMBER)) {
            lengthenIfNecessary(number, length, i);
            (*number)[i] = c;
            ++i;
            c = getchar();
        } else {
            *correct = false;
        }
    }
    if (i == 0) {
        *correct = false;
    }
    lengthenIfNecessary(number, length, i);
    ungetc(c, stdin);
    (*number)[i] = '\0';
    lengthenIfNecessary(number, length, i);
    (*number)[i] = '#';
    char *pointer = NULL;
    errno = 0;
    poly_coeff_t res = strtol(*number, &pointer, 10);
    if (errno != 0) {
        *correct = false;
    }
    return res;
}

/**
 * Wczytuje ze standardowego wejścia znaki i konwertuje je do postaci liczby typu unsigned long.
 * Jeśli wczyta niedozwolony znak
 * lub nastąpi przekroczenie zakresu typu unsigned long,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] number : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość buffera
 * @return wczytana wartość
 */
static unsigned long readUnsignedLong(bool *correct, char **number, size_t *length) {
    int c = getchar();
    size_t i = 0;
    if (c == '-') {
        *correct = false;
    }
    while ((c != EOF) && (c != ',') && (c != '\n') && *correct) {
        if ((c >= FIRST_NUMBER) && (c <= LAST_NUMBER)) {
            lengthenIfNecessary(number, length, i);
            (*number)[i] = c;
            ++i;
            c = getchar();
        } else {
            *correct = false;
        }
    }
    if (i == 0) {
        *correct = false;
    }
    lengthenIfNecessary(number, length, i);
    ungetc(c, stdin);
    (*number)[i] = '\0';
    lengthenIfNecessary(number, length, i);
    (*number)[i] = '#';
    char *pointer = NULL;
    errno = 0;
    unsigned long res = strtoul(*number, &pointer, 10);
    if (errno != 0) {
        *correct = false;
    }
    return res;
}

/**
 * Wczytuje ze standardowego wejścia znaki i konwertuje je do postaci liczby typu poly_exp_t.
 * Jeśli wczyta niedozwolony znak
 * lub nastąpi przekroczenie zakresu typu unsigned long,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] number : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość buffera
 * @return wczytana wartość
 */
static poly_exp_t readExp(bool *correct, char **number, size_t *length) {
    int c = getchar();
    size_t i = 0;
    while ((c != EOF) && (c != ')') && (c != '\n') && *correct) {
        if ((c >= FIRST_NUMBER) && (c <= LAST_NUMBER)) {
            lengthenIfNecessary(number, length, i);
            (*number)[i] = c;
            ++i;
            c = getchar();
        } else {
            ungetc(c, stdin);
            *correct = false;
            return 0;
        }
    }
    ungetc(c, stdin);
    if (i == 0) {
        *correct = false;
        return 0;
    }
    lengthenIfNecessary(number, length, i);
    (*number)[i] = '\0';
    lengthenIfNecessary(number, length, i);
    (*number)[i] = '#';
    char *pointer = NULL;
    long long help = strtol(*number, &pointer, 10);
    if ((help > INT_MAX) || (help < 0)) {
        *correct = false;
        return 0;
    }
    poly_exp_t res = (poly_exp_t) help;
    return res;
}

static Poly readPoly(bool *correct, char **buffer, size_t *bufferLength);

/**
 * Tworzy jednomian na podstawie znaków, wczytanych ze standardowego wejścia.
 * Jeśli wczyta niedozwolony znak,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] buffer : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość buffera
 * @return wczytany jednomian
 */
static Mono readMono(bool *correct, char **buffer, size_t *length) {
    int a = getchar();
    if (a == '-') {
        a = getchar();
        if (a == '\n') {
            *correct = false;
            ungetc(a, stdin);
        }
        if ((a >= FIRST_NUMBER) && (a <= LAST_NUMBER)) {
            ungetc(a, stdin);
            poly_coeff_t coeff = readCoeff(correct, buffer, length);
            a = getchar();
            if (a != '\n') {
                *correct = false;
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            ungetc(a, stdin);
            Poly res = PolyFromCoeff(-1 * coeff);
            return MonoFromPoly(&res, 0);
        } else {
            *correct = false;
            Poly res = PolyZero();
            return MonoFromPoly(&res, 0);
        }
    } else if ((a >= FIRST_NUMBER) && (a <= LAST_NUMBER)) {
        ungetc(a, stdin);
        poly_coeff_t coeff = readCoeff(correct, buffer, length);
        a = getchar();
        if (a != '\n') {
            *correct = false;
            Poly res = PolyZero();
            return MonoFromPoly(&res, 0);
        }
        ungetc(a, stdin);
        Poly res = PolyFromCoeff(coeff);
        return MonoFromPoly(&res, 0);
    } else if (a == '(') {
        a = getchar();
        if (a == '\n') {
            ungetc(a, stdin);
        }
        if (((a >= FIRST_NUMBER) && (a <= LAST_NUMBER)) || (a == '-')) {
            ungetc(a, stdin);
            poly_coeff_t coeff = readCoeff(correct, buffer, length);
            a = getchar();
            if (a == '\n') {
                ungetc(a, stdin);
            }
            if (a != ',') {
                *correct = false;
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            poly_exp_t exp = readExp(correct, buffer, length);
            a = getchar();
            if (a == '\n') {
                ungetc(a, stdin);
            }
            if (a != ')') {
                *correct = false;
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            if (coeff == 0) {
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            if (exp == 0) {
                Poly res = PolyFromCoeff(coeff);
                return MonoFromPoly(&res, 0);
            }
            Poly p = PolyFromCoeff(coeff);
            return MonoFromPoly(&p, exp);
        } else {
            if (a != '(') {
                *correct = false;
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            ungetc(a, stdin);
            Poly coeff = readPoly(correct, buffer, length);
            if (!*correct) {
                PolyDestroy(&coeff);
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            a = getchar();
            if (a == '\n') {
                ungetc(a, stdin);
            }
            if (a != ',') {
                PolyDestroy(&coeff);
                *correct = false;
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            poly_exp_t exp = readExp(correct, buffer, length);
            a = getchar();
            if (a == '\n') {
                ungetc(a, stdin);
            }
            if (a != ')') {
                *correct = false;
                PolyDestroy(&coeff);
                Poly res = PolyZero();
                return MonoFromPoly(&res, 0);
            }
            if (PolyIsZero(&coeff)) {
                return MonoFromPoly(&coeff, 0);
            }
            return MonoFromPoly(&coeff, exp);
        }
    } else {
        if (a == '\n') {
            ungetc(a, stdin);
        }
        *correct = false;
        Poly res = PolyZero();
        return MonoFromPoly(&res, 0);
    }
}

/**
 * Tworzy wielomian na podstawie znaków wczytanych ze standarowego wejścia.
 * Jeśli wczyta niedozwolony znak,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] buffer : bufor do wczytywania znaków z wejścia
 * @param[in] bufferLength : długość buffera
 * @return wczytany wielomian
 */
static Poly readPoly(bool *correct, char **buffer, size_t *bufferLength) {
    if (!*correct) {
        return PolyZero();
    }
    Mono m1 = readMono(correct, buffer, bufferLength);
    int c = getchar();
    if (c == '\n') {
        ungetc(c, stdin);
    }
    if (*correct == false) {
        MonoDestroy(&m1);
        return PolyZero();
    }
    size_t length = 1024;
    Mono *arr = malloc(length * sizeof *arr);
    size_t i = 0;
    arr[i] = m1;
    ++i;
    while ((c != EOF) && (c == '+')) {
        Mono m2 = readMono(correct, buffer, bufferLength);
        if (!*correct) {
            for (size_t k = 0; k < i; ++k) {
                MonoDestroy(&arr[k]);
            }
            free(arr);
            return PolyZero();
        }
        LengthenArrayIfNecessary(&arr, &length, i);
        arr[i] = m2;
        ++i;
        c = getchar();
        if (c == '\n') {
            ungetc(c, stdin);
        }
    }
    if (c == ',') {
        ungetc(c, stdin);
    }
    if ((c != '\n') && (c != ',')) {
        *correct = false;
    }
    Poly res;
    if ((i == 1) && (arr[0].exp == 0) && (arr[0].p.arr == NULL)) {
        res = arr[0].p;
    } else {
        res = PolyAddMonos(i, arr);
    }
    free(arr);
    return res;
}

/**
 * Dodaje dwa wielomiany z wierzchu stosu, usuwa je
 * i wstawia na wierzchołek stosu ich sumę.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool add(Stack *s) {
    if (s->top < 2) {
        return false;
    }
    Poly p = popPoly(s);
    Poly q = popPoly(s);
    Poly res = PolyAdd(&p, &q);
    PolyDestroy(&p);
    PolyDestroy(&q);
    pushPoly(s, res);
    return true;
}

/**
 * Sprawdza, czy wielomian na wierzchołku stosu jest współczynnikiem.
 * Wpisuje na standardowe wyjście 0, jeśli nie jest lub 1, jeśli jest.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool isCoeff(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = topPoly(s);
    if (PolyIsCoeff(&p)) {
        printf("%d\n", true);
    } else {
        if (PolyIsConst(&p)) {
            printf("%d\n", true);
        } else {
            printf("%d\n", false);
        }
    }
    return true;
}

/**
 * Sprawdza, czy wielomian na wierzchołku stosu jest tożsamościowo równy zeru.
 * Wpisuje na standardowe wyjście 0, jeśli nie jest lub 1, jeśli jest.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool isZero(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = topPoly(s);
    if (PolyIsZero(&p)) {
        printf("%d\n", true);
    } else {
        printf("%d\n", false);
    }
    return true;
}

/**
 * Wstawia na stos kopię wielomianu z wierzchołka.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool clone(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = topPoly(s);
    Poly q = PolyClone(&p);
    pushPoly(s, q);
    return true;
}

/**
 * Mnoży dwa wielomiany z wierzchu stosu, usuwa je
 * i wstawia na wierzchołek stosu ich iloczyn.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool mul(Stack *s) {
    if (s->top < 2) {
        return false;
    }
    Poly p = popPoly(s);
    Poly q = popPoly(s);
    Poly res = PolyMul(&p, &q);
    PolyDestroy(&p);
    PolyDestroy(&q);
    pushPoly(s, res);
    return true;
}

/**
 * Odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem,
 * usuwa je i wstawia na wierzchołek stosu różnicę.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool sub(Stack *s) {
    if (s->top < 2) {
        return false;
    }
    Poly p = popPoly(s);
    Poly q = popPoly(s);
    Poly res = PolySub(&p, &q);
    PolyDestroy(&p);
    PolyDestroy(&q);
    pushPoly(s, res);
    return true;
}

/**
 * Neguje wielomian na wierzchołku stosu.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool neg(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = popPoly(s);
    Poly q = PolyNeg(&p);
    PolyDestroy(&p);
    pushPoly(s, q);
    return true;
}

/**
 * Sprawdza, czy dwa wielomiany na wierzchu stosu są równe.
 * Wypisuje na standardowe wyjście 0, jeśli nie są lub 1, jeśli są.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool isEq(Stack *s) {
    if (s->top < 2) {
        return false;
    }
    Poly p = popPoly(s);
    Poly q = topPoly(s);
    pushPoly(s, p);
    if (PolyIsEq(&p, &q)) {
        printf("%d\n", true);
    } else {
        printf("%d\n", false);
    }
    return true;
}

/**
 * Wypisuje na standardowe wyjście stopień wielomianu z góry stosu.
 * Wypisuje −1 dla wielomianu tożsamościowo równego zeru.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool deg(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = topPoly(s);
    poly_exp_t exp = PolyDeg(&p);
    printf("%d\n", exp);
    return true;
}

static void PrintPoly(const Poly *p);

/**
 * Wypisuje na standardowe wyjście wielomian z wierzchołka stosu.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool print(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = topPoly(s);
    PrintPoly(&p);
    printf("\n");
    return true;
}

/**
 * Usuwa wielomian z wierzchołka stosu.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @return Czy udało się wykonać polecenie?
 */
static bool pop(Stack *s) {
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = popPoly(s);
    PolyDestroy(&p);
    return true;
}

/**
 * Wczytuje ze standardowego wejścia wartość @f$idx@f$.
 * Jeśli wczyta niedozwolony znak,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * Wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną o numerze @f$idx@f$.
 * Wypisuje −1 dla wielomianu tożsamościowo równego zeru.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] buffer : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość buffera
 * @return Czy udało się wykonać polecenie?
 */
static bool degBy(Stack *s, bool *correct, char **buffer, size_t *length) {
    unsigned long idx = readUnsignedLong(correct, buffer, length);
    if (!*correct) {
        return true;
    }
    int a = getchar();
    if ((a != EOF) && (a != '\n')) {
        *correct = false;
        return false;
    }
    ungetc(a, stdin);
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = topPoly(s);
    poly_exp_t degBy = PolyDegBy(&p, idx);
    printf("%d\n", degBy);
    return true;
}

/**
 * Wczytuje ze standardowego wejścia wartość @f$k@f$.
 * Jeśli wczyta niedozwolony znak,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * Funkcja zdejmuje ze stosu najpierw wielomian @f$p@f$.
 * Następnie zdejumuje ze stosu kolejno @f$q[k - 1], q[k - 2],\ldots, q[0]@f$.
 * Umieszcza na stosie wynik operacji złożenia.
 * Wynikiem złożenia jest wielomian powstający przez podstawienie w wielomianie @f$p@f$
 * pod zmienną @f$x_i@f$ wielomianu @f$q_i@f$ dla @f$i = 0,1,2,\ldots,min(k,l)-1@f$.
 * Jeśli @f$k < l@f$, to pod zmienne @f$x_k, \ldots x_{l-1}@f$ podstawiane są zera.
 * @f$l@f$ oznacza liczbę zmiennych wielomianu @f$p@f$.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów, aby wykonać polecenie.
 * @param[in,out] s : stos
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] buffer : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość bufora
 * @return Czy udało się wykonać polecenie?
 */
static bool compose(Stack *s, bool *correct, char **buffer, size_t *length) {
    unsigned long k = readUnsignedLong(correct, buffer, length);
    if (!*correct) {
        return true;
    }
    int a = getchar();
    if ((a != EOF) && (a != '\n')) {
        *correct = false;
        return false;
    }
    ungetc(a, stdin);
    if (s->top - 1 < k) {
        return false;
    }
    Poly p = popPoly(s);
    Poly *q = malloc(k * sizeof *q);
    size_t i = k;
    while (i > 0) {
        q[i - 1] = popPoly(s);
        --i;
    }
    Poly res = PolyCompose(&p, k, q);
    pushPoly(s, res);
    PolyDestroy(&p);
    for (size_t j = 0; j < k; ++j) {
        PolyDestroy(&q[j]);
    }
    free(q);
    return true;
}

/**
 * Wczytuje ze standardowego wejścia wartość @f$x@f$.
 * Jeśli wczyta niedozwolony znak,
 * ustawia wartość zmiennej, na którą wskazuje wskaźnik @f$correct@f$, na false.
 * Wylicza wartość wielomianu w punkcie @f$x@f$, usuwa wielomian z wierzchołka i wstawia na stos wynik operacji.
 * Zwraca wartość false, jeśli na stosie jest za mało wielomianów,
 * aby wykonać polecenie.
 * @param[in,out] s : stos
 * @param[in,out] correct : wskaźnik na zmienną typu bool
 * @param[in] buffer : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość bufora
 * @return Czy udało się wykonać polecenie?
 */
static bool at(Stack *s, bool *correct, char **buffer, size_t *length) {
    poly_coeff_t x = readCoeff(correct, buffer, length);
    if (!*correct) {
        return true;
    }
    int c = getchar();
    if ((c != EOF) && (c != '\n')) {
        *correct = false;
        return false;
    }
    ungetc(c, stdin);
    if (emptyPoly(s)) {
        return false;
    }
    Poly p = popPoly(s);
    Poly res = PolyAt(&p, x);
    PolyDestroy(&p);
    pushPoly(s, res);
    return true;
}

/**
 * Wstawia na wierzchołek stosu wielomian tożsamościowo równy zeru.
 * @param[in] s : stos
 */
static void zero(Stack *s) {
    Poly p = PolyZero();
    pushPoly(s, p);
}

/**
 * Wczytuje ze standardowego wejścia polecenie i je wykonuje.
 * Jeśli wykryje niepoprawną nazwę polecenia zwraca wartość false.
 * @param[in,out] s : stos
 * @param[in] line : numer lini, w której znajduje się dane polecenie
 * @param[in] read : bufor do wczytywania znaków z wejścia
 * @param[in] length : długość buffera
 * @return Czy udało się wczytać poprawne polecenie?
 */
static bool readCommand(Stack *s, int line, char **read, size_t *length) {
    int i = 0;
    int c = getchar();
    while ((c != EOF) && (c != '\n') && (!isspace(c))) {
        if (!isLetter(c) && (c != '_')) {
            return false;
        }
        lengthenIfNecessary(read, length, i);
        (*read)[i] = c;
        ++i;
        c = getchar();
    }
    bool EndLine = false;
    if ((c == EOF) || (c == '\n')) {
        EndLine = true;
    }
    if (EndLine) {
        ungetc(c, stdin);
    }
    lengthenIfNecessary(read, length, i);
    (*read)[i] = '\0';
    ++i;
    switch ((*read)[0]) {
        case 'Z':
            if (!EndLine) {
                return false;
            }
            if (strncmp(ZERO, *read, i) == 0) {
                zero(s);
            } else {
                return false;
            }
            break;
        case 'I':
            if (!EndLine) {
                return false;
            }
            if (strncmp(IS_COEFF, *read, i) == 0) {
                if (!isCoeff(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else if (strncmp(IS_ZERO, *read, i) == 0) {
                if (!isZero(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else if (strncmp(IS_EQ, *read, i) == 0) {
                if (!isEq(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'C':
            if (strncmp(COMPOSE, *read, i) == 0) {
                if (c != ' ') {
                    fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line);
                    return true;
                }
                bool correct = true;
                bool noUnderflow = compose(s, &correct, read, length);
                if (!correct) {
                    fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line);
                } else if (!noUnderflow) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else if (strncmp(CLONE, *read, i) == 0) {
                if (!EndLine) {
                    return false;
                }
                if (!clone(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'A':
            if (strncmp(ADD, *read, i) == 0) {
                if (!EndLine) {
                    return false;
                }
                if (!add(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else if (strncmp(AT, *read, i) == 0) {
                if (c != ' ') {
                    fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line);
                    return true;
                }
                bool correct = true;
                bool noUnderflow = at(s, &correct, read, length);
                if (!correct) {
                    fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line);
                } else if (!noUnderflow) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'M':
            if (!EndLine) {
                return false;
            }
            if (strncmp(MUL, *read, i) == 0) {
                if (!mul(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'N':
            if (!EndLine) {
                return false;
            }
            if (strncmp(NEG, *read, i) == 0) {
                if (!neg(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'S':
            if (!EndLine) {
                return false;
            }
            if (strncmp(SUB, *read, i) == 0) {
                if (!sub(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'P':
            if (!EndLine) {
                return false;
            }
            if (strncmp(PRINT, *read, i) == 0) {
                if (!print(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else if (strncmp(POP, *read, i) == 0) {
                if (!pop(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        case 'D':
            if (strncmp(DEG_BY, *read, i) == 0) {
                if (c != ' ') {
                    fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line);
                    return true;
                }
                bool correct = true;
                bool noUnderflow = degBy(s, &correct, read, length);
                if (!correct) {
                    fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line);
                } else if (!noUnderflow) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else if (strncmp(DEG, *read, i) == 0) {
                if (!EndLine) {
                    return false;
                }
                if (!deg(s)) {
                    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
                }
            } else {
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

static void PrintMono(const Mono *m);

/**
 * Wypisuje na standarsoe wyjście wielomian @f$p@f$.
 * @param[in] p : wielomian @f$p@f$
 */
static void PrintPoly(const Poly *p) {
    if (p->arr == NULL) {
        printf("%ld", p->coeff);
    } else {
        for (size_t i = 0; i < p->size; ++i) {
            printf("(");
            PrintMono(&p->arr[i]);
            if (i != p->size - 1) {
                printf(",%d)+", p->arr[i].exp);

            } else {
                printf(",%d)", p->arr[i].exp);

            }
        }
    }
}

/**
 * Wypisuje na standarsoe wyjście jednomain @f$m@f$.
 * @param[in] m : jednomian @f$m@f$
 */
static void PrintMono(const Mono *m) {
    PrintPoly(&m->p);
}

/**
 * Wczytuje dane z wejścia, wykonuje polecenia i wypisuje kominikaty o błędach.
 */
void calculator(void) {
    Stack stack = newPolyStack();
    int c = getchar();
    int line = 1;
    char *buffer = malloc(INITIAL_LENGTH * sizeof *buffer);
    size_t length = INITIAL_LENGTH;
    while (c != EOF) {
        switch (c) {
            case '#':
                readRestOfLine();
                break;
            case '\n':
                break;
            default:
                ungetc(c, stdin);
                if (isLetter(c)) {
                    if (!readCommand(&stack, line, &buffer, &length)) {
                        fprintf(stderr, "ERROR %d WRONG COMMAND\n", line);
                    }
                    readRestOfLine();
                } else {
                    bool correct = true;
                    Poly p = readPoly(&correct, &buffer, &length);
                    c = getchar();
                    if ((c != EOF) && (c != '\n')) {
                        correct = false;
                    }
                    ungetc(c, stdin);
                    readRestOfLine();
                    if (!correct) {
                        PolyDestroy(&p);
                        fprintf(stderr, "ERROR %d WRONG POLY\n", line);
                    } else {
                        pushPoly(&stack, p);
                    }
                }
        }
        c = getchar();
        ++line;
    }
    free(buffer);
    freeStack(&stack);
}

int main(void) {
    calculator();
    return 0;
}