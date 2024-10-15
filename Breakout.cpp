/*---------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3056 - Programacion de Microprocesadores
Colaboradores:

Jorge Luis Felipe Aguilar Portillo - 23195
Ricardo Arturo Godínez Sanchez - 23247
Vianka Vanessa Castro Ordoñez - 23201
Genser Andree Catalan Espina - 23401
Angie Nadissa Vela Lopez - 23764

Fecha: 10/../2024
Breakout.cpp
Programa que simula el juego Breakout implementando programación
paralela pór medio de Pthreads
---------------------------------------*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <ncurses.h>
#include <unistd.h>

int ancho_pantalla = 80;
int alto_pantalla = 24;

int pala1_x = 11, pala1_y = 11;
int pala2_x = 5, pala2_y = 12;

int ancho_pala = 10;

int pelota_x = 40, pelota_y = 12;
int pelota_dir_x = 1, pelota_dir_y = 1;

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


//Definicion de quien es la pala y dibujarla
void mover_pala(int jugador, int &pala_x, int pala_y, char pala_char) {
    while (!game_over) {
        pthread_mutex_lock(&paddle_mutex);

        // Crear un array que represente la pala
        char pala[ancho_pala];
        for (int i = 0; i < ancho_pala; ++i) {
            pala[i] = pala_char;
        }

        // Limpiar la pala actual de la pantalla
        mvprintw(pala_y, pala_x, std::string(ancho_pala, ' ').c_str());

        // Detectar el jugador y ajustar la posición si se presiona una tecla
        int key = getch();
        if (jugador == 1) {  // Jugador 1 con flechas
            if (key == KEY_LEFT && pala_x > 0) {
                pala_x -= 1;
            } else if (key == KEY_RIGHT && pala_x < ancho_pantalla - ancho_pala) {
                pala_x += 1;
            }
        } else if (jugador == 2) {  // Jugador 2 con 'A' y 'D'
            if (key == 'a' && pala_x > 0) {
                pala_x -= 1;
            } else if (key == 'd' && pala_x < ancho_pantalla - ancho_pala) {
                pala_x += 1;
            }
        }

        // Redibujar la pala en su nueva posición
        mvprintw(pala_y, pala_x, std::string(pala, pala + ancho_pala).c_str());
        refresh();

        pthread_mutex_unlock(&paddle_mutex);
        usleep(30000);  // Delay to control frame rate
    }
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

void *logica_pelota(void *arg) {
    while (!game_over) {
        pthread_mutex_lock(&ball_mutex);

        //Borrar la pelota actual
        mvprintw(pelota_y, pelota_x, " "); 

        //Actualiza la posicion de la pelota
        pelota_x += pelota_dir_x;
        pelota_y += pelota_dir_y;

        //Colisiones con las paredes
        if (pelota_x <= 0 || pelota_x >= ancho_pantalla - 1) {
            pelota_dir_x *= -1; 
        }
        if (pelota_y <= 0) {
            pelota_dir_y *= -1; 
        }
        if (pelota_y >= alto_pantalla) {
            game_over = true;
        }

        //Dibujar la pelota en su nueva posición
        mvprintw(pelota_y, pelota_x, "O"); 
        refresh();

        pthread_mutex_unlock(&ball_mutex);
        usleep(30000);  
    }
    return NULL;
}

void *logica_pala1(void *arg)
{
    while (!game_over) {
        pthread_mutex_lock(&paddle_mutex);

        // Borrar la pala actual
        mvprintw(pala1_y, pala1_x, std::string(ancho_pala, ' ').c_str());

        int key = getch();
        // Mover la pala 1 con las teclas de flecha
        if (key == KEY_LEFT && pala1_x > 0) {
            pala1_x -= 1;
        } else if (key == KEY_RIGHT && pala1_x < ancho_pantalla - ancho_pala) {
            pala1_x += 1;
        }

        // Redibujar la pala en la nueva posición
        mvprintw(pala1_y, pala1_x, std::string(ancho_pala, '=').c_str());
        refresh();

        pthread_mutex_unlock(&paddle_mutex);
        usleep(30000);  // Control del frame rate
    }
    return NULL;
}

void *logica_pala2(void *arg)
{
    while (!game_over) {
        pthread_mutex_lock(&paddle_mutex);

        // Borrar la pala actual
        mvprintw(pala2_y, pala2_x, std::string(ancho_pala, ' ').c_str());

        int key = getch();
        // Mover la pala 2 con las teclas 'A' y 'D'
        if (key == 'a' && pala2_x > 0) {
            pala2_x -= 1;
        } else if (key == 'd' && pala2_x < ancho_pantalla - ancho_pala) {
            pala2_x += 1;
        }

        // Redibujar la pala en la nueva posición
        mvprintw(pala2_y, pala2_x, std::string(ancho_pala, '_').c_str());
        refresh();

        pthread_mutex_unlock(&paddle_mutex);
        usleep(30000);  // Control del frame rate
    }
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
    clear();

    // impresion bloques + -> nivel 3
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n3[i][j].getEstado() == 1)
            {
                mvprintw(i , j*2, "+ ");
            }
        }
        //printf("\n");
    }

    // impresion bloques # -> nivel 2
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n2[i][j].getEstado() == 1)
            {
                mvprintw(i + 2, j * 2, "# ");
            }
        }
      
    }

    // impresion bloques ▄ -> nivel 1
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n1[i][j].getEstado() == 1)
            {
                mvprintw(i + 4, j * 2, "-");
            }
        }
       // printf("\n");
    }

   
    refresh();
    


}

/*

  ____                 _               _   
 | __ ) _ __ ___  __ _| | _____  _   _| |_ 
 |  _ \| '__/ _ \/ _ | |/ / _ \| | | | __|
 | |_) | | |  __/ (_| |   < (_) | |_| | |_ 
 |____/|_|  \___|\__,_|_|\_\___/ \__,_|\__|
                                           
*/

