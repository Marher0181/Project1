#ifndef REPORTES_H
#define REPORTES_H

#include "entities.h"

void generarCSV(const Parqueo& p);
void generarHTML(const Parqueo& p);
void generarPDF(const Parqueo& p);
void generarTodos(const Parqueo& p);

#endif
