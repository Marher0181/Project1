#ifndef ENTITIES_H
#define ENTITIES_H

#include <ctime>

#define MAX_CARRILES     10
#define MAX_ESPACIOS     10
#define MAX_COLA         50
#define MAX_HISTORIAL    100
#define DATA_FILE        "parqueo.dat"
#define CONFIG_FILE      "config.dat"
#define LOG_FILE         "movimientos.dat"

enum TipoMovimiento {
    MOV_ENTRADA  = 1,
    MOV_SALIDA   = 2,
    MOV_TEMPORAL = 3,
    MOV_REGRESO  = 4
};

struct Vehiculo {
    char   placa[10];
    char   marca[30];
    char   modelo[30];
    time_t horaEntrada;
    time_t horaSalida;
    bool   activo;
};

struct NodoPila {
    Vehiculo  dato;
    NodoPila* siguiente;
};

struct NodoCola {
    Vehiculo  dato;
    NodoCola* siguiente;
};

struct RegistroMovimiento {
    TipoMovimiento tipo;
    char           placa[10];
    int            carril;
    int            posicion;
    time_t         timestamp;
};

struct Configuracion {
    int    numCarriles;
    int    espaciosPorCarril;
    int    maxCola;
    double tasaPorHora;
    char   dbHost[64];
    char   dbUser[32];
    char   dbPass[32];
    char   dbName[32];
    int    dbPort;
};

struct EstadisticasDia {
    int    totalVehiculos;
    double montoTotal;
    double promedioTiempoMinutos;
    time_t fecha;
};

struct Pila {
    NodoPila* tope;
    int       cantidad;
};

struct Cola {
    NodoCola* inicio;
    NodoCola* fin;
    int       cantidad;
};

struct Parqueo {
    Pila          carriles[MAX_CARRILES];
    Cola          colaEspera;
    Vehiculo      historial[MAX_HISTORIAL];
    int           totalHistorial;
    Configuracion config;
};

#endif
