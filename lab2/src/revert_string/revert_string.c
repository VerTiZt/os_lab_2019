#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
	int len = strlen(str); // длина строки
	
    // идем до середины строки и меняем местами противоположные символы
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i]; // сохраняем текущий левый символ
        str[i] = str[len - 1 - i]; // на его место ставим симметричный правый
        str[len - 1 - i] = temp; // на место правого ставим сохраненный левый
    }
}

