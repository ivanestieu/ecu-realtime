# Architecture ECU — Régulateur de Vitesse
> Document de référence — à joindre aux futures conversations

---

## C'est quoi ce projet ?

Un programme embarqué sur **ESP32** qui simule un régulateur de vitesse de voiture.

```
PC (script Python)          ESP32 (notre ECU)
──────────────────          ─────────────────
Envoie SPEED       ──▶      Reçoit et décode
Envoie SETPOINT    ──▶      Met à jour l'état
Envoie MODE        ──▶      Calcule la commande
                   ◀──      Envoie OUTPUT (commande moteur)
                   ◀──      Envoie STATS (toutes les 1s)
                   ◀──      Envoie ALARM (si problème)
```

---

## Le protocole — format d'une trame

```
[0xAA] [LEN 2 octets] [TYPE 1 octet] [PAYLOAD N octets] [CRC 1 octet]
  ↑          ↑              ↑                ↑                ↑
début     taille        type de          la donnée        vérification
          du message    message                           d'intégrité
```

**CRC** = XOR de tous les octets sauf le 0xAA de début.

### Types de messages

| ID | Nom | Payload | Description |
|---|---|---|---|
| 0x01 | SETPOINT | float 4o | Consigne de vitesse |
| 0x02 | SPEED | float 4o | Vitesse actuelle |
| 0x05 | MODE_SET | uint8 1o | 0=OFF, 1=MANUAL, 2=AUTO |
| 0x80 | OUTPUT | float 4o | Commande moteur (on envoie ça) |
| 0x83 | STATS | uint32 * N | Télémétrie (on envoie ça) |
| 0x85 | ALARM | string | Alerte critique (on envoie ça) |
| 0xFF | DBG | string | Debug (on envoie ça) |

### Validation d'une trame (dans l'ordre)
```
1. Commence par 0xAA ?        non → jeter
2. LEN cohérent ?             non → jeter
3. CRC calculé == CRC reçu ?  non → jeter + incrémenter compteur erreurs
                              oui → traiter !
```

---

## Structure des fichiers

```
ecu/
├── main/
│   ├── main.c                   ← point d'entrée, init + lancement des tâches
│   │
│   ├── frame/
│   │   ├── parser.h / parser.c  ← décode les trames reçues (RX)
│   │   └── builder.h/builder.c  ← construit les trames à envoyer (TX)
│   │
│   ├── pid/
│   │   ├── pid.h / pid.c        ← algorithme PID pur (pas de FreeRTOS)
│   │
│   ├── shared/
│   │   ├── shared_state.h/.c    ← variables partagées + mutex
│   │
│   └── tasks/
│       ├── task_rx.c            ← tâche réception UART
│       ├── task_pid.c           ← tâche calcul PID (100ms)
│       ├── task_telemetry.c     ← tâche télémétrie (1s)
│       └── task_failsafe.c      ← tâche sécurité
│
└── CMakeLists.txt
```

**Total : ~12 fichiers** — un fichier = une responsabilité.

---

## Rôle de chaque fichier

### `parser.h / parser.c`
- Lit les octets entrants **un par un** (machine à états)
- Reconstitue une trame complète
- Vérifie le CRC
- Retourne une `frame_t` décodée si valide

```c
// Utilisation :
frame_t trame;
if (parser_push_byte(octet, &trame)) {
    // trame complète et valide !
}
```

### `builder.h / builder.c`
- Construit les trames à envoyer
- Calcule le CRC automatiquement
- Fonctions : `build_output()`, `build_stats()`, `build_alarm()`

```c
// Utilisation :
uint8_t buf[32];
int len = build_output(buf, 187.5f);
uart_write_bytes(UART_NUM_0, buf, len);
```

### `pid.h / pid.c`
- Algorithme PID discret pur
- Pas de FreeRTOS, pas d'UART — juste les maths
- Sortie saturée entre 0 et 255
- Inclut l'anti-windup

```c
// Utilisation :
pid_t mon_pid = pid_init(2.0f, 0.8f, 0.02f);
float commande = pid_compute(&mon_pid, setpoint, speed, 0.1f);
```

### `shared_state.h / shared_state.c`
- Variables globales protégées par mutex
- Getters/setters thread-safe
- Contient : vitesse, consigne, mode, compteurs stats

```c
// Utilisation :
state_set_speed(90.5f);     // depuis task_rx
float v = state_get_speed(); // depuis task_pid
```

### `task_rx.c`
- Tâche FreeRTOS haute priorité
- Lit l'UART en permanence octet par octet
- Appelle `parser_push_byte()` à chaque octet
- Quand trame valide → met à jour `shared_state`
- Incrémente les compteurs d'erreurs si trame invalide

