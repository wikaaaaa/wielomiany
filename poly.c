/** @file
  Interfejs klasy wielomianów rzadkich wielu zmiennych.
  @author Wiktoria Walczak
  @date 2021
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include "poly.h"
#include "additional_functions.h"

/** Zwraca większą wartość. */
static size_t max(size_t a, size_t b) {
    if (a < b) {
        return b;
    } else {
        return a;
    }
}

/**
 * Sumuje dwie tablice jedmonianów w jedną.
 * @param[in] p : tablica jednomianów @f$p@f$
 * @param[in] size_p : liczba jednomianów w tablicy @f$p@f$
 * @param[in] q : tablica jednomianów @f$q@f$
 * @param[in] size_q : liczba jednomianów w tablicy @f$q@f$
 * @param[in] new_size : liczba jednomianów w tablicy wynikowej
 * @return tablica jednomianów
 */
static Mono *AddTwoMonoArrays(const Mono *p, size_t size_p, const Mono *q, size_t size_q, size_t *new_size) {
    size_t length = max(size_p, size_q);
    Mono *new = malloc(length * sizeof *new);
    size_t index_p = 0;
    size_t index_q = 0;
    size_t i = 0;
    while ((index_p < size_p) && (index_q < size_q)) {
        LengthenArrayIfNecessary(&new, &length, i);
        if (p[index_p].exp == q[index_q].exp) {
            new[i].exp = p[index_p].exp;
            new[i].p = PolyAdd(&p[index_p].p, &q[index_q].p);
            ++index_p;
            ++index_q;
        } else if (p[index_p].exp < q[index_q].exp) {
            new[i].exp = p[index_p].exp;
            new[i].p = PolyClone(&p[index_p].p);
            ++index_p;
        } else {
            new[i].exp = q[index_q].exp;
            new[i].p = PolyClone(&q[index_q].p);
            ++index_q;
        }
        if (!PolyIsZero(&new[i].p)) {
            ++i;
        } else {
            PolyDestroy(&new[i].p);
        }
    }
    if (index_p < size_p) {
        while (index_p < size_p) {
            LengthenArrayIfNecessary(&new, &length, i);
            new[i].exp = p[index_p].exp;
            new[i].p = PolyClone(&p[index_p].p);
            ++i;
            ++index_p;
        }
    } else {
        while (index_q < size_q) {
            LengthenArrayIfNecessary(&new, &length, i);
            new[i].exp = q[index_q].exp;
            new[i].p = PolyClone(&q[index_q].p);
            ++i;
            ++index_q;
        }
    }
    *new_size = i;
    if (i == 0) {
        free(new);
        return NULL;
    }
    return new;
}

/**
 * Tworzy kopię tablicy jednomianów @f$m@f$ powiększoną o wyraz wolny
 * @param[in] m : tablica jednomianów @f$m@f$
 * @param[in] size : liczba jednomianów w tablicy @f$m@f$
 * @param[in] coeff : wyraz wolny
 * @return tablica jednomianów
 */
static Mono *AddExpZero(const Mono *m, size_t *size, poly_coeff_t coeff) {
    ++(*size);
    Mono *new = malloc(*size * sizeof(*new));
    new[0].exp = 0;
    new[0].p.coeff = coeff;
    new[0].p.arr = NULL;
    for (size_t i = 1; i < *size; ++i) {
        new[i] = MonoClone(&m[i - 1]);
    }
    return new;
}

static bool MonoSimplify(const Mono *m, poly_coeff_t *coeff);

/**
 * Sprawdza, czy wielomian @f$p@f$ da się uprościć do wspólczynnika.
 * Jeśli tak, zapisuje na wskaźniku wartość współczynnika.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] coeff : wskaźnik, na który zostanie zapisany wyraz wolny, jeśli wielomian da się uprościć
 * @return Czy wielomian da się uprościć?
 */
static bool PolySimplify(const Poly *p, poly_coeff_t *coeff) {
    if (p->arr == NULL) {
        *coeff = p->coeff;
        return true;
    } else {
        if (p->size != 1) {
            return false;
        }
        return MonoSimplify(&p->arr[0], coeff);
    }
}

