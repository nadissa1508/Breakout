# Breakout

**Proyecto No. 3** del curso **Programación de Microprocesadores**. El programa consiste en simular el juego Breakout implementando paralelización para optimizar el funcionamiento del juego.

## Descripción del Proyecto

Este proyecto implementa un juego de Breakout en el que dos jugadores controlan palas para hacer rebotar una pelota y destruir bloques. Se utiliza la librería **ncurses** para el manejo de la interfaz gráfica en terminal, y **pthreads** para la paralelización de diferentes componentes del juego como el control de las palas, la lógica de la pelota y la creación de bloques.

## Objetivos

1. Implementar un juego tipo Breakout con bloques destructibles y movimiento de palas controlado por dos jugadores.
2. Utilizar **pthreads** para la paralelización del movimiento de las palas, la lógica de la pelota y la actualización de la pantalla.
3. Manejar la sincronización y la mutua exclusión en la manipulación de los bloques, puntajes y la pantalla.

## Bibliotecas Importadas

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
```

## Instalación
Para ejecutar el programa, es necesario contar con las siguientes herramientas:

- Compilador GCC
- Librería ncurses

## Instrucciones de Compilación
Compila el programa utilizando el siguiente comando:
```c
gcc -o breakout Breakout.cpp -lncurses -lpthread
```
Ejecuta el programa:
```c
./breakout
```

## Controles del Juego
- Jugador 1: Controla su pala con las teclas de flecha izquierda (←) y derecha (→).
- Jugador 2: Controla su pala con las teclas A (izquierda) y D (derecha).

## Colaboradores
- Angie Nadissa Vela Lopez - 23764
- Genser Andree Catalan Espina - 23401
- Vianka Vanessa Castro Ordoñez - 23201
- Ricardo Arturo Godínez Sanchez - 23247
- Jorge Luis Felipe Aguilar Portillo - 23195

