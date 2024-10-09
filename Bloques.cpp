class Bloque {

public:
    int x;
    int y;
    bool activo;

    Bloque(int posX, int posY): x(posX), y(posY), activo(true){}


    void* generarBloques(){

    }

    void destruirBloques(){
        activo = false;
    }


};