#include "parqueo.h"
#include "persistencia.h"
#include "utils.h"
#include <iostream>
#include <ctime>
#include <cmath>
using namespace std;

void initConfig(Configuracion& cfg) {
    cfg.numCarriles = 2;
    cfg.espaciosPorCarril = 2;
    cfg.maxCola = 10;
    cfg.tasaPorHora = 10.0;
    cfg.dbPort = 3306;
    strCopiar(cfg.dbHost, "localhost", 64);
    strCopiar(cfg.dbUser, "root", 32);
    strCopiar(cfg.dbPass, "root", 32);
    strCopiar(cfg.dbName, "db_parqueo", 32);
}

void initParqueo(Parqueo& p, const Configuracion& cfg) {
    p.config = cfg;
    p.totalHistorial = 0;
    for (int i = 0; i < MAX_CARRILES; i++) {
        p.carriles[i].tope = NULL;
        p.carriles[i].cantidad = 0;
    }
    p.colaEspera.inicio = NULL;
    p.colaEspera.fin = NULL;
    p.colaEspera.cantidad = 0;
}

void pilaPush(Pila& pila, Vehiculo v) {
    NodoPila* nodo = new NodoPila();
    nodo->dato = v;
    nodo->siguiente = pila.tope;
    pila.tope = nodo;
    pila.cantidad++;
}

Vehiculo pilaPop(Pila& pila) {
    NodoPila* tmp = pila.tope;
    Vehiculo  v = tmp->dato;
    pila.tope = tmp->siguiente;
    delete tmp;
    pila.cantidad--;
    return v;
}

Vehiculo pilaPeek(const Pila& pila) { return pila.tope->dato; }
bool     pilaVacia(const Pila& pila) { return pila.tope == NULL; }

