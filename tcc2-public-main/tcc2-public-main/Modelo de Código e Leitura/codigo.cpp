#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <time.h>

#include "header.h"

#define MAX(X, Y) ((X > Y) ? X : Y)
#define MIN(X, Y) ((X < Y) ? X : Y)

int main()
{
    st_solucao s;
    char arq[50];

    return 0;
}

void calcFO(st_solucao &s)
{
    s.funObj = 0;
    for (int j = 0; j < num_rodadas; j++)
    {
        for (int i = 0; i < num_times; i++)
        {
            s.funObj += dist[i][j];
            // decidir quais as penalizações que serão feitas na FO e quais restrições o algortimo irá tratar na construção de uma solução.
        }
    }
}