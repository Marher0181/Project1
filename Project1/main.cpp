#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <cmath>

#include "entities.h"
#include "utils.h"
#include "persistencia.h"
#include "parqueo.h"
#include "sincronizacion.h"
#include "reportes.h"

using namespace std;

void limpiar() { system("cls"); }

void linea(char c = '-', int n = 55) {
    for (int i = 0; i < n; i++) cout << c;
    cout << "\n";
}

void cabecera(const char* titulo) {
    limpiar(); linea('=');
    cout << "  SISTEMA DE PARQUEO  |  " << titulo << "\n";
    linea('=');
}

void pausar() {
    cout << "\n  Presione ENTER para continuar...";
    cin.ignore(); cin.get();
}

void leerLinea(const char* prompt, char* buf, int n) {
    cout << "  " << prompt;
    cin.getline(buf, n);
}

int leerInt(const char* prompt) {
    int v; cout << "  " << prompt;
    while (!(cin >> v)) { cin.clear(); cin.ignore(1000, '\n'); cout << "  Invalido. " << prompt; }
    return v;
}

double leerDouble(const char* prompt) {
    double v; cout << "  " << prompt;
    while (!(cin >> v)) { cin.clear(); cin.ignore(1000, '\n'); cout << "  Invalido. " << prompt; }
    return v;
}

void menuIngreso(Parqueo& p);
void menuSalida(Parqueo& p);
void menuBuscar(Parqueo& p);
void menuEstado(Parqueo& p);
void menuConfiguracion(Parqueo& p);
void menuSincronizacion(Parqueo& p);
void menuReportes(Parqueo& p);

void wizardPrimerInicio(Configuracion& cfg) {
    limpiar(); linea('=');
    cout << "  SISTEMA DE PARQUEO  |  Configuracion Inicial\n"; linea('=');
    cout << "\n  Bienvenido. Configure el sistema antes de comenzar.\n\n";

    linea('-');
    cout << "  CAPACIDAD DEL PARQUEO\n"; linea('-');
    cfg.numCarriles = leerInt("Numero de carriles:         ");
    if (cfg.numCarriles < 1 || cfg.numCarriles > MAX_CARRILES)
        cfg.numCarriles = 2;
    cfg.espaciosPorCarril = leerInt("Espacios por carril:        ");
    if (cfg.espaciosPorCarril < 1 || cfg.espaciosPorCarril > MAX_ESPACIOS)
        cfg.espaciosPorCarril = 2;
    cfg.maxCola = leerInt("Maximo vehiculos en cola:   ");
    if (cfg.maxCola < 1 || cfg.maxCola > MAX_COLA)
        cfg.maxCola = 10;

    cout << "\n"; linea('-');
    cout << "  TARIFA\n"; linea('-');
    cfg.tasaPorHora = leerDouble("Tasa por hora (Q):          ");

    cout << "\n"; linea('-');
    cout << "  CONEXION A BASE DE DATOS\n"; linea('-');
    cout << "  (Presione ENTER para usar el valor por defecto)\n\n";

    char tmp[64];
    cin.ignore();

    cout << "  Host     [localhost]: "; cin.getline(tmp, 64);
    if (tmp[0] != '\0') strCopiar(cfg.dbHost, tmp, 64);

    cout << "  Usuario  [root]:      "; cin.getline(tmp, 32);
    if (tmp[0] != '\0') strCopiar(cfg.dbUser, tmp, 32);

    cout << "  Password [root]:      "; cin.getline(tmp, 32);
    if (tmp[0] != '\0') strCopiar(cfg.dbPass, tmp, 32);

    cout << "  Base de datos [db_parqueo]: "; cin.getline(tmp, 32);
    if (tmp[0] != '\0') strCopiar(cfg.dbName, tmp, 32);

    cout << "  Puerto   [3306]:      "; cin.getline(tmp, 10);
    if (tmp[0] != '\0') {
        int port = 0;
        for (int i = 0; tmp[i] != '\0'; i++) port = port * 10 + (tmp[i] - '0');
        if (port > 0) cfg.dbPort = port;
    }

    cout << "\n"; linea('=');
    cout << "  RESUMEN DE CONFIGURACION\n"; linea('=');
    char tasaS[20]; dblStr(cfg.tasaPorHora, tasaS);
    cout << "  Carriles:        " << cfg.numCarriles << "\n"
        << "  Espacios/carril: " << cfg.espaciosPorCarril << "\n"
        << "  Max cola:        " << cfg.maxCola << "\n"
        << "  Tasa/hora:       Q" << tasaS << "\n"
        << "  DB Host:         " << cfg.dbHost << "\n"
        << "  DB Usuario:      " << cfg.dbUser << "\n"
        << "  DB Nombre:       " << cfg.dbName << "\n"
        << "  DB Puerto:       " << cfg.dbPort << "\n\n";

    cout << "  Confirmar configuracion? (s/n): ";
    char conf; cin >> conf;
    if (conf != 's' && conf != 'S') {
        initConfig(cfg);
        wizardPrimerInicio(cfg);
        return;
    }

    guardarConfig(cfg);
    cout << "\n  [OK] Configuracion guardada.\n";
    pausar();
}

