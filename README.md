# Eraclea-1

🛰️ **Simulatore di Satellite Spaziale Eraclea-1**

Un simulatore completo di un satellite spaziale che implementa il protocollo PUS (Packet Utilization Standard) utilizzato dalle agenzie spaziali come ESA.

## 🚀 Caratteristiche

### Sistema Multi-Task
- **Sensor Task**: Raccolta dati da sensori simulati
- **Processing Task**: Elaborazione dati e generazione telemetria
- **Health Task**: Monitoraggio stato sistema e telecomandi
- **TM Sender Task**: Invio telemetria verso terra

### Protocollo PUS
- Implementazione completa del protocollo PUS per telecomandi (TC) e telemetria (TM)
- Buffer circolari thread-safe per comunicazione
- Gestione errori e ACK/NACK

## Come Usare

### Avvio
```bash
make
./eraclea1
```

### Sequenza Operativa
1. **Accendi il satellite** (opzione 1) - Avvia tutti i sistemi
2. **Abilita acquisizione dati** (opzione 2) - I sensori iniziano a raccogliere dati
3. **Monitora lo stato** (opzione 5) - Controlla il funzionamento
4. **Spegni il sistema** (opzione 3) - Arresto sicuro

### Telecomandi Disponibili
- **Service 1**: Controllo potenza
  - Subtype 1: Switch ON
  - Subtype 2: Enable Data
  - Subtype 3: Shutdown
- **Service 17**: Test
- **Service 3**: Housekeeping

## 🏗️ Architettura

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   SENSORS       │ -> │     BUFFER      │ -> │   PROCESSING    │
│   (Thread)      │    │   (Circular)    │    │   (Thread)      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   TM MANAGER    │ <- │   TC HANDLER    │    │   HEALTH TASK   │
│   (Buffer)      │    │   (Commands)    │    │   (Monitor)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
┌─────────────────┐
│   GROUND OUTPUT │
│   (Telemetry)   │
└─────────────────┘
```

## 📁 Struttura Progetto

```
eraclea-1/
├── main.c              # Entry point con menu interattivo
├── Makefile            # Build system
├── README.md           # Questa documentazione
├── core/
│   ├── system.c        # Sistema principale e stati
│   └── system.h        # Header sistema
├── comm/
│   ├── pus.h           # Definizioni PUS
│   ├── pus.c           # Utilità PUS
│   ├── tc_handler.c    # Gestore telecomandi
│   ├── tc_handler.h    # Header TC handler
│   ├── tm_manager.c    # Gestore telemetria
│   └── tm_manager.h    # Header TM manager
├── data/
│   ├── buffer.c        # Buffer circolare
│   └── buffer.h        # Header buffer
├── sensor/
│   ├── sensor.c        # Simulazione sensori
│   └── sensor.h        # Header sensori
├── platform/
│   ├── platform.c      # Astrazione piattaforma
│   └── platform.h      # Header platform
└── ground/
    ├── ground_sim.c    # Simulazione stazione terra
    └── ground_sim.h    # Header ground sim
```

## 🔧 Compilazione

```bash
make clean
make
```

## 🧪 Test

```bash
./test_demo.sh  # Script di test automatico
```

## 📊 Stati del Sistema

- **POWER_OFF**: Sistema spento, nessuna attività
- **POWER_ON**: Sistema acceso ma acquisizione dati disabilitata
- **DATA_ENABLED**: Acquisizione e processamento attivi
- **ERROR**: Stato di errore
- **SHUTDOWN**: Spegnimento in corso

## 🎯 Protocollo PUS

Il sistema implementa il **Packet Utilization Standard (PUS)** utilizzato nei satelliti reali:

- **Telecomandi (TC)**: Comandi dalla terra al satellite
- **Telemetria (TM)**: Dati dal satellite alla terra
- **Servizi standard**: Test, housekeeping, controllo potenza

## 🤝 Contributi

Progetto educativo per dimostrare concetti di:
- Sistemi embedded real-time
- Programmazione multi-thread
- Protocolli di comunicazione spaziale
- Architetture software robuste