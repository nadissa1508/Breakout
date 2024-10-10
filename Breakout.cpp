/*---------------------------------------
UNIVERSIDAD DEL VALLE DE GUATEMALA
CC3056 - Programacion de Microprocesadores
Colaboradores:





Fecha: 10/../2019

Breakout.cpp
......
---------------------------------------*/

#include <stdio.h>
#include <pthread.h>
#include<stdlib.h>
#include <iostream>
#include <unistd.h>

int ancho_pantalla = 80;
int alto_pantalla =24;

int pala1_x = 30, pala1_y = 20;
int pala2_x = 30, pala2_y = 20;

int ancho_pala =10;

int pelota_x = 40, pelota_y =12;
int pelota_dir_x = 1, pelota_dir_2 =1;

int score = 0;

bool game_over = false;

pthread_mutex_t ball_mutex;
pthread_mutex_t paddle_mutex;

void* dibujar_palas(){
    
}

void* dibujar_ladrillos(){

}

void* logica_pelota(void* arg){

}

void* logica_pala1(void* arg){

}

void* logica_pala2(void* arg){

}

void menu(){

}


int main(void){

}