#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <vector>
#include <string>

using namespace std;

// Struct para instancias de Beasley (Unipista)
struct Avion {
    int id;
    double tiempo_llegada;
    double tiempo_minimo;
    double tiempo_objetivo;
    double tiempo_maximo;
    double costo_segundo_temprano;
    double costo_segundo_tarde;
};

// Struct para instancias FTP (Multipista)
struct AvionReal {
    int id;          
    string codigo_vuelo;     
    string modelo;           
    char categoria_estela;   
    char operacion;          
    double tiempo_objetivo;  
    double penalidad;        
};

// Declaramos los dos espacios de prueba: Unipista y Multipista
namespace Uni {
    void ejecutarPruebas();
}

namespace Multi {
    void ejecutarPruebas(int pistas_ingresadas);
}

#endif