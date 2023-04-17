# IPK-projekt2 - IOTA

## Autor: Jan Novák <xnovak3i@stud.fit.vutbr.cz>

## Popis projektu

Cílem projektu je implementace serverové části kalkulacky. Server bude umět provádět základní matematické operace a bude komunikovat s klientem pomocí TCP nebo UDP protokolu. Program zvládne zpracovat více klientů současně.

## Popis implementace

Program je implementován v jazyce C a využívá knihovny pro síťovou komunikaci a správu procesů. Podle vstuúních parametrů je vybrána buď TCP nebo UDP komunikace.

### TCP implementace

Program čeká na navázání spojení s klietem. Následně se od programu oddělí 
proces, který bude komunikovat s klientem. V případě, že se nepodaří vytvořit nový proces, program se ukončí s chybovou hláškou. K rozdělení procesu byla využita funkce `fork()`, která nám umožní zajistit že původní proces čeká na nového klienta a nový proces se může starat o toho stávajícího bez rizika, že dojde ke konfliktům v kominikaci. V novém procesu se program pokusí začit komunikaci s klientem a čeká na příchozí příkaz `HELLO`, pomocí něhož je zahájena komunikace se serverem.<br>
Následně se program pokusí zpracovat příkazy od klienta. Pokud se na vstupu objeví příkaz začínající slovem `SOLVE` zkontroluje zda následující sekvence znakú v daném packetu odpovídá validnímu příkazu a pokud ano, provede výpočet a odešle odpověď klientovi. Pokud se na vstupu objeví jiný příkaz, program odešle uživateli `BYE` a komunikace se ukončí a to včetně procesu, který komunikoval se klientem. Stejným způsobem se program chová, pokud se na vstupu objeví příkaz `BYE`.<br>

### UDP implementace

Program čeká na příchozí packet od klienta. Ve chvíly, kdy packet přijde, program zkontroluje zda je packet validní a pokud ano, zpracuje příkaz a odešle odpověď klientovi. Pokud je packet nevalidní, program odešle klientovi hlášku o špatném formátu packetu. Program se neukončí, ale pokračuje v čekání na další packet.
#### Popis UDP packetu
Odchozí pacekt je tvořen 3 částmi. První část je `Opcode`, který udáva, za je packet požadavek na server nebo odpověď. Druhá je velikost packety v bajtech a třetí je data, která obsahují příkaz
```c
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +---------------+---------------+-------------------------------+
 |     Opcode    |Payload Length |          Payload Data         |
 |      (8)      |      (8)      |                               |
 +---------------+---------------+ - - - - - - - - - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
```
Příchozí packet je tvořen 4 částmi. První část je `Opcode`, který udáva, za je packet požadavek na server nebo odpověď. Druhá je `Status`, který udáva, zda byl příkaz úspěšně zpracován nebo ne. Třetí je velikost packetu v bajtech a čtvrtá je data, která obsahují odpověď na příkaz.

```c
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +---------------+---------------+---------------+---------------+
 |     Opcode    |  Status Code  |Payload Length | Payload Data  |
 |      (8)      |      (8)      |      (8)      |               |
 +---------------+---------------+---------------+ - - - - - - - +
 :                     Payload Data continued ...                :
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                     Payload Data continued ...                |
 +---------------------------------------------------------------+
```

| Opcode   | Hodnota |
|:---------|:------|
| Požadavek  | 0     |
| Odpověd | 1     |

| Status | Hodnota |
|:-------|:------|
| OK     | 0     |
| Error  | 1     |

## Testování

K otestování byl použi můj program z projektu 1, který je schopen komunikovat s serverem pomocí TCP nebo UDP protokolu a nový program, který je schopen komunikovat s klientem.<br>
Z důvodu komplikovanosti testovacího prostředí na NixOS jsem přistoupil k manuálnímu testování.
Testovaly se následující případy:<br>
- ### Komunikace s klientem pomocí TCP protokolu<br>
  - Se správnou adresou a portem<br>
    - Validni příkazy - všechny výsledky seděly<br>
    - Nevalidní otevírací příkaz - server ukončil komunikaci odesláním `BYE`<br>
    - Nevalidní uzavírací příkaz - server ukončil komunikaci odesláním `BYE`<br>
    - Nevalidní příkaz výpočtu - server ukončil komunikaci odesláním `BYE`<br>
  - S rozdílnou adresou
    - klientovi se nepodařilo navázat spojení<br>
  - S rozdílným portem
    - klientovi se nepodařilo navázat spojení<br>
  - S více klienty současně
    - Zprávy mezi klienty a serverem se zasílaly pouze danému klientovi<br>
- ### Komunikace s klientem pomocí UDP protokolu<br>
  - Se správnou adresou a portem<br>
    - Validni příkazy - všechny výsledky seděly<br>
    - S nevalidním příkazem - server odeslal klientovi packet se Status Code 1<br>
  - S rozdílnou adresou
    - klientovi se nepodařilo navázat spojení<br>
  - S rozdílným portem
    - klientovi se nepodařilo navázat spojení<br>

  
## Použité zdroje

NESFIT. (2023). *Protocol.md*. [Online]. Poslední aktualizace: 3. března 2023 Dostupné z: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md<br>
NESFIT. (2023). *server.c*. [Online]. Poslední aktualizace: 30. ledna 2023 Dostupné z: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp/DemoUdp/server.c




