#ifndef PARQUEO_H
#define PARQUEO_H

#include "entities.h"

void initConfig(Configuracion& cfg);
void initParqueo(Parqueo& p, const Configuracion& cfg);

// Pila operations
void     pilaPush(Pila& pila, Vehiculo v);
Vehiculo pilaPop(Pila& pila);
Vehiculo pilaPeek(const Pila& pila);
bool     pilaVacia(const Pila& pila);
bool     pilaBuscar(const Pila& pila, const char* placa, Vehiculo& resultado);
void     pilaLimpiar(Pila& pila);

// Cola operations
void     colaAdd(Cola& cola, Vehiculo v);
Vehiculo colaRemove(Cola& cola);
Vehiculo colaPeek(const Cola& cola);
bool     colaVacia(const Cola& cola);

// Parking operations
int      espaciosLibres(const Parqueo& p);
bool     estaLleno(const Parqueo& p);
int      seleccionarCarril(const Parqueo& p);
void     persistirEstado(Parqueo& p);
void     estacionar(Parqueo& p, Vehiculo v);
void     admitirDeCola(Parqueo& p);
int      ingresarVehiculo(Parqueo& p, Vehiculo v);
Vehiculo extraerDeCarril(Parqueo& p, int carrilIdx, const char* placa);
double   salirVehiculo(Parqueo& p, const char* placa, double& tiempoHoras);
bool     buscarVehiculo(const Parqueo& p, const char* placa, Vehiculo& resultado);
EstadisticasDia calcularEstadisticas(const Parqueo& p);
void     restaurarEstado(Parqueo& p);
bool     reconfigurar(Parqueo& p, int nc, int ne, int nq, double nt);

#endif
