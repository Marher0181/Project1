#include "reportes.h"
#include "parqueo.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cmath>
using namespace std;

void generarCSV(const Parqueo& p) {
    EstadisticasDia stats = calcularEstadisticas(p);
    char fecha[12]; fechaStr(stats.fecha, fecha);
    char fname[32]; fname[0]='\0';
    strAppend(fname,"reporte_"); strAppend(fname,fecha); strAppend(fname,".csv");

    ofstream f(fname);
    if (!f) { cout << "  [RPT] No se pudo crear " << fname << "\n"; return; }

    char tA[20],tB[20],tC[20];
    intStr(stats.totalVehiculos,tA);
    dblStr(stats.montoTotal,tB);
    dblStr(stats.promedioTiempoMinutos,tC);

    f << "RESUMEN DEL DIA\n"
      << "Fecha," << fecha << "\n"
      << "Total Vehiculos," << tA << "\n"
      << "Monto Total,Q" << tB << "\n"
      << "Promedio (min)," << tC << "\n\n"
      << "DETALLE DE VEHICULOS\n"
      << "Placa,Marca,Modelo,Entrada,Salida,Tiempo (min),Monto\n";

    for (int i = 0; i < p.totalHistorial; i++) {
        const Vehiculo& v = p.historial[i];
        double mins=0.0, monto=0.0;
        if (v.horaSalida > 0) {
            double secs = difftime(v.horaSalida, v.horaEntrada);
            mins  = secs / 60.0;
            monto = ceil(secs / 3600.0) * p.config.tasaPorHora;
        }
        char dtE[20], dtS[20], minsS[20], montoS[20];
        dtStr(v.horaEntrada, dtE);
        dblStr(mins, minsS); dblStr(monto, montoS);

        f << v.placa << "," << v.marca << "," << v.modelo << "," << dtE << ",";
        if (v.horaSalida > 0) { dtStr(v.horaSalida, dtS); f << dtS; }
        else f << "En parqueo";
        f << "," << minsS << ",Q" << montoS << "\n";
    }
    cout << "  [RPT] CSV generado: " << fname << "\n";
}

void generarHTML(const Parqueo& p) {
    EstadisticasDia stats = calcularEstadisticas(p);
    char fecha[12]; fechaStr(stats.fecha, fecha);
    char fname[32]; fname[0]='\0';
    strAppend(fname,"reporte_"); strAppend(fname,fecha); strAppend(fname,".html");

    ofstream f(fname);
    if (!f) { cout << "  [RPT] No se pudo crear " << fname << "\n"; return; }

    char tA[20],tB[20],tC[20];
    intStr(stats.totalVehiculos,tA);
    dblStr(stats.montoTotal,tB);
    dblStr(stats.promedioTiempoMinutos,tC);

    f << "<!DOCTYPE html><html lang='es'><head><meta charset='UTF-8'>"
      << "<title>Reporte " << fecha << "</title><style>"
      << "body{font-family:Arial,sans-serif;margin:40px;color:#333}"
      << "h1{color:#1a237e}h2{color:#283593;border-bottom:2px solid #3949ab;padding-bottom:4px}"
      << ".cards{display:flex;gap:20px;margin-bottom:30px}"
      << ".card{background:#e8eaf6;border-radius:8px;padding:16px 24px;min-width:150px}"
      << ".lbl{font-size:12px;color:#5c6bc0;text-transform:uppercase}"
      << ".val{font-size:24px;font-weight:bold;color:#1a237e}"
      << "table{border-collapse:collapse;width:100%}"
      << "th{background:#3949ab;color:#fff;padding:10px;text-align:left}"
      << "td{padding:8px 10px;border-bottom:1px solid #c5cae9}"
      << "tr:nth-child(even){background:#f3f4fc}"
      << "</style></head><body>"
      << "<h1>Reporte de Operaciones</h1>"
      << "<p>Fecha: <strong>" << fecha << "</strong></p>"
      << "<h2>Resumen</h2><div class='cards'>"
      << "<div class='card'><div class='lbl'>Vehiculos</div>"
      << "<div class='val'>" << tA << "</div></div>"
      << "<div class='card'><div class='lbl'>Recaudado</div>"
      << "<div class='val'>Q" << tB << "</div></div>"
      << "<div class='card'><div class='lbl'>Prom. tiempo</div>"
      << "<div class='val'>" << tC << " min</div></div>"
      << "</div><h2>Detalle</h2>"
      << "<table><thead><tr>"
      << "<th>Placa</th><th>Marca</th><th>Modelo</th>"
      << "<th>Entrada</th><th>Salida</th><th>Tiempo</th><th>Monto</th>"
      << "</tr></thead><tbody>\n";

    for (int i = 0; i < p.totalHistorial; i++) {
        const Vehiculo& v = p.historial[i];
        double mins=0.0, monto=0.0;
        if (v.horaSalida > 0) {
            double secs = difftime(v.horaSalida, v.horaEntrada);
            mins  = secs / 60.0;
            monto = ceil(secs / 3600.0) * p.config.tasaPorHora;
        }
        char dtE[20], dtS[20], minsS[20], montoS[20];
        dtStr(v.horaEntrada, dtE);
        dblStr(mins, minsS); dblStr(monto, montoS);

        f << "<tr><td>" << v.placa << "</td><td>" << v.marca
          << "</td><td>" << v.modelo << "</td><td>" << dtE << "</td><td>";
        if (v.horaSalida > 0) { dtStr(v.horaSalida, dtS); f << dtS; }
        else f << "En parqueo";
        f << "</td><td>" << minsS << " min</td><td>Q" << montoS << "</td></tr>\n";
    }
    f << "</tbody></table></body></html>";
    cout << "  [RPT] HTML generado: " << fname << "\n";
}


