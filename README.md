# PenPlotter Firmware

Firmware für einen CoreXY-Stiftplotter auf Basis des ESP32-Mikrocontrollers. Entwickelt mit PlatformIO (Arduino + FreeRTOS), geschrieben in C++20.

## Funktionsübersicht

- **CoreXY-Kinematik** mit TMC2209-Schrittmotortreibern (sensorloses Homing via StallGuard)
- **G-Code-Interpreter** mit Unterstützung für Linien, Kreisbögen und Bézierkurven
- **Web-Interface** zur Dateiübertragung und Fernsteuerung über WLAN
- **LCD-Benutzeroberfläche** (20×4 Zeichen) mit Drehgeber-Navigation
- **RTOS-Architektur** – vollständig aufgabenbasiert mit FreeRTOS-Queues

---

## Hardware

| Komponente        | Details                                              |
|-------------------|------------------------------------------------------|
| Mikrocontroller   | ESP32 (DOIT DevKit V1)                               |
| Schrittmotortreiber | TMC2209 (UART-gesteuert, R_SENSE = 0,11 Ω)         |
| Display           | 20×4 I²C LCD (Adresse 0x27, SDA=21, SCL=22)        |
| Eingabe           | Drehgeber mit Taster (DT=26, CLK=27, SW=25)         |
| Stiftaktor        | Servo an Pin 13                                      |
| Summer            | Passiver Buzzer an Pin 14                            |
| Motor A           | STEP=19, DIR=4                                       |
| Motor B           | STEP=18, DIR=5                                       |
| Enable-Pin        | 23                                                   |
| Treiber UART      | TX=17, RX=16                                         |

### Mechanik & Kinematik

- **Arbeitsbereich**: 185 mm × 265 mm
- **Schritte pro mm**: 5,0 (16-fach Mikroschritt, 1000 mA Treiberstrom)
- Sensorloses Homing: Der TMC2209 erkennt Lastspitzen (StallGuard) – keine physischen Endschalter erforderlich.

---

## Unterstützte G-Code-Befehle

| Befehl     | Funktion                                                        |
|------------|-----------------------------------------------------------------|
| `G0` / `G1` | Linearbewegung (Eilgang / Zeichnen), Parameter: `X`, `Y`, `F` |
| `G2` / `G3` | Kreisbogen (im/gegen Uhrzeigersinn), Parameter: `X`, `Y`, `I`, `J` |
| `G5`       | Quadratische Bézierkurve (Parameter: `X`, `Y`, `C`, `D`)       |
| `G5.1`     | Kubische Bézierkurve (Parameter: `X`, `Y`, `A`, `B`, `C`, `D`) |
| `G28`      | Alle Achsen referenzfahren (Homing)                             |
| `G90` / `G91` | Absolut- / Relativmodus                                     |
| `M3`       | Stift absenken (Servo auf 65°)                                  |
| `M5`       | Stift heben (Servo auf 100°)                                    |

Der Vorschub wird automatisch anhand des Stiftzustands gewählt (Zeichnen: 20 mm/s, Eilgang: 50 mm/s), kann aber mit `F` überschrieben werden.

---

## Web-Interface

Das Gerät verbindet sich mit dem konfigurierten WLAN und ist unter `penPlttr.local` erreichbar (mDNS-Name konfigurierbar).

- **HTTP-Server** auf Port 80 – Datei-Upload (G-Code, max. 10 MB)
- **WebSocket-Server** auf Port 81 – Echtzeit-Kommunikation

Funktionen des Web-Interfaces:
- G-Code-Dateien hochladen, verwalten und Druckaufträge starten
- Aufträge pausieren, fortsetzen und abbrechen
- Einzelne G-Code-Befehle senden
- Maschinenzustand in Echtzeit abrufen (Position, Fortschritt, Stiftzustand)
- Alle Laufzeit-Einstellungen als JSON lesen und schreiben

---

## Benutzeroberfläche (LCD)

Die UI wird über einen Drehgeber gesteuert und enthält folgende Bildschirme:

- **Startbildschirm** (Home)
- **Manuelle Steuerung**
- **Druckauftrag** (Plotting) – Fortschrittsanzeige, Pause/Abbruch
- **Einstellungen** – alle konfigurierbaren Parameter

Akustisches Feedback über den Buzzer: Melodie bei Auftragsstart, -abschluss und -abbruch.

---

## Einstellungen & Konfiguration

Das System unterscheidet zwischen:

- **Compile-Zeit-Standardwerten** in `src/settings/defaults/`
- **Laufzeit-Einstellungen** (`RuntimeSettings`) – thread-sichere Werte im Betrieb
- **Persistierten Einstellungen** (`SettingPersistence`) – werden im Flash gespeichert und beim Start geladen

Konfigurierbare Parameter (Auswahl):

| Kategorie      | Parameter                                                |
|----------------|----------------------------------------------------------|
| Motoren        | Treiberstrom, Mikroschritt-Auflösung, Vorschubgeschwindigkeiten |
| Homing         | Geschwindigkeit, StallGuard-Schwellwert, Rückfahrschritte, Timeout |
| Stift          | Servo-Winkel für oben/unten, Stiftplatz-Daten (Farbe, Strichbreite) |
| WLAN           | SSID, Passwort, mDNS-Name                               |

---

## Dateisystem

Das ESP32-Flash wird mit **LittleFS** genutzt:

- `plotting/` – fertige G-Code-Dateien
- `tmp/` – temporärer Bereich für laufende Uploads

---

## Projekt aufsetzen

### Voraussetzungen

- [PlatformIO](https://platformio.org/) (VS Code Extension oder CLI)
- ESP32-Board (DOIT DevKit V1 oder kompatibel)

### Bauen & Flashen

```bash
# Firmware bauen und flashen
pio run --target upload

# Dateisystem-Image flashen (LittleFS)
pio run --target uploadfs
```

### Konfiguration anpassen

Hardwarespezifische Einstellungen befinden sich in `src/config/`:

| Datei                    | Inhalt                                      |
|--------------------------|---------------------------------------------|
| `HardwareConfig.hpp`     | Arbeitsbereich, Schritte/mm, Strom          |
| `PinsConfig.hpp`         | Pin-Belegung aller Komponenten              |
| `PenSlotsConfig.hpp`     | Anzahl der Stiftplätze                      |
| `DirectoriesConfig.hpp`  | Verzeichnisnamen im Dateisystem             |
| `UiConfig.hpp`           | Display-Adresse, Entprellzeit               |
| `VersionConfig.hpp`      | Firmware-Version                            |

Standard-Laufzeitwerte können in `src/settings/defaults/` angepasst werden.

---

## Architektur

```
main.cpp
  └── startRtosTasks()
        ├── plottingTask   ← G-Code-Parsing, Bewegungsausführung
        └── systemTask     ← UI, Web-Interface, Einstellungen
```

Die Kommunikation zwischen Tasks erfolgt über **FreeRTOS-Queues** (`RtosQueue`). Komponenten sind über das **Observer-Pattern** entkoppelt (`JobObserver`, `SettingObserver`, `FileObserver`).

---

## Abhängigkeiten

| Bibliothek         | Zweck                                    |
|--------------------|------------------------------------------|
| TMCStepper         | TMC2209-Treiberkommunikation (UART)      |
| LCD-I2C            | I²C LCD-Ansteuerung                      |
| ESP32Servo         | Servo-PWM                                |
| WebSockets         | WebSocket-Server (Port 81)               |
| LittleFS           | Dateisystem auf ESP32-Flash              |
