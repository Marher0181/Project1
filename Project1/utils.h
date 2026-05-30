#ifndef UTILS_H
#define UTILS_H

#include <ctime>

void strIgual_impl(const char* a, const char* b, bool& out);
bool strIgual(const char* a, const char* b);
void strCopiar(char* dst, const char* src, int n);
void strAppend(char* dst, const char* src);
void intStr(int v, char* buf);
void dblStr(double v, char* buf);
void fechaStr(time_t t, char* buf);
void timeStr(time_t t, char* buf);
void dtStr(time_t t, char* buf);
void formatTiempo(double secs, char* buf);

#endif
