# ESPHome Ni1000 Raumfühler-Simulation für Brötje HR MICRO G

ESP32-basierte Smart-Heizungssteuerung, die einen Ni1000 Raumtemperaturfühler (QAA 36.2) für Brötje HR MICRO G / Siemens RVP 76.132 Heizungsregler simuliert.

## ✨ Features

- **Raumtemperatur-Simulation**: Simuliert einen Ni1000 Raumfühler über Digital-Potentiometer
- **Multi-Raum-Durchschnitt**: Berechnet virtuelle Raumtemperatur aus mehreren Home Assistant Sensoren
- **8× DS18B20 Monitoring**: Überwacht alle Heizkreise (Vorlauf/Rücklauf)
- **Intelligente Statuserkennung**: Erkennt aktive Heizkreise mit gleitendem Durchschnitt (kein Flackern bei taktenden Pumpen)
- **Nachtabsenkung-Erkennung**: Erkennt automatisch Absenkphasen durch Vergleich von 30min- und 1h-Durchschnitt
- **Betriebsmodi**: Automatik, Schnellaufheizen, Absenkbetrieb, Manuell
- **Non-Volatile**: MCP4162 behält Widerstandswert bei Stromausfall
- **Optimierte Schreibzyklen**: Poti wird nur bei Änderung beschrieben

## 🔧 Hardware

### Bauteile

| Bauteil | Typ | Ca. Preis |
|---------|-----|-----------|
| ESP32 DevKit V1 | ESP32-WROOM-32 | 5-8€ |
| Digital-Potentiometer | MCP4162-502E/P (5kΩ, DIP-8, Non-Volatile) | 2-3€ |
| Temperatursensoren | DS18B20 (8 Stück) | 8-15€ |
| Vorwiderstand | 1kΩ (0,25W) | 0,10€ |
| Parallelwiderstand | 180Ω (0,25W) | 0,10€ |
| Pull-up Widerstand | 4,7kΩ (für 1-Wire Bus) | 0,10€ |
| Netzteil | 5V/1A USB | 3-6€ |
| Gehäuse | optional | 3-8€ |

**Gesamtkosten: ca. 25-45€**

### Schaltplan

```
                         MCP4162-502E/P (DIP-8)
                           ┌────────┐
             GPIO5    CS ──┤1      8├── VDD ── 3.3V
             GPIO18  SCK ──┤2      7├── P0B ───────────┬─────────── Klemme M (Heizung)
             GPIO23  SDI ──┤3      6├── P0W ───── 180Ω ┴──── 1kΩ ── Klemme B5 (Heizung)
                     GND ──┤4      5├── P0A    (parallel)
                           └────────┘         


Klemme B5 (Heizung)
       │
     ┌─┴─┐
     │1kΩ│  Vorwiderstand
     └─┬─┘
       │
       ┴────────────────┬──────── P0W (Pin 6, Schleifer)
                        │
                      ┌─┴─┐
                      │   │
                      │180│  Parallelwiderstand
                      │ Ω │
                      │   │
                      └─┬─┘
                        │
       ┬────────────────┴──────── P0B (Pin 7, Terminal B)
       │
 Klemme M (Heizung)


DS18B20 Sensoren (alle parallel):

                     ┌─────────────┐
3.3V ────┬───────────┼── VDD       │
         │4,7kΩ      │             │
GPIO0 ───┴───────────┼── DQ    DS18B20 (×8)
                     │             │
GND ─────────────────┼── GND       │
                     └─────────────┘
```

### Pinbelegung ESP32

| GPIO | Funktion |
|------|----------|
| GPIO5 | MCP4162 CS (Chip Select) |
| GPIO18 | MCP4162 SCK (Clock) |
| GPIO23 | MCP4162 SDI (Data In) |
| GPIO0 | DS18B20 1-Wire Bus |
| GPIO2 | Status LED |

### Anschluss an Heizung

| Heizungsklemme | Verbindung |
|----------------|------------|
| B5 | → 1kΩ → Parallelschaltung → MCP4162 P0W |
| M | → MCP4162 P0B |

## 📁 Dateien

```
├── esphome-heizung-raumfuehler.yaml  # ESPHome Hauptkonfiguration
├── mcp4162.h                          # MCP4162 SPI-Treiber (ESP-IDF)
├── LICENSE                            # MIT Lizenz
└── README.md
```

## 🚀 Installation

### 1. Voraussetzungen