int main() {
    Configuracion cfg;
    initConfig(cfg);

    if (!cargarConfig(cfg)) {
        wizardPrimerInicio(cfg);
    }
    else {
        limpiar(); linea('=');
        cout << "  SISTEMA DE PARQUEO  |  Bienvenido\n"; linea('=');
        char tasaS[20]; dblStr(cfg.tasaPorHora, tasaS);
        cout << "\n  Configuracion cargada.\n"
            << "  Carriles: " << cfg.numCarriles
            << " x " << cfg.espaciosPorCarril << " espacios"
            << "  |  Tasa: Q" << tasaS << "/hora\n";
        pausar();
    }

    Parqueo parqueo;
    initParqueo(parqueo, cfg);
    restaurarEstado(parqueo);

    char op;
    do {
        cabecera("MENU PRINCIPAL");
        cout << "\n"
            << "  [1] Ingresar vehiculo\n"
            << "  [2] Registrar salida\n"
            << "  [3] Buscar vehiculo\n"
            << "  [4] Ver estado del parqueo\n"
            << "  [5] Configuracion\n"
            << "  [6] Sincronizar con base de datos\n"
            << "  [7] Generar reportes\n"
            << "  [0] Cerrar programa\n";
        linea();
        cout << "  Opcion: "; cin >> op;

        switch (op) {
        case '1': menuIngreso(parqueo);        break;
        case '2': menuSalida(parqueo);         break;
        case '3': menuBuscar(parqueo);         break;
        case '4': menuEstado(parqueo);         break;
        case '5': menuConfiguracion(parqueo);  break;
        case '6': menuSincronizacion(parqueo); break;
        case '7': menuReportes(parqueo);       break;
        case '0': break;
        default: cout << "\n  Opcion no valida.\n"; pausar();
        }
    } while (op != '0');

    cabecera("CERRANDO");
    cout << "\n  Sistema cerrado correctamente.\n\n";
    return 0;
}

void menuIngreso(Parqueo& p) {
    cabecera("INGRESO DE VEHICULO");

    if (estaLleno(p) && p.colaEspera.cantidad >= p.config.maxCola) {
        cout << "\n  [!] Parqueo y cola llenos. No se puede ingresar.\n";
        pausar(); return;
    }

    Vehiculo v;
    v.horaEntrada = 0; v.horaSalida = 0; v.activo = false;
    cout << "\n"; cin.ignore();
    leerLinea("Placa:  ", v.placa, 10);

    Vehiculo dummy;
    if (buscarVehiculo(p, v.placa, dummy)) {
        cout << "\n  [!] " << v.placa << " ya esta dentro.\n";
        pausar(); return;
    }

    leerLinea("Marca:  ", v.marca, 30);
    leerLinea("Modelo: ", v.modelo, 30);
    cout << "\n";

    int res = ingresarVehiculo(p, v);
    if (res == 0) cout << "  Vehiculo estacionado.\n";
    else if (res == 1) cout << "  Parqueo lleno. En cola de espera.\n";
    else                cout << "  [ERROR] Cola llena.\n";
    pausar();
}