/**
 * Sprawdza, czy jednomian @f$m@f$ da się uprościć do wspólczynnika i jeśli tak zapisuje na wskaźniku wartość współczynnika
 * @param[in] m : jedmonian @f$m@f$, którego wykładnik jest równy zeru
 * @param[in] coeff : wskaźnik, na który zostanie zapisany wyraz wolny, jeśli jednomian da się uprościć
 * @return Czy jednomian da się uprościć?
 */
static bool MonoSimplify(const Mono *m, poly_coeff_t *coeff) {
    if (m->p.arr == NULL) {
        *coeff = m->p.coeff;
        return true;
    }
    if (m->p.size != 1) {
        return false;
    }
    if (m->p.arr[0].exp != 0) {
        return false;
    }
    return (PolySimplify(&m->p, coeff));
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff + q->coeff);
    }
    Poly new;
    if (PolyIsCoeff(p)) {
        if (p->coeff == 0) {
            if (PolyIsZero(q)) {
                return PolyZero();
            }
            new = PolyClone(q);
            return new;
        }
        if (PolyHasExpZero(q)) {
            Poly temp = PolyAdd(&q->arr[0].p, p);
            if (PolyIsZero(&temp)) {
                PolyDestroy(&temp);
                new.size = q->size - 1;
                if (new.size == 0) {
                    new = PolyZero();
                } else {
                    new.arr = malloc(new.size * sizeof *new.arr);
                    for (size_t i = 0; i < new.size; ++i) {
                        new.arr[i] = MonoClone(&q->arr[i + 1]);
                    }
                }
            } else {
                new.size = q->size;
                new.arr = malloc(new.size * sizeof *new.arr);
                new.arr[0].p = temp;
                new.arr[0].exp = 0;
                for (size_t i = 1; i < new.size; ++i) {
                    new.arr[i] = MonoClone(&q->arr[i]);
                }
            }
        } else {
            size_t new_size = q->size;
            new.arr = AddExpZero(q->arr, &new_size, p->coeff);
            new.size = new_size;
        }
        return new;
    }
    if (PolyIsCoeff(q)) {
        if (q->coeff == 0) {
            if (PolyIsZero(p)) {
                return PolyZero();
            }
            new = PolyClone(p);
            return new;
        }
        if (PolyHasExpZero(p)) {
            Poly temp = PolyAdd(&p->arr[0].p, q);
            if (PolyIsZero(&temp)) {
                PolyDestroy(&temp);
                new.size = p->size - 1;
                if (new.size == 0) {
                    new = PolyZero();
                } else {
                    new.arr = malloc(new.size * sizeof *new.arr);
                    for (size_t i = 0; i < new.size; ++i) {
                        new.arr[i] = MonoClone(&p->arr[i + 1]);
                    }
                }
            } else {
                new.size = p->size;
                new.arr = malloc(new.size * sizeof *new.arr);
                new.arr[0].p = temp;
                new.arr[0].exp = 0;
                for (size_t i = 1; i < new.size; ++i) {
                    new.arr[i] = MonoClone(&p->arr[i]);
                }
            }
        } else {
            size_t new_size = p->size;
            new.arr = AddExpZero(p->arr, &new_size, q->coeff);
            new.size = new_size;
        }
        return new;
    }
    size_t new_size;
    new.arr = AddTwoMonoArrays(p->arr, p->size, q->arr, q->size, &new_size);
    new.size = new_size;
    if (new_size == 0) {
        new = PolyZero();
    } else {
        // sprawdzenie czy wielomian to nie wsółczynnik
        if ((new.arr[0].exp == 0) && (new.size == 1)) {
            poly_coeff_t coeff;
            if (MonoSimplify(&new.arr[0], &coeff)) {
                PolyDestroy(&new.arr[0].p);
                free(new.arr);
                new = PolyFromCoeff(coeff);
            }
        }

    }
    return new;
}

