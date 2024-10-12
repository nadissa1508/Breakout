/*---------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3056 - Programacion de Microprocesadores
Colaboradores:

Jorge Luis Felipe Aguilar Portillo - 23195
Vianka Vanessa Castro Ordoñez - 23201
Genser Andree Catalan Espina - 23401
Ricardo Arturo Godínez Sanchez - 23247
Angie Nadissa Vela Lopez - 23764

Fecha: 10/../2019

Breakout.cpp
Programa que simula el juego Breakout implementando programación
paralela pór medio de Pthreads
......
---------------------------------------*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <ncurses.h>
#include <unistd.h>

int ancho_pantalla = 80;
int alto_pantalla = 24;

int pala1_x = 30, pala1_y = 20;
int pala2_x = 30, pala2_y = 20;

int ancho_pala = 10;

int pelota_x = 40, pelota_y = 12;
int pelota_dir_x = 1, pelota_dir_2 = 1;

int puntaje_jugador1 = 0;
int puntaje_jugador2 = 0;

class Bloque
{
private:
    int estado;
    int valorBloque;
    int resistencia;

public:
    int getEstado() const { return estado; }
    int getResistencia() const { return resistencia; }
    int getValorBloque() const { return valorBloque; }

    void setEstado(int est) { estado = est; }
    void setResistencia(int res) { resistencia = res; }
    void setValorBloque(int valor) { valorBloque = valor; }

    void reducirResistencia() { resistencia--; }
};

// bloques +
Bloque matriz_n3[2][20];
// bloques #
Bloque matriz_n2[2][20];
// bloques ▄
Bloque matriz_n1[3][20];

bool game_over = false;

pthread_mutex_t ball_mutex;
pthread_mutex_t paddle_mutex;
pthread_barrier_t barrera;

void *dibujar_palas()
{
    return NULL;
}

void *crear_bloques(void *arg)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            matriz_n3[i][j].setEstado(1);
            matriz_n3[i][j].setResistencia(3);
            matriz_n3[i][j].setValorBloque(3);

            matriz_n2[i][j].setEstado(1);
            matriz_n2[i][j].setResistencia(2);
            matriz_n2[i][j].setValorBloque(2);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            matriz_n1[i][j].setEstado(1);
            matriz_n1[i][j].setResistencia(1);
            matriz_n1[i][j].setValorBloque(1);
        }
    }
    pthread_barrier_wait(&barrera);
    return NULL;
}

void *destruir_bloque(void *arg)
{
    return NULL;
}

void *logica_pelota(void *arg)
{
    return NULL;
}

void *logica_pala1(void *arg)
{
    return NULL;
}

void *logica_pala2(void *arg)
{
    return NULL;
}

void *sumar_puntaje(void *arg)
{
    return NULL;
}

void verificar_puntaje()
{
}

/*
Función para imprimir bloques en la pantalla
*/
void actualizar_pantalla()
{
    // impresion bloques + -> nivel 3
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n3[i][j].getEstado() == 1)
            {
                printf("+ ");
            }
        }
        printf("\n");
    }

    // impresion bloques # -> nivel 2
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n2[i][j].getEstado() == 1)
            {
                printf("# ");
            }
        }
        printf("\n");
    }

    // impresion bloques ▄ -> nivel 1
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n1[i][j].getEstado() == 1)
            {
                printf("▄ ");
            }
        }
        printf("\n");
    }
}

/*

  ____                 _               _   
 | __ ) _ __ ___  __ _| | _____  _   _| |_ 
 |  _ \| '__/ _ \/ _` | |/ / _ \| | | | __|
 | |_) | | |  __/ (_| |   < (_) | |_| | |_ 
 |____/|_|  \___|\__,_|_|\_\___/ \__,_|\__|
                                           
*/

int main()
{
    int n = 0;
    printf("\n ____                 _               _");   
    printf("\n| __ ) _ __ ___  __ _| | ______  _   _| |_"); 
    printf("\n|  _ -| '__/ _ -/ _` | |/  / _ -| | | | __|");
    printf("\n| |_) | | |  __/ (_| |    < (_) | |_| | |_ ");
    printf("\n|____/|_|  -___|-__,_|_|-__-___/ -__,_|-__|");
                                           

    printf("\n\nSeleccione el modo de juego: \n1. Un jugador \n2. Dos jugadores");
    printf("\n>>");
    scanf("%d", &n);

    if (n == 1 || n == 2)
    {
        pthread_t hilo_pala1, hilo_pala2, hilo_pelota, hilo_bloques;
        int id_pala1 = 1, id_pala2 = 2, id_pelota = 3, id_bloques = 4;

        pthread_barrier_init(&barrera, NULL, 2);

        // crear hilo pala 1
        pthread_create(&hilo_pala1, NULL, logica_pala1, (void *)&id_pala1);

        // crear hilo pelota
        pthread_create(&hilo_pelota, NULL, logica_pelota, (void *)&id_pelota);

        // crear hilo bloques
        pthread_create(&hilo_bloques, NULL, crear_bloques, (void *)&id_bloques);

        if (n == 2)
            pthread_create(&hilo_pala2, NULL, logica_pala2, (void *)&id_pala2);

        // ciclo while debe descomentarse al avanzar con la lógica del juego
        // while(!game_over){
        pthread_barrier_wait(&barrera);

        // llamar acá metodo para actualizar la pantalla
        actualizar_pantalla();
        /*Acá se debe agregar, lógica del movimiento de la pelota, lógica del movimiento de las palas
        actualizacion de colisiones, y puede ser verificacion de si la pelota toco ell borde inferior
        o verificar si ya se eliminaron todos los bloques
        */
        //  }

        // joins

        pthread_join(hilo_pala1, NULL);
        pthread_join(hilo_pelota, NULL);
        pthread_join(hilo_bloques, NULL);

        if (n == 2)
            pthread_join(hilo_pala2, NULL);
    }
    else
    {
        printf("\nError, ingrese una opción valida");
    }

    // Destuir mutex
    pthread_mutex_destroy(&ball_mutex);
    pthread_mutex_destroy(&paddle_mutex);
    pthread_barrier_destroy(&barrera);

    return 0;
}