void menuSalida(Parqueo& p) {
    cabecera("SALIDA DE VEHICULO");
    cout << "\n"; cin.ignore();

    char placa[10];
    leerLinea("Placa a retirar: ", placa, 10);

    Vehiculo found;
    if (!buscarVehiculo(p, placa, found)) {
        cout << "\n  [!] Placa " << placa << " no encontrada.\n";
        pausar(); return;
    }

    double secs = difftime(time(NULL), found.horaEntrada);
    double horas = secs / 3600.0;
    double frac = ceil(horas);
    double previo = frac * p.config.tasaPorHora;

    char tiempoS[12], montoS[20], tasaS[20];
    formatTiempo(secs, tiempoS);
    dblStr(previo, montoS);
    dblStr(p.config.tasaPorHora, tasaS);

    cout << "\n  Vehiculo : " << found.placa << " | "
        << found.marca << " " << found.modelo << "\n"
        << "  Tiempo   : " << tiempoS << "\n"
        << "  Monto est: Q" << montoS
        << "  (" << (int)frac << " hora(s) x Q" << tasaS << ")\n\n"
        << "  Confirmar salida? (s/n): ";
    char conf; cin >> conf;
    if (conf != 's' && conf != 'S') { pausar(); return; }

    double tiempoHoras;
    double monto = salirVehiculo(p, placa, tiempoHoras);
    if (monto < 0) {
        cout << "\n  [ERROR] No se pudo procesar.\n";
    }
    else {
        char minsS[20], montoF[20];
        dblStr(tiempoHoras * 60.0, minsS);
        dblStr(monto, montoF);
        cout << "\n"; linea('=');
        cout << "           TICKET DE PARQUEO\n"; linea('=');
        cout << "  Placa  : " << placa << "\n"
            << "  Tiempo : " << minsS << " minutos\n"
            << "  Horas  : " << (int)ceil(tiempoHoras) << " (fraccion)\n"
            << "  Tasa   : Q" << tasaS << "/hora\n";
        linea('-');
        cout << "  TOTAL  : Q" << montoF << "\n"; linea('=');
    }
    pausar();
}

void menuBuscar(Parqueo& p) {
    cabecera("BUSCAR VEHICULO");
    cout << "\n"; cin.ignore();

    char placa[10];
    leerLinea("Placa: ", placa, 10);

    Vehiculo v;
    if (!buscarVehiculo(p, placa, v)) {
        cout << "\n  [!] No encontrado.\n"; pausar(); return;
    }

    double secs = difftime(time(NULL), v.horaEntrada);
    char tiempoS[12], montoS[20];
    formatTiempo(secs, tiempoS);
    dblStr(ceil(secs / 3600.0) * p.config.tasaPorHora, montoS);

    cout << "\n"; linea('-');
    cout << "  Placa   : " << v.placa << "\n"
        << "  Marca   : " << v.marca << "\n"
        << "  Modelo  : " << v.modelo << "\n"
        << "  Entrada : " << ctime(&v.horaEntrada)
        << "  Tiempo  : " << tiempoS << "\n"
        << "  Monto est: Q" << montoS << "\n";
    linea('-');
    pausar();
}

void menuEstado(Parqueo& p) {
    cabecera("ESTADO DEL PARQUEO");
    int total = p.config.numCarriles * p.config.espaciosPorCarril;
    int libres = espaciosLibres(p);

    cout << "\n  Capacidad: " << total
        << "  |  Ocupados: " << total - libres
        << "  |  Libres: " << libres << "\n"
        << "  Cola: " << p.colaEspera.cantidad
        << " / " << p.config.maxCola << "\n\n";

    for (int i = 0; i < p.config.numCarriles; i++) {
        cout << "  Carril " << i + 1 << " ["
            << p.carriles[i].cantidad << "/"
            << p.config.espaciosPorCarril << "]:  ";

        if (pilaVacia(p.carriles[i])) {
            cout << "(vacio)";
        }
        else {
            Vehiculo buf[MAX_ESPACIOS]; int n = 0;
            NodoPila* nodo = p.carriles[i].tope;
            while (nodo != NULL) { buf[n++] = nodo->dato; nodo = nodo->siguiente; }
            for (int j = n - 1; j >= 0; j--) {
                if (j < n - 1) cout << " -> ";
                cout << buf[j].placa;
            }
            cout << "  <-- TOPE";
        }
        cout << "\n";
    }

    if (!colaVacia(p.colaEspera)) {
        cout << "\n  Cola: ";
        NodoCola* nodo = p.colaEspera.inicio; int pos = 1;
        while (nodo != NULL) { cout << pos++ << "." << nodo->dato.placa << " "; nodo = nodo->siguiente; }
        cout << "\n";
    }
    pausar();
}

void resetFabrica(Parqueo& p) {
    cabecera("RESTABLECER DE FABRICA");
    cout << "\n  ADVERTENCIA: Esta accion eliminara TODO:\n"
        << "    - Configuracion\n"
        << "    - Vehiculos estacionados\n"
        << "    - Historial del dia\n"
        << "    - Archivos de datos\n\n"
        << "  Escriba CONFIRMAR para continuar: ";

    char confirm[20]; cin.ignore();
    cin.getline(confirm, 20);

    const char* palabra = "CONFIRMAR";
    bool igual = true; int i = 0;
    while (palabra[i] != '\0' || confirm[i] != '\0') {
        if (palabra[i] != confirm[i]) { igual = false; break; }
        i++;
    }
    if (!igual) { cout << "\n  Operacion cancelada.\n"; pausar(); return; }
    for (int j = 0; j < p.config.numCarriles; j++) pilaLimpiar(p.carriles[j]);
    while (!colaVacia(p.colaEspera)) colaRemove(p.colaEspera);
    p.totalHistorial = 0;
    remove(DATA_FILE); remove(CONFIG_FILE); remove(LOG_FILE);

    cout << "\n  [OK] Sistema restablecido. Se ejecutara el wizard inicial.\n";
    pausar();

    Configuracion nueva; initConfig(nueva);
    wizardPrimerInicio(nueva);
    initParqueo(p, nueva);
}

