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

int ancho_pantalla = 40;
int alto_pantalla = 15;

int pala1_x = 11, pala1_y = 13;
int pala2_x = 5, pala2_y = 14;

int ancho_pala = 10;

int pelota_x = ancho_pantalla / 2;
int pelota_y = pala1_y - 1;
int pelota_dir_x = 1, pelota_dir_y = -1;

int puntaje_jugador1 = 0;
int puntaje_jugador2 = 0;

volatile int ControlP1 = ERR;
volatile int ControlP2 = ERR;

bool ball_moving = false;

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
// bloques -
Bloque matriz_n1[3][20];

bool game_over = false;

pthread_mutex_t ball_mutex;
pthread_mutex_t paddle_mutex;
pthread_mutex_t points_mutex;
pthread_barrier_t barrera;

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

    // impresion bloques - -> nivel 1
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (matriz_n1[i][j].getEstado() == 1)
            {
                mvprintw(i + 4, j * 2, "-");
            }
        }
    }

    mvprintw(15 , 0, "Puntaje jugador 1: %d | Puntaje jugador 2: %d", puntaje_jugador1, puntaje_jugador2);
    refresh();

}

bool destruir_bloque(int x, int y, int idJugador) {
    // Coliciones con bloques de resistenca 3
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n3[i][j].getEstado() == 1 && y == i && x / 2 == j) {
                matriz_n3[i][j].reducirResistencia();
                if (matriz_n3[i][j].getResistencia() == 0) {
                   pthread_mutex_lock(&points_mutex);
                    matriz_n3[i][j].setEstado(0);
                    if(idJugador == 1){
                        puntaje_jugador1 += matriz_n3[i][j].getValorBloque();
                    }else{
                        puntaje_jugador2 += matriz_n3[i][j].getValorBloque();
                    }
                    actualizar_pantalla();
                    pthread_mutex_unlock(&points_mutex);
                }
                return true;
            }
        }
    }

    // Coliciones con bloques de resistenca 2
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n2[i][j].getEstado() == 1 && y == i + 2 && x / 2 == j) {
                matriz_n2[i][j].reducirResistencia();
                if (matriz_n2[i][j].getResistencia() == 0) {
                    pthread_mutex_lock(&points_mutex);
                    matriz_n2[i][j].setEstado(0);
                    if(idJugador == 1){
                        puntaje_jugador1 += matriz_n2[i][j].getValorBloque();
                    }else{
                        puntaje_jugador2 += matriz_n2[i][j].getValorBloque();
                    }
                    actualizar_pantalla();
                    pthread_mutex_unlock(&points_mutex);
                }
                return true;
            }
        }
    }

    // Coliciones con bloques de resistenca 1
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n1[i][j].getEstado() == 1 && y == i + 4 && x / 2 == j) {
                matriz_n1[i][j].reducirResistencia();
                if (matriz_n1[i][j].getResistencia() == 0) {
                    pthread_mutex_lock(&points_mutex);
                    matriz_n1[i][j].setEstado(0);
                    if(idJugador == 1){
                        puntaje_jugador1 += matriz_n1[i][j].getValorBloque();
                    }else{
                        puntaje_jugador2 += matriz_n1[i][j].getValorBloque();
                    }
                    actualizar_pantalla();
                    pthread_mutex_unlock(&points_mutex);
                }
                return true;
            }
        }
    }

    return false;
}

bool todos_bloques_destruidos() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n3[i][j].getEstado() == 1 || matriz_n2[i][j].getEstado() == 1 || matriz_n1[i][j].getEstado() == 1) {
                return false;  // Aún hay bloques en pie
            }
        }
    }
    return true;  // Todos los bloques han sido destruidos
}