static void pdfEscape(const char* src, char* dst) {
    int j = 0;
    for (int i = 0; src[i] != '\0'; i++) {
        if (src[i]=='(' || src[i]==')' || src[i]=='\\') dst[j++]='\\';
        dst[j++] = src[i];
    }
    dst[j] = '\0';
}

static void pdfBT(ofstream& f, float x, float y, float sz, const char* txt) {
    char ex[200]; pdfEscape(txt, ex);
    char xs[15],ys[15],szs[10];
    dblStr(x,xs); dblStr(y,ys); dblStr(sz,szs);
    f << "BT /F1 " << szs << " Tf " << xs << " " << ys << " Td (" << ex << ") Tj ET\n";
}

static void pdfBTB(ofstream& f, float x, float y, float sz, const char* txt) {
    char ex[200]; pdfEscape(txt, ex);
    char xs[15],ys[15],szs[10];
    dblStr(x,xs); dblStr(y,ys); dblStr(sz,szs);
    f << "BT /F2 " << szs << " Tf " << xs << " " << ys << " Td (" << ex << ") Tj ET\n";
}

static void pdfRect(ofstream& f, float x, float y, float w, float h,
                    float r, float g, float b) {
    char xs[10],ys[10],ws[10],hs[10],rs[8],gs[8],bs[8];
    dblStr(x,xs);dblStr(y,ys);dblStr(w,ws);dblStr(h,hs);
    dblStr(r,rs);dblStr(g,gs);dblStr(b,bs);
    f << rs<<" "<<gs<<" "<<bs<<" rg "
      << xs<<" "<<ys<<" "<<ws<<" "<<hs<<" re f\n"
      << "0 0 0 rg\n";
}

#define PAGE_W  595
#define PAGE_H  842
#define MARGIN   40
#define ROW_H    14
#define HDR_H    18

static float COL_X[] = { 40, 95, 155, 220, 310, 400, 460, 520 };
static const char* COL_HDR[] = {"Placa","Marca","Modelo","Entrada","Salida","Min","Monto"};