int main()
{ 
   // Initialize ncurses
    initscr();            // Start ncurses mode
    cbreak();             // Disable line buffering
    keypad(stdscr, TRUE); // Enable special keys like arrow keys
    noecho();             // Don't show typed characters
    curs_set(0);          // Hide the cursor

    int n = 0;
    int ancho = 80;
    int alto = 25;

     // Obtener el tamaño actual de la terminal
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Verificar si la terminal es lo suficientemente grande
    if (max_y < alto || max_x < ancho) {
        endwin();
        printf("La terminal es demasiado pequeña. Por favor, ajústala a al menos %dx%d.\n", ancho, alto);
        return 1;
    }

    // Crear una nueva ventana centrada
    WINDOW *ventana = newwin(alto, ancho, (max_y - alto) / 2, (max_x - ancho) / 2);
    box(ventana, 0, 0);     // Dibuja un borde alrededor de la ventana
    wrefresh(ventana);      // Actualiza la ventana para mostrar los cambios

    // Clear screen and display title
    clear();
    mvprintw(0, 0, " ____                 _               _");
    mvprintw(1, 0, "| __ ) _ __ ___  __ _| | ______  _   _| |_");
    mvprintw(2, 0, "|  _ -| '__/ _ -/ _ | |/  / _ -| | | | __|");
    mvprintw(3, 0, "| |_) | | |  __/ (_| |    < (_) | |_| | |_ ");
    mvprintw(4, 0, "|____/|_|  -___|-__,_|_|-__-___/ -__,_|-__|");
    refresh(); 

    // Ask for game mode
    mvprintw(6, 0, "Seleccione el modo de juego: \n");
    mvprintw(7, 0, "1. Un jugador \n");
    mvprintw(8, 0, "2. Dos jugadores\n");
    mvprintw(9, 0, ">> ");
    refresh();

    // Validate input
    if (scanw("%d", &n) != 1 || (n != 1 && n != 2)) {
        mvprintw(10, 0, "Error: Entrada no válida.");
        refresh();
        endwin(); // Properly end ncurses
        return 1;
    }

    // Initialize threads and variables
    pthread_t hilo_pala1, hilo_pala2, hilo_pelota, hilo_bloques;
    int id_pala1 = 1, id_pala2 = 2, id_pelota = 3, id_bloques = 4;

    pthread_barrier_init(&barrera, NULL, 2);
    pthread_mutex_init(&ball_mutex, NULL);      //PRUEBA


    // Create paddle 1 thread
    pthread_create(&hilo_pala1, NULL, logica_pala1, (void *)&id_pala1);

    // Create ball thread
    pthread_create(&hilo_pelota, NULL, logica_pelota, (void *)&id_pelota);

    // Create blocks thread
    pthread_create(&hilo_bloques, NULL, crear_bloques, (void *)&id_bloques);

    pthread_create(&hilo_pelota, NULL, logica_pelota, NULL);        //PRUEBA2


    if (n == 2) {
        // Create paddle 2 thread for two-player mode
        pthread_create(&hilo_pala2, NULL, logica_pala2, (void *)&id_pala2);
    }

    // Wait for all threads to synchronize (e.g., block creation)
    pthread_barrier_wait(&barrera);

    // Game logic and display updates (to be implemented)
    actualizar_pantalla();

    // Join threads to ensure they complete
    pthread_join(hilo_pala1, NULL);
    pthread_join(hilo_pelota, NULL);
    pthread_join(hilo_bloques, NULL);

    if (n == 2) {
        pthread_join(hilo_pala2, NULL);
    }

    // Clean up resources
    pthread_mutex_destroy(&ball_mutex);
    pthread_mutex_destroy(&paddle_mutex);
    pthread_barrier_destroy(&barrera);

    // Esperar una tecla para salir
    wgetch(ventana);

    // Limpiar y cerrar
    delwin(ventana);
    endwin();  // Properly end ncurses

    return 0;
}