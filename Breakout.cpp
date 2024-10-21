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
Proyecto: Breakout implementado con Phtreads
Archivo: Breakout.cpp

Descripción: Este programa simula el clásico juego Breakout,
utilizando programación multihilo mediante Phtreads para
dividir las tareas de control de la pelota, palas y bloques. 
Se empleo la biblioteca ncurses para mostrar el jeugo en la terminal. 
---------------------------------------*/

/*Bibliotecas importadas*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <ncurses.h>
#include <unistd.h>


/*Variables Globales*/

// Dimensiones de la pantalla
int ancho_pantalla = 40;
int alto_pantalla = 15;

// Dimensiones de las palas
int pala1_x = 11, pala1_y = 13;
int pala2_x = 5, pala2_y = 14;
int ancho_pala = 10;
// Variables de la pelota
int pelota_x = ancho_pantalla / 2;
int pelota_y = pala1_y - 1;
int pelota_dir_x = 1, pelota_dir_y = -1;
// Puntaje de los jugadores
int puntaje_jugador1 = 0;
int puntaje_jugador2 = 0;

//Control de las palas 
volatile int ControlP1 = ERR;
volatile int ControlP2 = ERR;

//Contador de los bloques destruidos
int blockCount = 0;  

// Estado del juego
bool ball_moving = false;

//Modo de juego (1 u 2 jugadores)
int n;

/*Clase Bloque 
Descripción: Define las características de los bloques destructibles. 
Atributos: 
- estado: Determina si el bloque está activo (1) o destruido (0).
- valorBloque: Valor en puntos que otorga el bloque al ser destruido.
- resistencia: Número de golpes que puede resistir el bloque antes de ser destruido. 
Métodos: 
- Getters y setters para manipular los atributos. 
- reducirResistencia: Reduce la resistencia del bloque en 1
*/
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

/* Matrices de bloques */
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

/* Funcion crear_bloques
Descripción: Inicializa los bloques en la matriz, asignando resistencia
ademas de el valor de cada bloque segun su nivel
*/
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
    //Sincroniza los hilos
    pthread_barrier_wait(&barrera);
    return NULL;
}

