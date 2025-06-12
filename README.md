# AnyDesk Log Parser

Acest fișier conține un program C++ care parsează jurnalele (log-urile) aplicației AnyDesk pe Windows 10/11.

## Funcționalități

- Deschide fișierul de log implicit: `C:\ProgramData\AnyDesk\ad_svc.trace`
- Extrage timpul de început și sfârșit al sesiunii
- Extrage adresa IP a conexiunii
- Extrage numărul de biți trimiși (TxBytes) și primiți (RxBytes)
- Sortează sesiunile după timpul de început
- Salvează rezultatele într-un fișier CSV: `anydesk_sessions.csv`

## Compilare și rulare

```bash
g++ -std=c++17 anydesk_parser.cpp -o anydesk_parser.exe
./anydesk_parser.exe
```