- [ESPHome](https://esphome.io/) installiert
- Home Assistant mit ESPHome Integration
- Temperatursensoren in Home Assistant (für virtuelle Raumtemperatur)

### 2. Dateien kopieren

```bash
# In dein ESPHome Konfigurationsverzeichnis
cp esphome-heizung-raumfuehler.yaml /config/esphome/
cp mcp4162.h /config/esphome/
```

### 3. Konfiguration anpassen

**WLAN-Zugangsdaten** in `esphome-heizung-raumfuehler.yaml`:
```yaml
wifi:
  ssid: "DEIN-WLAN"              # Anpassen!
  password: "DEIN-PASSWORT"      # Anpassen!
  domain: .domain.local          # Anpassen!
  manual_ip:
    static_ip: xxx.xxx.xxx.xxx   # Anpassen!
    gateway: xxx.xxx.xxx.xxx     # Anpassen!
    subnet: xxx.xxx.xxx.xxx      # Anpassen!
    dns1: xxx.xxx.xxx.xxx        # Anpassen!
```

**Home Assistant Sensoren** für virtuelle Raumtemperatur:
```yaml
- platform: homeassistant
  id: temp_wohnzimmer
  entity_id: sensor.dein_wohnzimmer_temperatur  # Anpassen!
  internal: true

- platform: homeassistant
  id: temp_kinderzimmer
  entity_id: sensor.dein_kinderzimmer_temperatur  # Anpassen!
  internal: true
```

### 4. DS18B20 Adressen ermitteln

Beim ersten Flash werden die Sensoren noch nicht erkannt. So findest du die Adressen:

1. Flashe den ESP32 mit der Konfiguration
2. Öffne die Logs im ESPHome Dashboard
3. Suche nach `Found sensors:` – dort stehen die 64-bit Adressen
4. Trage die Adressen in der YAML ein (alle mit `# Anpassen!` markiert)
5. Flashe erneut

### 5. Kompilieren und Flashen

```bash
esphome run esphome-heizung-raumfuehler.yaml
```

## 📊 Sensoren & Entitäten

### Temperatursensoren (DS18B20)

| Sensor | Beschreibung |
|--------|--------------|
| Temp. Fußboden Vorlauf/Rücklauf | Fußbodenheizung |
| Temp. Radiator Vorlauf/Rücklauf | Heizkörper |
| Temp. Kamin Vorlauf/Rücklauf | Kamin-Wärmetauscher |
| Temp. Wasserspeicher Vorlauf/Rücklauf | Warmwasserspeicher |

### Berechnete Werte

| Sensor | Beschreibung |
|--------|--------------|
| Spreizung Fußboden/Radiator/Kamin/Wasserspeicher | Temperaturdifferenz VL-RL |
| Temp. Ø30min Fußboden Vorlauf | Gleitender Durchschnitt (30 Min) |
| Temp. Ø30min Radiator Vorlauf | Gleitender Durchschnitt (30 Min) |
| Temp. Ø1h Fußboden Vorlauf | Gleitender Durchschnitt (1 Stunde) |
| Temp. Ø1h Radiator Vorlauf | Gleitender Durchschnitt (1 Stunde) |
| Virtuelle Raumtemperatur | Durchschnitt der HA-Sensoren |
| Simulierter Ni1000 Widerstand | Aktueller Widerstandswert |

### Status-Sensoren

| Sensor | Bedingung | Hinweis |
|--------|-----------|---------|
| Kreislauf Fußboden aktiv | Vorlauf-Ø30min > 25,5°C | Gleitender Durchschnitt glättet Takten |
| Kreislauf Radiator aktiv | Vorlauf-Ø30min > 30°C | Gleitender Durchschnitt glättet Takten |
| Kreislauf Kamin aktiv | Rücklauf > 30°C | |
| Kreislauf Warmwasser aktiv | Vorlauf > 50°C | Wird gerade geladen |
| Warmwasserbedarf | Vorlauf < 25°C | Speicher ist kalt |
| Heizung aktiv | FBH ODER Radiator aktiv | |
| Nachtabsenkung Fußboden | Ø30min ≤ 25,5°C UND Ø1h - Ø30min ≥ 3K | Erkennt Absenkphasen |
| Nachtabsenkung Radiator | Ø30min ≤ 30°C UND Ø1h - Ø30min ≥ 4K | Erkennt Absenkphasen |

### Steuerung

| Entität | Typ | Beschreibung |
|---------|-----|--------------|
| Temperatur Offset | Number | -6K bis +6K Korrektur |
| Manuelle Temperatur | Number | 5°C bis 35°C |
| Manueller Modus | Switch | Fixe Temperatur statt Durchschnitt |
| Schnellaufheizen | Switch | Offset -4K (Heizung denkt es ist kälter) |
| Absenkbetrieb | Switch | Offset +4K (Heizung denkt es ist wärmer) |

## 🔬 Technische Details

### Ni1000 Kennlinie

Der Ni1000 ist ein Nickel-Temperatursensor mit **positiver** Kennlinie:

```
R = 4,2 × T + 1021

Beispiele:
10°C → 1063Ω
20°C → 1105Ω
30°C → 1147Ω
```

### Widerstandsberechnung

```
Gesamtwiderstand = Vorwiderstand + Parallelschaltung(Poti, 180Ω)

Mit:
- Vorwiderstand: 1000Ω (fest)
- Parallelwiderstand: 180Ω
- MCP4162: 0-5000Ω (257 Stufen)

Effektiver Bereich: 1000Ω - 1165Ω
Auflösung: ~0,4°C pro Wiper-Stufe
```

### SPI-Protokoll MCP4162

```
16-bit Kommando: [Addr(4bit)][Cmd(2bit)][Data(10bit)]
- Wiper 0 Adresse: 0x00
- Write Command: 0b00
- 257 Stufen (0-256)
```

### Statuserkennung mit gleitendem Durchschnitt

Die Heizungspumpen takten häufig (an/aus im Minutentakt). Um Flackern der Status-Anzeige zu vermeiden, werden **gleitende Durchschnitte** verwendet:

| Durchschnitt | Zweck |
|--------------|-------|
| Ø30min | Statuserkennung (FBH/Radiator aktiv) |
| Ø1h | Nachtabsenkung-Erkennung |

```yaml
filters:
  - sliding_window_moving_average:
      window_size: 30    # 30 Messungen = 30 Minuten
      send_every: 1
```

### Nachtabsenkung-Erkennung

Die Nachtabsenkung wird erkannt durch Vergleich von 30min- und 1h-Durchschnitt:

```
Nachtabsenkung FBH = (Ø30min ≤ 25,5°C) UND (Ø1h - Ø30min ≥ 3K)
Nachtabsenkung Radiator = (Ø30min ≤ 30°C) UND (Ø1h - Ø30min ≥ 4K)
```

Wenn die Temperatur schnell fällt (30min-Wert deutlich unter 1h-Wert), ist die Heizung in Absenkung.

### Optimierte Schreibzyklen

Der MCP4162 wird nur beschrieben, wenn sich der Wiper-Wert tatsächlich ändert:

```cpp
if (wiper != id(last_wiper)) {
  mcp4162.setWiper(wiper);
  id(last_wiper) = wiper;
}
```

Dies schont das EEPROM und reduziert SPI-Traffic.

## 🏠 Home Assistant Integration

Nach dem Flashen erscheint der ESP32 automatisch in Home Assistant.

### Beispiel-Automatisierung: Nachtabsenkung

```yaml
automation:
  - alias: "Heizung Nachtabsenkung"
    trigger:
      - platform: time
        at: "22:00:00"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.heizung_raumfuhler_absenkbetrieb

  - alias: "Heizung Tagbetrieb"
    trigger:
      - platform: time
        at: "06:00:00"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.heizung_raumfuhler_absenkbetrieb
```

### Beispiel-Dashboard (YAML)

```yaml
type: entities
title: Heizungssteuerung
entities:
  - entity: sensor.heizung_raumfuhler_virtuelle_raumtemperatur
  - entity: sensor.heizung_raumfuhler_heizungsstatus
  - entity: sensor.heizung_raumfuhler_betriebsmodus
  - type: divider
  - entity: binary_sensor.heizung_raumfuhler_kreislauf_fussboden_aktiv
  - entity: binary_sensor.heizung_raumfuhler_kreislauf_radiator_aktiv
  - entity: binary_sensor.heizung_raumfuhler_kreislauf_kamin_aktiv
  - entity: binary_sensor.heizung_raumfuhler_kreislauf_warmwasser_aktiv
  - type: divider
  - entity: binary_sensor.heizung_raumfuhler_nachtabsenkung_fussboden
  - entity: binary_sensor.heizung_raumfuhler_nachtabsenkung_radiator
  - type: divider
  - entity: switch.heizung_raumfuhler_schnellaufheizen
  - entity: switch.heizung_raumfuhler_absenkbetrieb
  - entity: switch.heizung_raumfuhler_manueller_modus
  - entity: number.heizung_raumfuhler_temperatur_offset
```

### Beispiel-Temperatur-Graph

```yaml
type: custom:apexcharts-card
header:
  title: Heizkreise
graph_span: 24h
series:
  - entity: sensor.heizung_raumfuhler_temp_fussboden_vorlauf
    name: FBH Vorlauf
  - entity: sensor.heizung_raumfuhler_temp_fussboden_rucklauf
    name: FBH Rücklauf
  - entity: sensor.heizung_raumfuhler_temp_radiator_vorlauf
    name: Radiator Vorlauf
  - entity: sensor.heizung_raumfuhler_temp_wasserspeicher_vorlauf
    name: Warmwasser Vorlauf
```

### Beispiel: Nachtabsenkung-Benachrichtigung

```yaml
automation:
  - alias: "Benachrichtigung Nachtabsenkung aktiv"
    trigger:
      - platform: state
        entity_id: binary_sensor.heizung_raumfuhler_nachtabsenkung_fussboden
        to: "on"
    action:
      - service: notify.mobile_app
        data:
          message: "Heizung ist in Nachtabsenkung"
```

## ⚠️ Sicherheitshinweise

- Arbeiten an der Heizung nur im **spannungslosen Zustand**
- Original-Sicherheitsfunktionen der Heizung bleiben erhalten
- Bei ESP32-Ausfall behält der MCP4162 den letzten Widerstandswert (Non-Volatile)
- Keine Gewährleistung – Nutzung auf eigene Gefahr

## 📝 Lizenz

MIT License – siehe [LICENSE](LICENSE)

## 🙏 Credits

- [ESPHome](https://esphome.io/) – Firmware-Framework
- [Home Assistant](https://www.home-assistant.io/) – Smart Home Plattform