/*
Función actualizar_pantalla
Descripción: Imprimir bloques en la pantalla cada vez
que ocurre un cambio
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

    //Puntajes segun el numero de jugadores
    if (n == 1) {  //Modo de un jugador
        mvprintw(15, 0, "Puntaje jugador 1: %d", puntaje_jugador1);
    } else if (n == 2) { //Modo de dos jugadores
        mvprintw(15, 0, "Puntaje jugador 1: %d | Puntaje jugador 2: %d", puntaje_jugador1, puntaje_jugador2);
    }

    //Contador de bloques destruidos 
    mvprintw(16, 0, "Bloques destruidos: %d", blockCount);

    refresh();
}

/* destruir_bloque
Descripción: Verifica si la pelota ha chocado con un bloque 
actualiza la pantalla y el estado del bloque a muerto y 
el puntaje del jugador
Parámetros:
- x, y: coordenadas de la pelota
- idJugador: identificacion de quien destruyo el bloque
*/
bool destruir_bloque(int x, int y, int idJugador) {
    // Coliciones con bloques de resistenca 3
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n3[i][j].getEstado() == 1 && y == i && x / 2 == j) {
                matriz_n3[i][j].reducirResistencia();
                if (matriz_n3[i][j].getResistencia() == 0) {
                   pthread_mutex_lock(&points_mutex);
                    matriz_n3[i][j].setEstado(0);
                    blockCount++; 
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

    //Coliciones con bloques de resistenca 2
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n2[i][j].getEstado() == 1 && y == i + 2 && x / 2 == j) {
                matriz_n2[i][j].reducirResistencia();
                if (matriz_n2[i][j].getResistencia() == 0) {
                    pthread_mutex_lock(&points_mutex);
                    matriz_n2[i][j].setEstado(0);
                    blockCount++; 
                    if(idJugador == 2){
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

    //Coliciones con bloques de resistenca 1
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 20; j++) {
            if (matriz_n1[i][j].getEstado() == 1 && y == i + 4 && x / 2 == j) {
                matriz_n1[i][j].reducirResistencia();
                if (matriz_n1[i][j].getResistencia() == 0) {
                    pthread_mutex_lock(&points_mutex);
                    matriz_n1[i][j].setEstado(0);
                    blockCount++; 
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

/* todos_bloques_destruidos
Verificación de estados de los bloques
false: existen bloques en el juego
true: todos los bloques han sido destruidos
*/
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

/*Funcion logica_pelota
Descripción: Verifica el estado de la pelota y colisiones con 
otros elementos como pala, paredes y bloques. 
Parámetro:
- velocidad_pelota: para modificar el tiempo de pausa para que
se redibuje otra vez
*/
void *logica_pelota(void *arg) {
    int velocidad_pelota = *(int*)arg; // Recibir la velocidad de la pelota
    int num_jugador = 1;  // Variable para definir qué jugador fue el último en tocar la pelota
    int puntos_jugador1 = 0; // Puntos del jugador 1
    int puntos_jugador2 = 0; // Puntos del jugador 2

    while (!game_over) {
        pthread_mutex_lock(&ball_mutex);  // Asegura el acceso exclusivo a la pelota

        if (ball_moving) {
            // Borrar la pelota actual en la pantalla
            mvprintw(pelota_y, pelota_x, " "); 

            // Calcular nueva posición de la pelota
            int new_x = pelota_x + pelota_dir_x;
            int new_y = pelota_y + pelota_dir_y;

            // Comprobar colisión con bloques
            bool block_hit = destruir_bloque(new_x, new_y, num_jugador);
            if (block_hit) {
                // Incrementar puntos
                if (num_jugador == 1) {
                    puntos_jugador1++;
                } else if (num_jugador == 2) {
                    puntos_jugador2++;
                }

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

            // Logica del "Game Over"
            if (pelota_y >= alto_pantalla + 1) {  // Si la pelota cae por debajo de la pantalla
                game_over = true;
                clear();  // Limpia la pantalla para mostrar el mensaje

                //Modo de un jugador
                if (n == 1) {
                    mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 5, "Perdiste! Game Over!");
                }
                //Modo de dos jugadores
                else {
                    if (puntos_jugador1 > puntos_jugador2) {
                        mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Game Over, Jugador con más puntos: 1");
                    } else if (puntos_jugador2 > puntos_jugador1) {
                        mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Game Over, Jugador con más puntos: 2");
                    } else {
                        mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Empate, Game Over");
                    }
                }

                refresh();  // Refresca la pantalla para mostrar el mensaje
                break;  
            }

            //Lógica de "Ganaste"
            if (todos_bloques_destruidos()) {  // Si se destruyen todos los bloques
                game_over = true;
                clear();  

                // Modo de un jugador
                if (n == 1) {
                    mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 5, "Felicidades, Game Over!");
                }
                // Modo de dos jugadores
                else {
                    if (puntos_jugador1 > puntos_jugador2) {
                        mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Game Over, Jugador con más puntos: 1");
                    } else if (puntos_jugador2 > puntos_jugador1) {
                        mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Game Over, Jugador con más puntos: 2");
                    } else {
                        mvprintw(alto_pantalla / 2, ancho_pantalla / 2 - 7, "Empate, Game Over");
                    }
                }

                refresh();  // Refresca la pantalla para mostrar el mensaje
                break;  // Salir del bucle
            }


            // Colisión con la pala 1 (jugador 1)
            if (pelota_y == pala1_y - 1 && pelota_x >= pala1_x && pelota_x < pala1_x + ancho_pala) {
                pelota_dir_y = -1;  // Cambia la dirección de la pelota al chocar con la pala
                num_jugador = 1;
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
            if (pelota_y == pala2_y - 1 && pelota_x >= pala2_x && pelota_x < pala2_x + ancho_pala) {
                pelota_dir_y = -1;  // Cambia la dirección de la pelota al chocar con la pala
                num_jugador = 2;
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
        usleep(velocidad_pelota);  // Controlar la velocidad del juego (pausa en microsegundos)
    }
    return NULL;
}


/*Funcion handle_input
Descripción: Captura la entrada del teclado para mover
las palas y activar el movimiento de la pelota
*/
void *handle_input(void *arg)
{
    while (!game_over) {
        int ch = getch();
        if (ch != ERR) {
            if (!ball_moving) {
                ball_moving = true;
            }
            // Jugador 1
            if (ch == KEY_LEFT) {
                ControlP1 = KEY_LEFT;
            } else if (ch == KEY_RIGHT) {
                ControlP1 = KEY_RIGHT;
            // Jugador 2
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

/*Funcion logica_pala1
Descripción: Mueve la pala del jugador 1 de acuerdo con las flechas del teclado
*/
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

        // Resetear la ultima tecla
        ControlP1 = ERR;

        // Redibujar la pala en la nueva posición
        mvprintw(pala1_y, pala1_x, std::string(ancho_pala, '=').c_str());
        refresh();

        pthread_mutex_unlock(&paddle_mutex);
        usleep(30000);  // Velocidad de la pala
    }
    return NULL;
}

/*Funcion logica_pala2
Descripción: Mueve la pala del jugador 2 de acuerdo con las teclas "a" y "d"
*/
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

        // Resetear a la ultima tecla presionada
        ControlP2 = ERR;

        // Redibujar la pala en la nueva posición
        mvprintw(pala2_y, pala2_x, std::string(ancho_pala, '_').c_str());
        refresh();

        pthread_mutex_unlock(&paddle_mutex);
        usleep(30000);  // Velocidad pala
    }
    return NULL;
}

/*Funcion sumar_puntaje
*/
void *sumar_puntaje(void *arg)
{
    return NULL;
}
/*Funcion verificar_puntaje
*/
void verificar_puntaje()
{
}
/*Funcion titulo
Descripción: Imprimir el titulo del juego
*/
void titulo(){
clear();
    mvprintw(0, 0, " ____                 _                _");
    mvprintw(1, 0, "| __ ) _ __ ___  __ _| | ______  _   _| |_");
    mvprintw(2, 0, "|  _ -| '__/ _ -/ _  | |/  / _ -| | | | __|");
    mvprintw(3, 0, "| |_) | | |  __/ (_| |    < (_) | |_| | |_ ");
    mvprintw(4, 0, "|____/|_|  -___|-__,_|_|-__-___/ -__,_|-__|");
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
   // Inicalizacion de ncurses
    initscr();            // inicio del modo ncurses
    cbreak();
    keypad(stdscr, TRUE); // Habilitar el uso de flechas 
    curs_set(0);          // Esconder el cursor
    noecho();


    n = 0; //opcion de jugador
    // Dimensiones de terminal
    int ancho = 40;
    int alto = 25;

    int modo; //opcion de dificultad
    int velocidad_pelota;

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

    // Limpiar la pantalla y mostrar el titulo
    titulo();
    mvprintw(6, 0, "Instrucciones del juego:");
    mvprintw(7, 0, "1. Usa las flechas izquierda y derecha para mover la pala 1 (jugador 1).");
    mvprintw(8, 0, "2. El jugador 2 utiliza las teclas 'A' y 'D' para mover la pala 2.");
    mvprintw(9, 0, "3. El objetivo es evitar que la pelota caiga usando las palas.");
    mvprintw(10, 0, "4. Destruye los bloques con la pelota para sumar puntos.");
    mvprintw(11, 0, "5. Ganas si destruyes todos los bloques.");
    mvprintw(12, 0, "6. Pierdes si la pelota cae fuera de la pantalla.");
    mvprintw(13, 0, "¡Buena suerte!");
    mvprintw(14, 0, "Presiona cualquier tecla para comenzar...");
    refresh();
    getch();
    clear();

    titulo();
    // Preguntar modo de juego
    mvprintw(6, 0, "Seleccione el modo de juego: \n");
    mvprintw(7, 0, "1. Un jugador \n");
    mvprintw(8, 0, "2. Dos jugadores\n");
    mvprintw(9, 0, ">> ");
    refresh();

     // Leer entrada para el número de jugadores
    if (scanw("%d", &n) != 1 || (n != 1 && n != 2)) {
        mvprintw(10, 0, "Error: Entrada no válida.");
        refresh();
        endwin(); // terminar ncurses
        return 1;
    }

    // Preguntar la dificultad
    mvprintw(11,0, "Seleccione la dificultad: \n");
    mvprintw(12, 0, "1. Modo Fácil\n");
    mvprintw(13, 0, "2. Modo Difícil\n");
    mvprintw(14, 0, ">> ");
    refresh();

     // Leer entrada para el modo de juego
    if (scanw("%d", &modo) != 1 || (modo != 1 && modo != 2)) {
        mvprintw(15, 0, "Error: Entrada no válida.");
        refresh();
        endwin(); // terminar ncurses
        return 1;
    }

    // Determinar la velocidad en función de la dificultad
    if (modo == 1) {
        velocidad_pelota = 300000; // Modo Fácil 
    } else {
        velocidad_pelota = 150000; // Modo Difícil 
    }

    // Inicializar hilos y variables 
    pthread_t hilo_pala1, hilo_pala2, hilo_pelota, hilo_bloques, hilo_input;
    int id_pala1 = 1, id_pala2 = 2, id_bloques = 4;


    pthread_mutex_init(&ball_mutex, NULL);
    pthread_mutex_init(&paddle_mutex, NULL);
    pthread_mutex_init(&points_mutex, NULL);
    pthread_barrier_init(&barrera, NULL, 2);
    
    // Hilo que controla los inputs del usuario
    pthread_create(&hilo_input, NULL, handle_input, NULL);

    // Crear hilo pala 1
    pthread_create(&hilo_pala1, NULL, logica_pala1, (void *)&id_pala1);

    // Crear hilo pelota
    pthread_create(&hilo_pelota, NULL, logica_pelota, (void *)&velocidad_pelota);

    // Crear hilo bloques
    pthread_create(&hilo_bloques, NULL, crear_bloques, (void *)&id_bloques);

   // pthread_create(&hilo_pelota, NULL, logica_pelota, NULL);        //PRUEBA2


    if (n == 2) {
        // Si el modo es de dos jugadores crear hilo pala 2
        pthread_create(&hilo_pala2, NULL, logica_pala2, (void *)&id_pala2);
    }

    //  Esperar que se sincronicen los hilos 
    pthread_barrier_wait(&barrera);

    actualizar_pantalla();

    // Unir hilos y verificar que terminen
    pthread_join(hilo_input, NULL);
    pthread_join(hilo_pala1, NULL);
    pthread_join(hilo_pelota, NULL);
    pthread_join(hilo_bloques, NULL);

    if (n == 2) {
        pthread_join(hilo_pala2, NULL);
    }

    // Limpiar recursos
    pthread_mutex_destroy(&ball_mutex);
    pthread_mutex_destroy(&paddle_mutex);
    pthread_mutex_destroy(&points_mutex);
    pthread_barrier_destroy(&barrera);

    // Esperar una tecla para salir
    wgetch(ventana);

    // Limpiar y cerrar
    delwin(ventana);
    endwin();  // Terminar ncurses

    return 0;
}