bool pilaBuscar(const Pila& pila, const char* placa, Vehiculo& resultado) {
    NodoPila* actual = pila.tope;
    while (actual != NULL) {
        if (strIgual(actual->dato.placa, placa)) {
            resultado = actual->dato;
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

void pilaLimpiar(Pila& pila) {
    while (!pilaVacia(pila)) pilaPop(pila);
}

void colaAdd(Cola& cola, Vehiculo v) {
    NodoCola* nodo = new NodoCola();
    nodo->dato = v;
    nodo->siguiente = NULL;
    if (cola.inicio == NULL) cola.inicio = nodo;
    else cola.fin->siguiente = nodo;
    cola.fin = nodo;
    cola.cantidad++;
}

Vehiculo colaRemove(Cola& cola) {
    NodoCola* tmp = cola.inicio;
    Vehiculo  v = tmp->dato;
    if (cola.inicio == cola.fin) { cola.inicio = NULL; cola.fin = NULL; }
    else cola.inicio = cola.inicio->siguiente;
    delete tmp;
    cola.cantidad--;
    return v;
}

Vehiculo colaPeek(const Cola& cola) { return cola.inicio->dato; }
bool     colaVacia(const Cola& cola) { return cola.inicio == NULL; }

int espaciosLibres(const Parqueo& p) {
    int ocupados = 0;
    for (int i = 0; i < p.config.numCarriles; i++)
        ocupados += p.carriles[i].cantidad;
    return (p.config.numCarriles * p.config.espaciosPorCarril) - ocupados;
}

bool estaLleno(const Parqueo& p) { return espaciosLibres(p) == 0; }

int seleccionarCarril(const Parqueo& p) {
    int mejor = -1, menorOcc = p.config.espaciosPorCarril + 1;
    for (int i = 0; i < p.config.numCarriles; i++) {
        int occ = p.carriles[i].cantidad;
        if (occ < p.config.espaciosPorCarril && occ < menorOcc) {
            menorOcc = occ; mejor = i;
        }
    }
    return mejor;
}

void persistirEstado(Parqueo& p) {
    Vehiculo todos[MAX_HISTORIAL];
    int n = 0;
    for (int i = 0; i < p.config.numCarriles && n < MAX_HISTORIAL; i++) {
        NodoPila* nodo = p.carriles[i].tope;
        while (nodo != NULL && n < MAX_HISTORIAL) {
            todos[n++] = nodo->dato;
            nodo = nodo->siguiente;
        }
    }
    for (int i = 0; i < p.totalHistorial && n < MAX_HISTORIAL; i++)
        todos[n++] = p.historial[i];
    guardarVehiculos(todos, n);
}

void estacionar(Parqueo& p, Vehiculo v) {
    int idx = seleccionarCarril(p);
    if (idx < 0) return;
    pilaPush(p.carriles[idx], v);
    logMovimiento(MOV_ENTRADA, v.placa, idx, p.carriles[idx].cantidad - 1);
    persistirEstado(p);
    cout << "  [OK] " << v.placa << " estacionado en carril "
        << idx + 1 << ", posicion " << p.carriles[idx].cantidad << ".\n";
}

void admitirDeCola(Parqueo& p) {
    if (colaVacia(p.colaEspera) || estaLleno(p)) return;
    Vehiculo siguiente = colaRemove(p.colaEspera);
    siguiente.horaEntrada = time(NULL);
    estacionar(p, siguiente);
    cout << "  [COLA] " << siguiente.placa << " admitido desde cola de espera.\n";
}

int ingresarVehiculo(Parqueo& p, Vehiculo v) {
    v.horaEntrada = time(NULL);
    v.activo = true;
    v.horaSalida = 0;
    if (!estaLleno(p)) { estacionar(p, v); return 0; }
    if (p.colaEspera.cantidad >= p.config.maxCola) return -1;
    colaAdd(p.colaEspera, v);
    cout << "  [COLA] " << v.placa << " en posicion "
        << p.colaEspera.cantidad << " de espera.\n";
    return 1;
}

Vehiculo extraerDeCarril(Parqueo& p, int carrilIdx, const char* placa) {
    Pila& carril = p.carriles[carrilIdx];
    Vehiculo buffer[MAX_ESPACIOS];
    int      bufCount = 0;
    Vehiculo objetivo;
    objetivo.placa[0] = '\0';

    while (!pilaVacia(carril)) {
        Vehiculo tope = pilaPop(carril);
        logMovimiento(MOV_TEMPORAL, tope.placa, carrilIdx, bufCount);
        if (strIgual(tope.placa, placa)) { objetivo = tope; break; }
        buffer[bufCount++] = tope;
    }
    for (int i = bufCount - 1; i >= 0; i--) {
        pilaPush(carril, buffer[i]);
        logMovimiento(MOV_REGRESO, buffer[i].placa, carrilIdx, carril.cantidad - 1);
    }
    return objetivo;
}

double salirVehiculo(Parqueo& p, const char* placa, double& tiempoHoras) {
    for (int i = 0; i < p.config.numCarriles; i++) {
        Vehiculo dummy;
        if (!pilaBuscar(p.carriles[i], placa, dummy)) continue;
        Vehiculo v = extraerDeCarril(p, i, placa);
        v.horaSalida = time(NULL);
        v.activo = false;
        double secs = difftime(v.horaSalida, v.horaEntrada);
        tiempoHoras = secs / 3600.0;
        double monto = ceil(tiempoHoras) * p.config.tasaPorHora;
        if (p.totalHistorial < MAX_HISTORIAL)
            p.historial[p.totalHistorial++] = v;
        logMovimiento(MOV_SALIDA, placa, i, 0);
        persistirEstado(p);
        admitirDeCola(p);
        return monto;
    }
    return -1.0;
}

bool buscarVehiculo(const Parqueo& p, const char* placa, Vehiculo& resultado) {
    for (int i = 0; i < p.config.numCarriles; i++)
        if (pilaBuscar(p.carriles[i], placa, resultado)) return true;
    return false;
}

EstadisticasDia calcularEstadisticas(const Parqueo& p) {
    EstadisticasDia stats;
    stats.fecha = time(NULL);
    stats.montoTotal = 0.0;
    double totalMinutos = 0.0;
    time_t ahora = time(NULL);

    int totalSalidos = p.totalHistorial;
    for (int i = 0; i < p.totalHistorial; i++) {
        const Vehiculo& v = p.historial[i];
        if (v.horaSalida > 0) {
            double secs = difftime(v.horaSalida, v.horaEntrada);
            stats.montoTotal += ceil(secs / 3600.0) * p.config.tasaPorHora;
            totalMinutos += secs / 60.0;
        }
    }

    int totalActivos = 0;
    for (int i = 0; i < p.config.numCarriles; i++) {
        NodoPila* nodo = p.carriles[i].tope;
        while (nodo != NULL) {
            double secs = difftime(ahora, nodo->dato.horaEntrada);
            stats.montoTotal += ceil(secs / 3600.0) * p.config.tasaPorHora;
            totalMinutos += secs / 60.0;
            totalActivos++;
            nodo = nodo->siguiente;
        }
    }

    NodoCola* nodoCola = p.colaEspera.inicio;
    while (nodoCola != NULL) { totalActivos++; nodoCola = nodoCola->siguiente; }

    stats.totalVehiculos = totalSalidos + totalActivos;

    if (stats.totalVehiculos > 0)
        stats.promedioTiempoMinutos = totalMinutos / stats.totalVehiculos;

    return stats;
}

void restaurarEstado(Parqueo& p) {
    Vehiculo lista[MAX_HISTORIAL];
    int n = cargarVehiculos(lista);
    for (int i = 0; i < n; i++) {
        if (lista[i].activo) {
            int idx = seleccionarCarril(p);
            if (idx >= 0 && p.carriles[idx].cantidad < p.config.espaciosPorCarril)
                pilaPush(p.carriles[idx], lista[i]);
        }
        else {
            if (p.totalHistorial < MAX_HISTORIAL)
                p.historial[p.totalHistorial++] = lista[i];
        }
    }
}

bool reconfigurar(Parqueo& p, int nc, int ne, int nq, double nt) {
    if (nc < p.config.numCarriles) {
        for (int i = nc; i < p.config.numCarriles; i++) {
            if (!pilaVacia(p.carriles[i])) {
                cout << "  [ERROR] Carril " << i + 1 << " tiene vehiculos.\n";
                return false;
            }
        }
    }
    if (ne < p.config.espaciosPorCarril) {
        for (int i = 0; i < p.config.numCarriles; i++) {
            if (p.carriles[i].cantidad > ne) {
                cout << "  [ERROR] Un carril excede la nueva capacidad.\n";
                return false;
            }
        }
    }
    p.config.numCarriles = nc;
    p.config.espaciosPorCarril = ne;
    p.config.maxCola = nq;
    p.config.tasaPorHora = nt;
    guardarConfig(p.config);
    return true;
}