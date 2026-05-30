#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "entities.h"

bool guardarConfig(const Configuracion& cfg);
bool cargarConfig(Configuracion& cfg);
bool guardarVehiculos(Vehiculo lista[], int n);
int  cargarVehiculos(Vehiculo lista[]);
void logMovimiento(TipoMovimiento tipo, const char* placa, int carril, int pos);

#endif