/**
 * Porównuje wykładniki dwóch jednomianów.
 * Zwraca 1, jeśli wykładnik @f$a@f$ jest większy od wykładnika @f$b@f$.
 * Zwraca -1 w przeciwnym przypadku.
 * Zwraca 0, jeśli wykładniki są równe.
 */
static int CompareMono(const void *_a, const void *_b) {
    Mono a = *(Mono *) _a;
    Mono b = *(Mono *) _b;
    if (a.exp > b.exp) {
        return 1;
    }
    if (a.exp < b.exp) {
        return -1;
    }
    return 0;
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count <= 0) {
        return PolyZero();
    }
    Mono *copy = malloc(count * sizeof *copy);
    for (size_t i = 0; i < count; ++i) {
        copy[i] = monos[i];
    }
    return PolyOwnMonos(count, copy);
}

Poly PolyOwnMonos(size_t count, Mono *monos) {
    if ((count <= 0) || (monos == NULL)) {
        return PolyZero();
    }
    qsort((void *) monos, count, sizeof(Mono), CompareMono);
    Poly new;
    size_t length = count;
    Mono *arr = malloc(length * sizeof *arr);
    size_t index_m = 1;
    size_t i = 1;
    if (PolyIsZero(&monos[0].p)) {
        PolyDestroy(&monos[0].p);
        if (count == 1) {
            free(monos);
            free(arr);
            return PolyZero();
        } else {
            arr[0] = monos[1];
            ++index_m;
        }
    } else {
        arr[0] = monos[0];
    }
    while (index_m < (count - 1)) {
        LengthenArrayIfNecessary(&arr, &length, i);
        Mono mono;
        if (monos[index_m].exp == monos[index_m + 1].exp) {
            Poly p = PolyAdd(&monos[index_m].p, &monos[index_m + 1].p);
            PolyDestroy(&monos[index_m].p);
            PolyDestroy(&monos[index_m + 1].p);
            mono.p = p;
            mono.exp = monos[index_m].exp;
            index_m = index_m + 2;
        } else {
            mono.p = monos[index_m].p;
            mono.exp = monos[index_m].exp;
            ++index_m;
        }
        if (!PolyIsZero(&mono.p)) {
            if (i > 0) {
                if (arr[i - 1].exp == mono.exp) {
                    Poly temp = arr[i - 1].p;
                    arr[i - 1].p = PolyAdd(&arr[i - 1].p, &mono.p);
                    PolyDestroy(&temp);
                    PolyDestroy(&mono.p);
                    if (PolyIsZero(&arr[i - 1].p)) {
                        PolyDestroy(&arr[i - 1].p);
                        --i;
                    }
                } else {
                    arr[i] = mono;
                    ++i;
                }
            } else {
                arr[i] = mono;
                ++i;
            }
        } else {
            PolyDestroy(&mono.p);
        }
    }
    if (index_m == count - 1) {
        if (arr[i - 1].exp == monos[index_m].exp) {
            Poly temp = arr[i - 1].p;
            arr[i - 1].p = PolyAdd(&arr[i - 1].p, &monos[index_m].p);
            PolyDestroy(&temp);
            PolyDestroy(&monos[index_m].p);
            if (PolyIsZero(&arr[i - 1].p)) {
                PolyDestroy(&arr[i - 1].p);
                --i;
            }
        } else {
            if (!PolyIsZero(&monos[index_m].p)) {
                LengthenArrayIfNecessary(&arr, &length, i);
                arr[i] = monos[index_m];
                ++i;
            } else {
                PolyDestroy(&monos[index_m].p);
            }
        }
    }
    new.size = i;
    if (new.size == 0) {
        free(arr);
        new = PolyZero();
    } else if ((new.size == 1) && (arr[0].exp == 0) && (arr[0].p.arr == NULL)) {
        Poly res = arr[0].p;
        free(arr);
        new = res;
    } else {
        new.arr = arr;
    }
    free(monos);
    return new;
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    if ((count <= 0) || (monos == NULL)) {
        return PolyZero();
    }
    Mono *copy = malloc(count * sizeof *copy);
    for (size_t i = 0; i < count; ++i) {
        copy[i] = MonoClone(&monos[i]);
    }
    return PolyOwnMonos(count, copy);
}