### `task_pid.c`
- Tâche FreeRTOS haute priorité
- S'exécute **exactement toutes les 100ms** (`xTaskDelayUntil`)
- Lit `shared_state` (speed + setpoint)
- Calcule la commande via `pid_compute()`
- Envoie un message OUTPUT via `builder`

### `task_telemetry.c`
- Tâche FreeRTOS basse priorité
- S'exécute toutes les **1 seconde**
- Lit les compteurs dans `shared_state`
- Envoie un message STATS

### `task_failsafe.c`
- Surveille deux événements :
  - Timeout 2s sans trame valide
  - Front sur GPIO (bouton d'urgence)
- Si déclenchement :
  - Commande moteur = 0
  - Mode = OFF
  - Envoie ALARM immédiatement

### `main.c`
- Initialise l'UART, les GPIO
- Crée les queues et mutex FreeRTOS
- Lance les 4 tâches
- Ne fait rien d'autre

---

## Les tâches FreeRTOS

| Tâche | Priorité | Période | Rôle |
|---|---|---|---|
| task_rx | 4 (haute) | continue | Lire l'UART |
| task_pid | 4 (haute) | 100ms strict | Calculer commande |
| task_failsafe | 5 (max) | événementiel | Sécurité |
| task_telemetry | 2 (basse) | 1s | Envoyer stats |

### Outils FreeRTOS utilisés
- **xTaskCreate** — créer une tâche
- **xTaskDelayUntil** — période stricte (pas vTaskDelay !)
- **xQueueCreate/Send/Receive** — communication entre tâches
- **xSemaphoreCreateMutex** — protéger shared_state
- **gpio_isr_handler_add** — interruption GPIO pour failsafe

---

## L'état partagé — shared_state

```c
// Variables principales :
float    g_current_speed;   // mis à jour par task_rx
float    g_setpoint;        // mis à jour par task_rx
uint8_t  g_mode;            // OFF / MANUAL / AUTO
uint32_t g_last_valid_rx;   // timestamp dernière trame valide (failsafe)

// Compteurs télémétrie :
uint32_t g_stats_valid;     // trames valides reçues
uint32_t g_stats_corrupted; // trames CRC invalide
uint32_t g_stats_dropped;   // trames perdues (queue pleine)
uint32_t g_stats_output;    // commandes OUTPUT envoyées
```

---

## Le flux complet

```
[UART]
  ↓ octets bruts
[task_rx]
  ↓ appelle parser_push_byte() octet par octet
[parser]
  ↓ trame complète + CRC ok
[shared_state]  ←── mise à jour speed / setpoint / mode
  ↑
[task_pid]  (toutes les 100ms)
  ↓ lit speed + setpoint
[pid_compute()]
  ↓ commande 0-255
[builder]
  ↓ construit trame OUTPUT
[UART]  ──▶  PC reçoit la commande

[task_telemetry]  (toutes les 1s)
  ↓ lit compteurs
[builder]
  ↓ construit trame STATS
[UART]  ──▶  PC reçoit les stats

[task_failsafe]  (événementiel)
  ↓ timeout ou GPIO
  → mode = OFF, commande = 0
[builder]
  ↓ construit trame ALARM
[UART]  ──▶  PC reçoit l'alerte
```

---

## Les enums définis dans parser.h

```c
typedef enum {
    MSG_SETPOINT  = 0x01,
    MSG_SPEED     = 0x02,
    MSG_MODE_SET  = 0x05,
    MSG_OUTPUT    = 0x80,
    MSG_STATS     = 0x83,
    MSG_ALARM     = 0x85,
    MSG_DBG       = 0xFF,
} msg_type_t;

typedef enum {
    MODE_OFF    = 0x00,
    MODE_MANUAL = 0x01,
    MODE_AUTO   = 0x02,
} ecu_mode_t;

typedef struct {
    uint8_t type;
    uint8_t payload[64];
    uint8_t len;
} frame_t;
```

---

## Points importants à retenir

- **`xTaskDelayUntil`** et pas `vTaskDelay` pour le PID → période absolue
- **Mutex obligatoire** sur shared_state → deux tâches peuvent écrire en même temps
- **Mutex obligatoire** sur l'UART TX → éviter l'entrelacement de trames
- **Parser = machine à états** → survive aux trames fragmentées et corrompues
- **Failsafe via ISR GPIO** → temps de réponse < 5ms garanti
- **Queue pleine = drop** → jamais bloquant pour task_rx