void generarPDF(const Parqueo& p) {
    EstadisticasDia stats = calcularEstadisticas(p);
    char fecha[12]; fechaStr(stats.fecha, fecha);
    char fname[32]; fname[0]='\0';
    strAppend(fname,"reporte_"); strAppend(fname,fecha); strAppend(fname,".pdf");

    int totalRows    = p.totalHistorial;
    int rowsPage1    = (PAGE_H - 280) / ROW_H;
    int rowsPerPage  = (PAGE_H - 120) / ROW_H;
    if (rowsPage1   < 1) rowsPage1   = 1;
    if (rowsPerPage < 1) rowsPerPage = 1;

    int numPages = 1;
    if (totalRows > rowsPage1)
        numPages += (totalRows - rowsPage1 + rowsPerPage - 1) / rowsPerPage;
    if (numPages > 20) numPages = 20;

    long streamLens[20];
    char tmpNames[20][24];

    int rowIdx = 0;
    for (int pg = 0; pg < numPages; pg++) {
        tmpNames[pg][0] = '\0';
        strAppend(tmpNames[pg], "pg_");
        char pgS[5]; intStr(pg, pgS);
        strAppend(tmpNames[pg], pgS);
        strAppend(tmpNames[pg], ".tmp");

        ofstream tmp(tmpNames[pg]);
        float y = PAGE_H - 50;

        if (pg == 0) {
            char titulo[60]; titulo[0]='\0';
            strAppend(titulo,"Reporte de Operaciones - "); strAppend(titulo,fecha);
            pdfBTB(tmp, MARGIN, y, 16, titulo); y -= 24;

            char tA[20],tB[20],tC[20];
            intStr(stats.totalVehiculos,tA);
            dblStr(stats.montoTotal,tB);
            dblStr(stats.promedioTiempoMinutos,tC);

            pdfRect(tmp, MARGIN-2, y-4, PAGE_W-MARGIN*2, 56, 0.91f,0.92f,0.97f);
            char ln[50];
            ln[0]='\0'; strAppend(ln,"Vehiculos atendidos: "); strAppend(ln,tA);
            pdfBTB(tmp, MARGIN, y, 10, ln); y -= 16;
            ln[0]='\0'; strAppend(ln,"Total recaudado: Q"); strAppend(ln,tB);
            pdfBTB(tmp, MARGIN, y, 10, ln); y -= 16;
            ln[0]='\0'; strAppend(ln,"Promedio tiempo: "); strAppend(ln,tC); strAppend(ln," min");
            pdfBTB(tmp, MARGIN, y, 10, ln); y -= 22;

            pdfBTB(tmp, MARGIN, y, 12, "Detalle de Vehiculos"); y -= 18;
        } else {
            char cont[40]; cont[0]='\0';
            strAppend(cont,"Reporte "); strAppend(cont,fecha); strAppend(cont," (cont.)");
            pdfBTB(tmp, MARGIN, y, 12, cont); y -= 18;
        }

        pdfRect(tmp, MARGIN-2, y-4, PAGE_W-MARGIN*2, HDR_H, 0.22f,0.28f,0.67f);
        tmp << "1 1 1 rg\n";
        for (int c = 0; c < 7; c++) pdfBTB(tmp, COL_X[c], y, 8, COL_HDR[c]);
        tmp << "0 0 0 rg\n";
        y -= HDR_H + 2;

        int maxThisPage = (pg == 0) ? rowsPage1 : rowsPerPage;
        int rowsThisPage = 0;

        while (rowIdx < totalRows && rowsThisPage < maxThisPage && y > 40) {
            const Vehiculo& v = p.historial[rowIdx];
            double mins=0.0, monto=0.0;
            if (v.horaSalida > 0) {
                double secs = difftime(v.horaSalida, v.horaEntrada);
                mins  = secs / 60.0;
                monto = ceil(secs / 3600.0) * p.config.tasaPorHora;
            }
            char dtE[10],dtS[10],minsS[10],montoS[12];
            timeStr(v.horaEntrada, dtE);
            intStr((int)mins, minsS); dblStr(monto, montoS);

            if (rowIdx % 2 == 0)
                pdfRect(tmp, MARGIN-2, y-4, PAGE_W-MARGIN*2, ROW_H, 0.95f,0.95f,0.99f);

            pdfBT(tmp, COL_X[0], y, 8, v.placa);
            pdfBT(tmp, COL_X[1], y, 8, v.marca);
            pdfBT(tmp, COL_X[2], y, 8, v.modelo);
            pdfBT(tmp, COL_X[3], y, 8, dtE);
            if (v.horaSalida > 0) { timeStr(v.horaSalida, dtS); pdfBT(tmp,COL_X[4],y,8,dtS); }
            else pdfBT(tmp, COL_X[4], y, 8, "---");
            pdfBT(tmp, COL_X[5], y, 8, minsS);
            pdfBT(tmp, COL_X[6], y, 8, montoS);

            y -= ROW_H; rowIdx++; rowsThisPage++;
        }

        char pgNum[20]; pgNum[0]='\0';
        strAppend(pgNum,"Pagina "); char pn[5]; intStr(pg+1,pn); strAppend(pgNum,pn);
        pdfBT(tmp, PAGE_W/2 - 20, 20, 8, pgNum);
        tmp.close();

        ifstream measure(tmpNames[pg], ios::binary | ios::ate);
        streamLens[pg] = (long)measure.tellg();
        measure.close();
    }

    ofstream pdf(fname, ios::binary);
    if (!pdf) {
        cout << "  [RPT] No se pudo crear " << fname << "\n";
        for (int pg=0;pg<numPages;pg++) remove(tmpNames[pg]);
        return;
    }

    int totalObjs = 4 + numPages * 2;
    long offsets[50];

    pdf << "%PDF-1.4\n";

    auto writeXrefEntry = [&](long off) {
        char s[11]; s[10]='\0';
        for (int k=9;k>=0;k--){ s[k]='0'+(off%10); off/=10; }
        pdf << s << " 00000 n \n";
    };

    offsets[1] = (long)pdf.tellp();
    pdf << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n\n";

    offsets[2] = (long)pdf.tellp();
    pdf << "2 0 obj\n<< /Type /Pages /Kids [";
    for (int pg=0;pg<numPages;pg++) {
        char n[5]; intStr(5+pg*2,n); pdf << n << " 0 R ";
    }
    char npS[5]; intStr(numPages,npS);
    pdf << "] /Count " << npS << " >>\nendobj\n\n";

    offsets[3] = (long)pdf.tellp();
    pdf << "3 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica "
        << "/Encoding /WinAnsiEncoding >>\nendobj\n\n";

    offsets[4] = (long)pdf.tellp();
    pdf << "4 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold "
        << "/Encoding /WinAnsiEncoding >>\nendobj\n\n";

    for (int pg=0;pg<numPages;pg++) {
        int pageObj    = 5 + pg*2;
        int contentObj = pageObj + 1;
        char poS[5],coS[5],pwS[6],phS[6];
        intStr(pageObj,poS); intStr(contentObj,coS);
        intStr(PAGE_W,pwS);  intStr(PAGE_H,phS);

        offsets[pageObj] = (long)pdf.tellp();
        pdf << poS << " 0 obj\n"
            << "<< /Type /Page /Parent 2 0 R\n"
            << "   /MediaBox [0 0 " << pwS << " " << phS << "]\n"
            << "   /Contents " << coS << " 0 R\n"
            << "   /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >>\n"
            << ">>\nendobj\n\n";

        offsets[contentObj] = (long)pdf.tellp();
        char lenS[15]; intStr((int)streamLens[pg],lenS);
        pdf << coS << " 0 obj\n<< /Length " << lenS << " >>\nstream\n";

        ifstream tmp(tmpNames[pg], ios::binary);
        char buf[512];
        while (tmp.read(buf,512)) pdf.write(buf,tmp.gcount());
        if (tmp.gcount()>0)       pdf.write(buf,tmp.gcount());
        tmp.close();

        pdf << "\nendstream\nendobj\n\n";
    }

    long xrefPos = (long)pdf.tellp();
    char totalS[10]; intStr(totalObjs+1,totalS);
    pdf << "xref\n0 " << totalS << "\n";
    pdf << "0000000000 65535 f \n";
    for (int i=1;i<=totalObjs;i++) writeXrefEntry(offsets[i]);

    char rootS[5]; intStr(1,rootS);
    pdf << "trailer\n<< /Size " << totalS << " /Root " << rootS << " 0 R >>\n";

    char xpS[20]; xpS[0]='\0';
    long xp = xrefPos;
    char xpTmp[20]; int xi=0;
    if(xp==0){xpTmp[xi++]='0';}
    else{ while(xp>0){xpTmp[xi++]='0'+(xp%10);xp/=10;} }
    for(int k=xi-1;k>=0;k--) xpS[xi-1-k]=xpTmp[k]; xpS[xi]='\0';
    pdf << "startxref\n" << xpS << "\n%%EOF\n";
    pdf.close();

    for (int pg=0;pg<numPages;pg++) remove(tmpNames[pg]);
    cout << "  [RPT] PDF generado: " << fname << "\n";
}

void generarTodos(const Parqueo& p) {
    generarCSV(p);
    generarHTML(p);
    generarPDF(p);
}
