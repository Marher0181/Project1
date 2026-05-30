#include "utils.h"
#include <ctime>

bool strIgual(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' || b[i] != '\0') {
        if (a[i] != b[i]) return false;
        i++;
    }
    return true;
}

void strCopiar(char* dst, const char* src, int n) {
    int i = 0;
    while (i < n - 1 && src[i] != '\0') { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

void strAppend(char* dst, const char* src) {
    int i = 0; while (dst[i]) i++;
    int j = 0; while (src[j]) dst[i++] = src[j++];
    dst[i] = '\0';
}

void intStr(int v, char* buf) {
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[20]; int i = 0; bool neg = v < 0;
    if (neg) v = -v;
    while (v > 0) { tmp[i++] = '0' + (v % 10); v /= 10; }
    if (neg) tmp[i++] = '-';
    int j = 0;
    for (int k = i - 1; k >= 0; k--) buf[j++] = tmp[k];
    buf[j] = '\0';
}

void dblStr(double v, char* buf) {
    int e = (int)v;
    int d = (int)((v - e) * 100 + 0.5);
    char es[20], ds[5];
    intStr(e, es); intStr(d, ds);
    int i = 0;
    for (int k = 0; es[k]; k++) buf[i++] = es[k];
    buf[i++] = '.';
    if (d < 10) buf[i++] = '0';
    for (int k = 0; ds[k]; k++) buf[i++] = ds[k];
    buf[i] = '\0';
}

void fechaStr(time_t t, char* buf)  { strftime(buf, 12, "%Y-%m-%d",          localtime(&t)); }
void timeStr (time_t t, char* buf)  { strftime(buf, 10, "%H:%M:%S",          localtime(&t)); }
void dtStr   (time_t t, char* buf)  { strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&t)); }

void formatTiempo(double secs, char* buf) {
    int h = (int)secs / 3600;
    int m = ((int)secs % 3600) / 60;
    int s = (int)secs % 60;
    buf[0]='0'+h/10; buf[1]='0'+h%10; buf[2]='h'; buf[3]=' ';
    buf[4]='0'+m/10; buf[5]='0'+m%10; buf[6]='m'; buf[7]=' ';
    buf[8]='0'+s/10; buf[9]='0'+s%10; buf[10]='s'; buf[11]='\0';
}