/**
 * Sprawdza, czy wielomian którego @f$arr@f$ nie jest równy NULL, jest stały.
 * @param[in] p : wielomian
 * @return Czy wielomian jest stały?
 */
bool PolyIsConst(const Poly *p) {
    if (p->size != 1) {
        return false;
    }
    if (p->arr[0].exp != 0) {
        return false;
    }
    if (p->arr[0].p.arr != NULL) {
        return PolyIsConst(&p->arr[0].p);
    }
    return true;
}

/**
 * Sprawdza równość dwóch jednomianów.
 * @param[in] m1 : jednomian @f$m_1@f$
 * @param[in] m2 : jednomian @f$m_2@f$
 * @return @f$m_1 = m_2@f$
 */
static bool MonoIsEqual(const Mono *m1, const Mono *m2) {
    if (m1->exp != m2->exp) {
        return false;
    }
    return (PolyIsEq(&m1->p, &m2->p));
}

static poly_coeff_t PolyGetCoeff(const Poly *p);

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p = q@f$
 */
bool PolyIsEq(const Poly *p, const Poly *q) {
    if ((p->arr == NULL) && (q->arr == NULL)) {
        return (p->coeff == q->coeff);
    }
    if ((p->arr == NULL) && (q->arr != NULL)) {
        if (!PolyIsConst(q)) {
            return false;
        }
        if (PolyGetCoeff(q) != p->coeff) {
            return false;
        }
        return true;
    }
    if ((p->arr != NULL) && (q->arr == NULL)) {
        if (!PolyIsConst(p)) {
            return false;
        }
        if (PolyGetCoeff(p) != q->coeff) {
            return false;
        }
        return true;
    }
    if (p->size != q->size) {
        return false;
    }
    for (size_t i = 0; i < p->size; ++i) {
        if (!MonoIsEqual(&p->arr[i], &q->arr[i])) {
            return false;
        }
    }
    return true;
}

void PolyDestroy(Poly *p) {
    if (p->arr != NULL) {
        for (size_t i = 0; i < p->size; ++i) {
            MonoDestroy(&p->arr[i]);
        }
        free(p->arr);
    }
}

/**
 * Mnoży dwa jednomiany.
 * @param[in] m1 : jednomian @f$m_1@f$
 * @param[in] m2 : jednomian @f$m_2@f$
 * @return @f$m_1 * m_2@f$
 */
static Mono MonoMul(const Mono *m1, const Mono *m2) {
    Mono new;
    new.exp = m1->exp + m2->exp;
    new.p = PolyMul(&m1->p, &m2->p);
    return new;
}

/**
 * Mnoży jednomian przez stałą.
 * @param[in] m : jednomianów @f$m@f$
 * @param[in] coeff : stała
 * @return @f$m * coeff@f$
 */
