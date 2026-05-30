# Sistema de Parqueo

## Requisitos
- Visual Studio 2022 Community
- MySQL Server 8.0

## Configuración antes de compilar

1. Abrir `Project1.sln` en Visual Studio
2. Ir a `Project → Properties → C/C++ → Additional Include Directories`
   Cambiar por: `C:\Program Files\MySQL\MySQL Server 8.0\include`
3. Ir a `Linker → General → Additional Library Directories`
   Cambiar por: `C:\Program Files\MySQL\MySQL Server 8.0\lib`
4. Copiar al lado del `.exe`:
   - `libmysql.dll`
   - `libssl-3-x64.dll`
   - `libcrypto-3-x64.dll`
   (Todas en `C:\Program Files\MySQL\MySQL Server 8.0\lib\`)

## Ejecutar
Abrir CMD en la carpeta del `.exe` y ejecutar `Project1.exe`

BRRRRRRRRRRRRRRRRRRRRRR
Real Hasta La Muerte