void menuConfiguracion(Parqueo& p) {
    char op;
    do {
        cabecera("CONFIGURACION");
        char tasaS[20]; dblStr(p.config.tasaPorHora, tasaS);
        cout << "\n  Carriles:        " << p.config.numCarriles << "\n"
            << "  Espacios/carril: " << p.config.espaciosPorCarril << "\n"
            << "  Max cola:        " << p.config.maxCola << "\n"
            << "  Tasa/hora:       Q" << tasaS << "\n"
            << "  DB Host:         " << p.config.dbHost << "\n"
            << "  DB Nombre:       " << p.config.dbName << "\n\n"
            << "  [1] Cambiar capacidad\n"
            << "  [2] Cambiar tasa\n"
            << "  [3] Cambiar BD\n"
            << "  [4] Restablecer de fabrica\n"
            << "  [0] Volver\n";
        linea(); cout << "  Opcion: "; cin >> op;

        switch (op) {
        case '1': {
            int nc = leerInt("Nuevo num. carriles:   ");
            int ne = leerInt("Nuevos espacios/carril:");
            int nq = leerInt("Nuevo max. cola:       ");
            if (reconfigurar(p, nc, ne, nq, p.config.tasaPorHora))
                cout << "\n  [OK] Capacidad actualizada.\n";
            pausar(); break;
        }
        case '2': {
            double nt = leerDouble("Nueva tasa (Q): ");
            p.config.tasaPorHora = nt;
            guardarConfig(p.config);
            char s[20]; dblStr(nt, s);
            cout << "\n  [OK] Tasa: Q" << s << "/hora.\n";
            pausar(); break;
        }
        case '3': {
            char tmp[64]; cin.ignore();
            leerLinea("Host DB:     ", tmp, 64); if (tmp[0]) strCopiar(p.config.dbHost, tmp, 64);
            leerLinea("Usuario DB:  ", tmp, 32); if (tmp[0]) strCopiar(p.config.dbUser, tmp, 32);
            leerLinea("Password DB: ", tmp, 32); if (tmp[0]) strCopiar(p.config.dbPass, tmp, 32);
            leerLinea("Nombre DB:   ", tmp, 32); if (tmp[0]) strCopiar(p.config.dbName, tmp, 32);
            int port = leerInt("Puerto DB:   ");
            if (port > 0) p.config.dbPort = port;
            guardarConfig(p.config);
            cout << "\n  [OK] Parametros actualizados.\n";
            pausar(); break;
        }
        case '4': {
            resetFabrica(p);
            op = '0'; break;
        }
        }
    } while (op != '0');
}

void menuSincronizacion(Parqueo& p) {
    cabecera("SINCRONIZACION");
    EstadisticasDia stats = calcularEstadisticas(p);
    char tA[20], tB[20], tC[20];
    intStr(stats.totalVehiculos, tA);
    dblStr(stats.montoTotal, tB);
    dblStr(stats.promedioTiempoMinutos, tC);

    cout << "\n  Vehiculos: " << tA
        << "\n  Monto:     Q" << tB
        << "\n  Promedio:  " << tC << " min"
        << "\n  Destino:   " << p.config.dbHost << "/" << p.config.dbName << "\n\n"
        << "  Confirmar? (s/n): ";
    char conf; cin >> conf;
    if (conf != 's' && conf != 'S') { pausar(); return; }
    cout << "\n";
    sincronizar(p);
    pausar();
}

void menuReportes(Parqueo& p) {
    cabecera("GENERAR REPORTES");
    char fecha[12]; fechaStr(time(NULL), fecha);
    char tA[20]; intStr(p.totalHistorial, tA);

    cout << "\n  Vehiculos en historial: " << tA << "\n"
        << "  Archivos a generar:\n"
        << "    reporte_" << fecha << ".csv\n"
        << "    reporte_" << fecha << ".html\n"
        << "    reporte_" << fecha << ".pdf\n\n"
        << "  Confirmar? (s/n): ";
    char conf; cin >> conf;
    if (conf != 's' && conf != 'S') { pausar(); return; }
    cout << "\n";
    generarTodos(p);
    pausar();
}