static Mono MonoCoeffMul(const Mono *m, const Poly *coeff) {
    Mono new;
    new.exp = m->exp;
    new.p = PolyMul(&m->p, coeff);
    return new;
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if ((p->arr == NULL) && (q->arr == NULL)) {
        return PolyFromCoeff(p->coeff * q->coeff);
    }
    Poly new;
    size_t index = 0;
    if ((p->arr != NULL) && (q->arr == NULL)) {
        size_t length = p->size;
        Mono *arr = malloc(length * sizeof *arr);
        if (PolyIsZero(q)) {
            free(arr);
            return PolyZero();
        }
        for (size_t index_p = 0; index_p < p->size; ++index_p) {
            LengthenArrayIfNecessary(&arr, &length, index);
            Mono temp = MonoCoeffMul(&p->arr[index_p], q);
            if (!PolyIsZero(&temp.p)) {
                arr[index] = temp;
                ++index;
            } else {
                PolyDestroy(&temp.p);
            }
        }
        if (index == 0) {
            free(arr);
            new = PolyZero();
        } else {
            new.arr = arr;
            new.size = index;
        }
        return new;
    }
    if ((q->arr != NULL) && (p->arr == NULL)) {
        size_t length = q->size;
        Mono *arr = malloc(length * sizeof *arr);
        if (PolyIsZero(p)) {
            free(arr);
            return PolyZero();
        }
        for (size_t index_q = 0; index_q < q->size; ++index_q) {
            LengthenArrayIfNecessary(&arr, &length, index);
            Mono temp = MonoCoeffMul(&q->arr[index_q], p);
            if (!PolyIsZero(&temp.p)) {
                arr[index] = temp;
                ++index;
            } else {
                PolyDestroy(&temp.p);
            }
        }
        if (index == 0) {
            free(arr);
            new = PolyZero();
        } else {
            new.arr = arr;
            new.size = index;
        }
        return new;
    }
    size_t length = max(p->size, q->size);
    Mono *arr = malloc(length * sizeof *arr);
    for (size_t index_p = 0; index_p < p->size; ++index_p) {
        for (size_t index_q = 0; index_q < q->size; ++index_q) {
            LengthenArrayIfNecessary(&arr, &length, index);
            arr[index] = MonoMul(&p->arr[index_p], &q->arr[index_q]);
            ++index;
        }
    }
    new = PolyAddMonos(index, arr);
    free(arr);
    return new;
}

/**
 * Zwraca przeciwny jednomian.
 * @param[in] m : jednomian @f$m@f$
 * @return @f$-m@f$
 */
Mono MonoNeg(const Mono *m) {
    Mono new;
    new.exp = m->exp;
    new.p = PolyNeg(&m->p);
    return new;
}

Poly PolyNeg(const Poly *p) {
    if (PolyIsZero(p)) {
        return PolyZero();
    }
    if (p->arr == NULL) {
        poly_coeff_t neg = -1;
        poly_coeff_t new = (neg * p->coeff);
        return PolyFromCoeff(new);
    }
    Mono *arr = malloc(p->size * sizeof *arr);
    for (size_t i = 0; i < p->size; ++i) {
        arr[i] = MonoNeg(&p->arr[i]);
    }
    Poly new;
    new.arr = arr;
    new.size = p->size;
    return new;
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly q_neg = PolyNeg(q);
    Poly new = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    if (PolyIsZero(&new)) {
        PolyDestroy(&new);
        return PolyZero();
    }
    return new;
}

