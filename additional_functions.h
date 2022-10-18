/** @file
  Implementacja funkcji pomocniczych.
  @author Wiktoria Walczak
  @date 2021
*/

#ifndef POLYNOMIALS_ADDITIONAL_FUNCTIONS_H
#define POLYNOMIALS_ADDITIONAL_FUNCTIONS_H

#include "poly.h"

/**
 * Powiększa, jeśli trzeba, tablicę typu char.
 * @param[in,out] text : tablica
 * @param[in,out] length : długość tablicy
 * @param[in] i : indeks, który ma się mieścić w tablicy
 */
void lengthenIfNecessary(char **text, size_t *length, size_t i);

/** Powiększa, jeśli trzeba, tablice typu Mono.
 * @param[in,out] arr : tablica
 * @param[in,out] length : długość tablicy
 * @param[in] i : indeks, który ma się mieścić w tablicy
 */
void LengthenArrayIfNecessary(Mono **arr, size_t *length, size_t i);

/** Sprawdza wynik realokacji pamięci. */
void CheckReallocOutcome(const void *a);

/** Powiększa liczbę.
 * @param[in] length : początkowa liczba
 * @return @f$length * 2 + 1@f$
 */
size_t more(size_t length);

#endif //POLYNOMIALS_ADDITIONAL_FUNCTIONS_H