void *logica_pelota(void *arg) {
    while (!game_over) {
        pthread_mutex_lock(&ball_mutex);  // Asegura el acceso exclusivo a la pelota

        if (ball_moving) {
            // Borrar la pelota actual en la pantalla
            mvprintw(pelota_y, pelota_x, " "); 

            // Calcular nueva posición de la pelota
            int new_x = pelota_x + pelota_dir_x;
            int new_y = pelota_y + pelota_dir_y;

            // Comprobar colisión con bloques
            bool block_hit = destruir_bloque(new_x, new_y, 1);

            if (block_hit) {
                // Cambiar dirección basado en desde dónde vino la pelota
                pelota_dir_y *= -1;
            } else {
                // Si no hay colisión con bloques, actualizar posición
                pelota_x = new_x;
                pelota_y = new_y;
            }

            // Colisiones con las paredes laterales
            if (pelota_x <= 0 || pelota_x >= ancho_pantalla - 1) {
                pelota_dir_x *= -1;  // Rebota en las paredes izquierda y derecha
            }
            // Colisión con la parte superior
            if (pelota_y <= 0) {
                pelota_dir_y *= -1;  // Rebota en la parte superior
            }

            // Lógica del "Game Over"
            if (pelota_y >= alto_pantalla + 2) {  // Si la pelota cae por debajo de la pantalla
                game_over = true;
                clear();  // Limpia la pantalla para mostrar el mensaje
                mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 5, "Perdiste! Game Over!");
                refresh();  // Refresca la pantalla para mostrar el mensaje
                break;  // Salir del bucle
            }

            // Lógica de "Ganaste"
            if (todos_bloques_destruidos()) {  // Si se destruyen todos los bloques
                game_over = true;
                clear();  // Limpia la pantalla para mostrar el mensaje
                mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Ganaste! Game Over!");
                refresh();  // Refresca la pantalla para mostrar el mensaje
                break;  // Salir del bucle
            }

            // Colisión con la pala 1 (jugador 1)
            if (pelota_y == pala1_y - 1 && pelota_x >= pala1_x && pelota_x < pala1_x + ancho_pala) {
                pelota_dir_y = -1;  // Cambia la dirección de la pelota al chocar con la pala
                // Cambiar dirección horizontal basado en dónde golpea la pelota en la pala
                int hit_position = pelota_x - pala1_x;
                if (hit_position < ancho_pala / 3) {
                    pelota_dir_x = -1;
                } else if (hit_position > (2 * ancho_pala) / 3) {
                    pelota_dir_x = 1;
                } else {
                    pelota_dir_x = 0;
                }
            }

            // Colisión con la pala 2 (jugador 2)
            if (pelota_y == pala2_y + 1 && pelota_x >= pala2_x && pelota_x < pala2_x + ancho_pala) {
                pelota_dir_y = 1;  // Cambia la dirección de la pelota al chocar con la pala
                // Cambiar dirección horizontal basado en dónde golpea la pelota en la pala
                int hit_position = pelota_x - pala2_x;
                if (hit_position < ancho_pala / 3) {
                    pelota_dir_x = -1;
                } else if (hit_position > (2 * ancho_pala) / 3) {
                    pelota_dir_x = 1;
                } else {
                    pelota_dir_x = 0;
                }
            }

            // Dibujar la pelota en su nueva posición
            mvprintw(pelota_y, pelota_x, "O"); 
            refresh();  // Refrescar la pantalla
        }

        pthread_mutex_unlock(&ball_mutex);  // Liberar el mutex
        usleep(300000);  // Controlar la velocidad del juego (pausa en microsegundos)
    }
    return NULL;
}

void *handle_input(void *arg)
{
    while (!game_over) {
        int ch = getch();
        if (ch != ERR) {
            if (!ball_moving) {
                ball_moving = true;
            }
            if (ch == KEY_LEFT) {
                ControlP1 = KEY_LEFT;
            } else if (ch == KEY_RIGHT) {
                ControlP1 = KEY_RIGHT;
            } else if (ch == 'a') {
                ControlP2 = 'a';
            } else if (ch == 'd') {
                ControlP2 = 'd';
            }
        }
        usleep(1000);
    }
    return NULL;
}

void *logica_pala1(void *arg)
{
    while (!game_over) {
        pthread_mutex_lock(&paddle_mutex);

        // Borrar la pala actual
        mvprintw(pala1_y, pala1_x, std::string(ancho_pala, ' ').c_str());

        // Mover la pala 1 con las teclas de flecha
        if (ControlP1 == KEY_LEFT && pala1_x > 0) {
            pala1_x -= 1;
        } else if (ControlP1 == KEY_RIGHT && pala1_x < ancho_pantalla - ancho_pala) {
            pala1_x += 1;
        }

        // Reset last key pressed
        ControlP1 = ERR;

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

        // Mover la pala 2 con las teclas 'A' y 'D'
        if (ControlP2 == 'a' && pala2_x > 0) {
            pala2_x -= 1;
        } else if (ControlP2 == 'd' && pala2_x < ancho_pantalla - ancho_pala) {
            pala2_x += 1;
        }

        // Reset last key pressed
        ControlP2 = ERR;

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
    cbreak();
    keypad(stdscr, TRUE); // Enable special keys like arrow keys
    curs_set(0);          // Hide the cursor
    noecho();

    int n = 0;
    int ancho = 40;
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
    mvprintw(0, 0, " ____                 _                _");
    mvprintw(1, 0, "| __ ) _ __ ___  __ _| | ______  _   _| |_");
    mvprintw(2, 0, "|  _ -| '__/ _ -/ _  | |/  / _ -| | | | __|");
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
    pthread_t hilo_pala1, hilo_pala2, hilo_pelota, hilo_bloques, hilo_input;
    int id_pala1 = 1, id_pala2 = 2, id_pelota = 3, id_bloques = 4;


    pthread_mutex_init(&ball_mutex, NULL);
    pthread_mutex_init(&paddle_mutex, NULL);
    pthread_mutex_init(&points_mutex, NULL);

    pthread_barrier_init(&barrera, NULL, 2);
    
    // Hilo que controla los inputs del usuario
    pthread_create(&hilo_input, NULL, handle_input, NULL);

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
    pthread_join(hilo_input, NULL);
    pthread_join(hilo_pala1, NULL);
    pthread_join(hilo_pelota, NULL);
    pthread_join(hilo_bloques, NULL);

    if (n == 2) {
        pthread_join(hilo_pala2, NULL);
    }

    // Clean up resources
    pthread_mutex_destroy(&ball_mutex);
    pthread_mutex_destroy(&paddle_mutex);
    pthread_mutex_destroy(&points_mutex);
    pthread_barrier_destroy(&barrera);

    // Esperar una tecla para salir
    wgetch(ventana);

    // Limpiar y cerrar
    delwin(ventana);
    endwin();  // Properly end ncurses

    return 0;
}