/** Zwraca większą wartość. */
static poly_exp_t max_poly_exp_t(poly_exp_t a, poly_exp_t b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

/**
 * Zwraca stopień wielomianu (który nie jest tożsamościowo równy zeru) ze względu na zadaną zmienną.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
static poly_exp_t PolyDegByHelper(const Poly *p, size_t var_idx) {
    if (p->arr == NULL) {
        return 0;
    }
    if (var_idx == 0) {
        return p->arr[p->size - 1].exp;
    }
    poly_exp_t maximum = 0;
    for (size_t i = 0; i < p->size; ++i) {
        poly_exp_t new = PolyDegByHelper(&p->arr[i].p, var_idx - 1);
        maximum = max_poly_exp_t(maximum, new);
    }
    return maximum;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    if (PolyIsZero(p)) {
        return -1;
    }
    return PolyDegByHelper(p, var_idx);
}

/**
 * Zwraca stopień wielomianu, który nie jest tożsamościowo równy zeru.
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
static poly_exp_t PolyDegHelper(const Poly *p) {
    if (p->arr == NULL) {
        return 0;
    }
    poly_exp_t maximum = 0;
    for (size_t i = 0; i < p->size; ++i) {
        poly_exp_t new = p->arr[i].exp + PolyDegHelper(&p->arr[i].p);
        maximum = max_poly_exp_t(maximum, new);
    }
    return maximum;
}

poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsZero(p)) {
        return -1;
    }
    return PolyDegHelper(p);
}

/**
 * Sprawdza, czy @f$p@f$ jest wielomianem tylko jednej zmiennej,
 * czyli czy po wstawieniu pod pierwszą zmienną stałej, wynikiem będzię stała.
 * @param[in] p : wielomian, którego @f$arr@f$ nie jest równa NULL
 * @return Czy wielomian jest wielomianem jednej zmiennej?
 */
static bool PolyIsSimple(const Poly *p) {
    bool simple = true;
    for (size_t i = 0; ((i < p->size) && (simple)); ++i) {
        simple = (PolyDeg(&p->arr[i].p) == 0);
    }
    return simple;
}

/**
 * Zwraca współczynnik wielomianu.
 * Wywoływana, jeśli wiadomo, że współczynnikiem @f$p@f$ jest stała.
 * @param[in] p : wielomian @f$p@f$ jednej zmiennej
 * @return wspólczynnik wielomianu @f$p@f$
 */
static poly_coeff_t PolyGetCoeff(const Poly *p) {
    if (p->arr == NULL) {
        return p->coeff;
    }
    assert(p->size == 1);
    assert(p->arr[0].exp == 0);
    return PolyGetCoeff(&p->arr[0].p);
}

/**
 * Podnosi @f$x@f$ do potęgi @f$exp@f$.
 * @param[in] x : podstawa
  * @param[in] exp : wykładnik
 * @return @f$x^{exp}@f$
 */
static poly_coeff_t Power(poly_coeff_t x, poly_exp_t exp) {
    poly_coeff_t res = 1;
    while (exp > 0) {
        if (exp % 2 == 1) {
            res = res * x;
        }
        exp = exp / 2;
        x = x * x;
    }
    return res;
}

/**
 * Zwraca jednomian w formie @f$coeff * x^0@f$.
 * @param[in] coeff : współczynnik
 * @return @f$coeff * x^0@f$
 */
static Mono MonoFromCoeff(poly_coeff_t coeff) {
    Mono new;
    new.exp = 0;
    new.p.arr = NULL;
    new.p.coeff = coeff;
    return new;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (p->arr == NULL) {
        return PolyClone(p);
    }
    if (PolyIsSimple(p)) {
        poly_coeff_t wynik = 0;
        for (size_t i = 0; i < p->size; ++i) {
            poly_coeff_t coeff = PolyGetCoeff(&p->arr[i].p);
            poly_coeff_t a = Power(x, p->arr[i].exp);
            wynik += coeff * a;
        }
        return PolyFromCoeff(wynik);
    }
    size_t length = p->size;
    Mono *arr = malloc(length * sizeof *arr);
    size_t index_arr = 0;
    for (size_t index_p = 0; index_p < p->size; ++index_p) {
        LengthenArrayIfNecessary(&arr, &length, index_arr);
        if (PolyDeg(&p->arr[index_p].p) == 0) {
            poly_coeff_t coeff = PolyGetCoeff(&p->arr[index_p].p);
            poly_coeff_t a = Power(x, p->arr[index_p].exp);
            arr[index_arr] = MonoFromCoeff(coeff * a);
            ++index_arr;
        } else {
            poly_coeff_t a = Power(x, p->arr[index_p].exp);
            Poly A = PolyFromCoeff(a);
            Poly temp = PolyMul(&p->arr[index_p].p, &A);
            for (size_t index_temp = 0; index_temp < temp.size; ++index_temp, ++index_arr) {
                LengthenArrayIfNecessary(&arr, &length, index_arr);
                arr[index_arr] = temp.arr[index_temp];
            }
            free(temp.arr);
        }
    }
    Poly res = PolyAddMonos(index_arr, arr);
    free(arr);
    return res;
}

/**
 * Podnosi wielomian @f$p@f$ do potęgi @f$exp@f$.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] exp : wykładnik @f$exp@f$
 * @return @f$p^{exp}@f$
 */
static Poly PowerPoly(const Poly *p, poly_exp_t exp) {
    if (exp == 0) {
        return PolyFromCoeff(1);
    }
    if (PolyIsZero(p)) {
        return PolyZero();
    }
    Poly x = PolyClone(p);
    Poly res = PolyFromCoeff(1);
    Poly help;
    while (exp > 0) {
        if (exp % 2 == 1) {
            help = res;
            res = PolyMul(&res, &x);
            PolyDestroy(&help);
        }
        exp = exp / 2;
        help = x;
        x = PolyMul(&x, &x);
        PolyDestroy(&help);
    }
    PolyDestroy(&x);
    return res;
}

static Poly PolyComposeHelperII(const Poly *p, size_t k, const Poly q[], size_t depth);

/**
 * Funkcja pomocnicza funkcji PolyCompose.
 * @param[in] m : jednomian
 * @param[in] k : liczba wielomianów w tablicy @f$q@f$
 * @param[in] q : tablica wielomianów
 * @param[in] depth : stopień zagnieżdzenia głównego wielomianu
 */
static Poly PolyComposeHelperI(const Mono *m, size_t k, const Poly q[], size_t depth) {
    if (m->p.arr == NULL) {
        Poly coeff = m->p;
        Poly composed;
        if (depth >= k) {
            Poly help = PolyZero();
            composed = PowerPoly(&help, m->exp);
        } else {
            composed = PowerPoly(&q[depth], m->exp);
        }
        Poly res = PolyMul(&coeff, &composed);
        PolyDestroy(&composed);
        return res;
    } else {
        Poly p = PolyComposeHelperII(&m->p, k, q, depth + 1);
        Poly composed;
        if (depth >= k) {
            Poly help = PolyZero();
            composed = PowerPoly(&help, m->exp);
        } else {
            composed = PowerPoly(&q[depth], m->exp);
        }
        Poly res = PolyMul(&p, &composed);
        PolyDestroy(&p);
        PolyDestroy(&composed);
        return res;
    }
}

/**
 * Funkcja pomocnicza funkcji PolyCompose.
 * @param[in] p : wielomian
 * @param[in] k : liczba wielomianów w tablicy @f$q@f$
 * @param[in] q : tablica wielomianów
 * @param[in] depth : stopień zagnieżdzenia głównego wielomianu
 */
static Poly PolyComposeHelperII(const Poly *p, size_t k, const Poly q[], size_t depth) {
    if (p->arr == NULL) {
        return PolyClone(p);
    }
    Mono *arr = malloc(k * sizeof *arr);
    size_t length = k;
    size_t i = 0;
    size_t index_p = 0;
    while (index_p < p->size) {
        Poly composed = PolyComposeHelperI(&p->arr[index_p], k, q, depth);
        ++index_p;
        if (composed.arr == NULL) {
            LengthenArrayIfNecessary(&arr, &length, i);
            arr[i] = MonoFromPoly(&composed, 0);
            ++i;
        } else {
            for (size_t j = 0; j < composed.size; ++j, ++i) {
                LengthenArrayIfNecessary(&arr, &length, i);
                arr[i] = composed.arr[j];
            }
            free(composed.arr);
        }
    }
    if (i == 0) {
        free(arr);
        return PolyZero();
    }
    return PolyOwnMonos(i, arr);
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    if (p->arr == NULL) {
        return PolyClone(p);
    }
    Mono *arr = malloc(k * sizeof *arr);
    size_t length = k;
    size_t i = 0;
    size_t index_p = 0;
    while (index_p < p->size) {
        Poly composed = PolyComposeHelperI(&p->arr[index_p], k, q, 0);
        ++index_p;
        if (composed.arr == NULL) {
            LengthenArrayIfNecessary(&arr, &length, i);
            arr[i] = MonoFromPoly(&composed, 0);
            ++i;
        } else {
            for (size_t j = 0; j < composed.size; ++j, ++i) {
                LengthenArrayIfNecessary(&arr, &length, i);
                arr[i] = composed.arr[j];
            }
            free(composed.arr);
        }
    }
    if (i == 0) {
        free(arr);
        return PolyZero();
    }
    arr = realloc(arr, i * sizeof *arr);
    Poly res = PolyOwnMonos(i, arr);
    return res;
}
