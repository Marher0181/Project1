#include "sincronizacion.h"
#include "parqueo.h"
#include "utils.h"
#include <mysql.h>
#include <iostream>
#include <ctime>
#include <cmath>
using namespace std;

static void ejecutarSQL(MYSQL* con, const char* sql) {
    if (mysql_query(con, sql))
        cout << "  [DB] Error: " << mysql_error(con) << "\n";
}

bool sincronizar(const Parqueo& p) {
    const Configuracion& cfg = p.config;

    MYSQL* con = mysql_init(NULL);
    if (!con) { cout << "  [DB] mysql_init fallo.\n"; return false; }

    
    con = mysql_real_connect(con, cfg.dbHost, cfg.dbUser, cfg.dbPass,
        NULL, cfg.dbPort, NULL, 0);
    if (!con) {
        cout << "  [DB] Conexion fallida: " << mysql_error(con) << "\n";
        cout << "  Verifique credenciales en Configuracion -> Cambiar BD.\n";
        return false;
    }

    cout << "  [DB] Conexion exitosa.\n";

    char sqlDB[128]; sqlDB[0] = '\0';
    strAppend(sqlDB, "CREATE DATABASE IF NOT EXISTS `");
    strAppend(sqlDB, cfg.dbName);
    strAppend(sqlDB, "` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci");
    ejecutarSQL(con, sqlDB);

    if (mysql_select_db(con, cfg.dbName)) {
        cout << "  [DB] Error al seleccionar BD: " << mysql_error(con) << "\n";
        mysql_close(con); return false;
    }
    cout << "  [DB] Base de datos '" << cfg.dbName << "' lista.\n";

    ejecutarSQL(con,
        "CREATE TABLE IF NOT EXISTS estadisticas_dia ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "fecha DATE NOT NULL UNIQUE,"
        "total_vehiculos INT NOT NULL,"
        "monto_total DECIMAL(10,2) NOT NULL,"
        "promedio_mins DECIMAL(10,2) NOT NULL)");

    ejecutarSQL(con,
        "CREATE TABLE IF NOT EXISTS placas_registradas ("
        "placa VARCHAR(10) PRIMARY KEY,"
        "primera_vez DATE NOT NULL)");

    ejecutarSQL(con,
        "CREATE TABLE IF NOT EXISTS detalle_vehiculos ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "placa VARCHAR(10) NOT NULL,"
        "marca VARCHAR(30),"
        "modelo VARCHAR(30),"
        "hora_entrada DATETIME NOT NULL,"
        "hora_salida DATETIME,"
        "monto DECIMAL(10,2),"
        "fecha_sync DATE NOT NULL)");

    EstadisticasDia stats = calcularEstadisticas(p);
    char fecha[12]; fechaStr(stats.fecha, fecha);
    char tA[20], tB[20], tC[20];
    intStr(stats.totalVehiculos, tA);
    dblStr(stats.montoTotal, tB);
    dblStr(stats.promedioTiempoMinutos, tC);

    char sql[512]; sql[0] = '\0';
    strAppend(sql, "INSERT INTO estadisticas_dia (fecha,total_vehiculos,monto_total,promedio_mins) VALUES ('");
    strAppend(sql, fecha); strAppend(sql, "',");
    strAppend(sql, tA);    strAppend(sql, ",");
    strAppend(sql, tB);    strAppend(sql, ",");
    strAppend(sql, tC);
    strAppend(sql, ") ON DUPLICATE KEY UPDATE "
        "total_vehiculos=VALUES(total_vehiculos),"
        "monto_total=VALUES(monto_total),"
        "promedio_mins=VALUES(promedio_mins)");
    ejecutarSQL(con, sql);
    cout << "  [DB] Estadisticas insertadas.\n";

    int nuevas = 0;
    for (int i = 0; i < p.totalHistorial; i++) {
        sql[0] = '\0';
        strAppend(sql, "INSERT IGNORE INTO placas_registradas (placa,primera_vez) VALUES ('");
        strAppend(sql, p.historial[i].placa);
        strAppend(sql, "','"); strAppend(sql, fecha); strAppend(sql, "')");
        mysql_query(con, sql);
        if (mysql_affected_rows(con) > 0) nuevas++;
    }

    for (int i = 0; i < p.config.numCarriles; i++) {
        NodoPila* nodo = p.carriles[i].tope;
        while (nodo != NULL) {
            sql[0] = '\0';
            strAppend(sql, "INSERT IGNORE INTO placas_registradas (placa,primera_vez) VALUES ('");
            strAppend(sql, nodo->dato.placa);
            strAppend(sql, "','"); strAppend(sql, fecha); strAppend(sql, "')");
            mysql_query(con, sql);
            if (mysql_affected_rows(con) > 0) nuevas++;
            nodo = nodo->siguiente;
        }
    }
    cout << "  [DB] " << nuevas << " placas nuevas registradas.\n";

    int totalRegistros = 0;
    for (int i = 0; i < p.totalHistorial; i++) {
        const Vehiculo& v = p.historial[i];
        double monto = 0.0;
        if (v.horaSalida > 0) {
            double horas = difftime(v.horaSalida, v.horaEntrada) / 3600.0;
            monto = ceil(horas) * p.config.tasaPorHora;
        }
        char dtE[20], dtS[20], montoS[20];
        dtStr(v.horaEntrada, dtE);
        dblStr(monto, montoS);
        sql[0] = '\0';
        strAppend(sql, "INSERT INTO detalle_vehiculos "
            "(placa,marca,modelo,hora_entrada,hora_salida,monto,fecha_sync) VALUES ('");
        strAppend(sql, v.placa);  strAppend(sql, "','");
        strAppend(sql, v.marca);  strAppend(sql, "','");
        strAppend(sql, v.modelo); strAppend(sql, "','");
        strAppend(sql, dtE);      strAppend(sql, "',");
        if (v.horaSalida > 0) {
            dtStr(v.horaSalida, dtS);
            strAppend(sql, "'"); strAppend(sql, dtS); strAppend(sql, "',");
        }
        else { strAppend(sql, "NULL,"); }
        strAppend(sql, montoS); strAppend(sql, ",'");
        strAppend(sql, fecha);  strAppend(sql, "')");
        ejecutarSQL(con, sql);
        totalRegistros++;
    }
    for (int i = 0; i < p.config.numCarriles; i++) {
        NodoPila* nodo = p.carriles[i].tope;
        while (nodo != NULL) {
            const Vehiculo& v = nodo->dato;
            double secs = difftime(time(NULL), v.horaEntrada);
            double monto = ceil(secs / 3600.0) * p.config.tasaPorHora;
            char dtE[20], montoS[20];
            dtStr(v.horaEntrada, dtE);
            dblStr(monto, montoS);
            sql[0] = '\0';
            strAppend(sql, "INSERT INTO detalle_vehiculos "
                "(placa,marca,modelo,hora_entrada,hora_salida,monto,fecha_sync) VALUES ('");
            strAppend(sql, v.placa);  strAppend(sql, "','");
            strAppend(sql, v.marca);  strAppend(sql, "','");
            strAppend(sql, v.modelo); strAppend(sql, "','");
            strAppend(sql, dtE);      strAppend(sql, "',NULL,");
            strAppend(sql, montoS);   strAppend(sql, ",'");
            strAppend(sql, fecha);    strAppend(sql, "')");
            ejecutarSQL(con, sql);
            totalRegistros++;
            nodo = nodo->siguiente;
        }
    }
    cout << "  [DB] " << totalRegistros << " registros insertados.\n";

    mysql_close(con);
    cout << "  [DB] Sincronizacion completada.\n";
    return true;
}