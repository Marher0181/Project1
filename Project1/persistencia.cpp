#include "persistencia.h"
#include "utils.h"
#include <fstream>
using namespace std;

bool guardarConfig(const Configuracion& cfg) {
    ofstream f(CONFIG_FILE, ios::binary | ios::trunc);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(&cfg), sizeof(Configuracion));
    return f.good();
}

bool cargarConfig(Configuracion& cfg) {
    ifstream f(CONFIG_FILE, ios::binary);
    if (!f) return false;
    f.read(reinterpret_cast<char*>(&cfg), sizeof(Configuracion));
    return f.good();
}

bool guardarVehiculos(Vehiculo lista[], int n) {
    ofstream f(DATA_FILE, ios::binary | ios::trunc);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(&n), sizeof(int));
    for (int i = 0; i < n; i++)
        f.write(reinterpret_cast<const char*>(&lista[i]), sizeof(Vehiculo));
    return f.good();
}

int cargarVehiculos(Vehiculo lista[]) {
    ifstream f(DATA_FILE, ios::binary);
    if (!f) return 0;
    int n = 0;
    f.read(reinterpret_cast<char*>(&n), sizeof(int));
    if (n > MAX_HISTORIAL) n = MAX_HISTORIAL;
    for (int i = 0; i < n; i++)
        f.read(reinterpret_cast<char*>(&lista[i]), sizeof(Vehiculo));
    return n;
}

void logMovimiento(TipoMovimiento tipo, const char* placa, int carril, int pos) {
    ofstream f(LOG_FILE, ios::binary | ios::app);
    if (!f) return;
    RegistroMovimiento reg;
    reg.tipo      = tipo;
    reg.carril    = carril;
    reg.posicion  = pos;
    reg.timestamp = time(NULL);
    strCopiar(reg.placa, placa, 10);
    f.write(reinterpret_cast<const char*>(&reg), sizeof(RegistroMovimiento));
}
