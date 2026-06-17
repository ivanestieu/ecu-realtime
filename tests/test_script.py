import os
import random
import struct
import threading
import time
from datetime import datetime
import serial

# --- CONFIGURATION ---
SERIAL_PORT = '/dev/ttyUSB0' # À adapter
BAUD_RATE = 115200
TIMEOUT = 0.1
SPEED_TX_PERIOD_S = 0.1
MOTOR_GAIN = 1.65
DRAG_COEFF = 0.9
MODEL_RESPONSE = 1.2
MAX_SPEED = 220.0

# Codes des messages
MSG_SETPOINT = 0x01
MSG_SPEED = 0x02
MSG_MODE_SET = 0x05
MSG_OUTPUT = 0x80
MSG_STATS = 0x83
MSG_ALARM = 0x85

# --- LOG FILE SETUP ---
LOG_DIR = "esp32_logs"
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)
LOG_FILE = os.path.join(LOG_DIR, f"esp32_test_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log")

class FrameHandler:
    """Handles frame-level protocol: CRC computation, frame serialization, and parsing"""
    
    def __init__(self, serial_port):
        """
        Args:
            serial_port: Serial port object for communication
        """
        self.ser = serial_port
    
    def compute_crc(self, data_bytes):
        """Calcule le CRC par XOR de tous les octets sauf le START"""
        crc = 0
        for b in data_bytes:
            crc ^= b
        return crc
    
    def send_frame(self, msg_type, payload=b''):
        """Construit et envoie une trame au format: [0xAA][LEN][TYPE][PAYLOAD][CRC]"""
        length = len(payload) + 1  # LEN: Taille de (TYPE + PAYLOAD)
        header = struct.pack('<HB', length, msg_type)
        full_msg = header + payload
        crc = self.compute_crc(full_msg)
        frame = struct.pack('B', 0xAA) + full_msg + struct.pack('B', crc)
        self.ser.write(frame)
    
    def receive_frame(self):
        """
        Reads and parses a single frame from serial port.
        Returns: (msg_type, payload) tuple if valid frame received, None otherwise
        Also returns raw text bytes that don't belong to structured frames.
        Returns: (msg_type, payload, raw_text) or (None, None, raw_text) if no valid frame
        """
        raw_buffer = b''
        
        while True:
            if self.ser.in_waiting > 0:
                byte = self.ser.read(1)
                if not byte:
                    continue
                
                if byte == b'\xaa':
                    # Frame header found
                    if raw_buffer:
                        return (None, None, raw_buffer)
                    
                    # Parse structured message
                    len_bytes = self.ser.read(2)
                    if len(len_bytes) < 2:
                        continue
                    length = struct.unpack('<H', len_bytes)[0]
                    
                    data = self.ser.read(length)
                    crc_received = self.ser.read(1)
                    
                    if len(data) == length and crc_received:
                        if self.compute_crc(len_bytes + data) == ord(crc_received):
                            msg_type = data[0]
                            payload = data[1:]
                            return (msg_type, payload, b'')
                else:
                    raw_buffer += byte
                    if b'\n' in raw_buffer or len(raw_buffer) > 256:
                        return (None, None, raw_buffer)
            else:
                break
        
        return (None, None, raw_buffer)


class ECUTester:
    def __init__(self):
        self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)
        self.frame_handler = FrameHandler(self.ser)
        self.current_speed = 50.0 # Vitesse initiale
        self.target_setpoint = 90.0
        self.last_output = 0.0
        self.running = True
        self.stats_count = 0
        self.start_time = time.time()
        self.log_file = open(LOG_FILE, 'w', encoding='utf-8', buffering=1)  # Line-buffered

    def _log(self, message):
        """Write message to both console and log file with timestamp"""
        timestamp = datetime.now().strftime('%H:%M:%S.%f')
        formatted_msg = f"[{timestamp}] {message}"
        self.log_file.write(formatted_msg + '\n')
        self.log_file.flush()

    def update_vehicle_model(self, dt):
        """Modèle 1er ordre: accel = moteur - traînée."""
        dt = max(0.0, min(dt, 0.2))
        acceleration = MODEL_RESPONSE * ((MOTOR_GAIN * self.last_output) -
(DRAG_COEFF * self.current_speed))
        self.current_speed += acceleration * dt
        self.current_speed = max(0.0, min(MAX_SPEED, self.current_speed))

    def receive_feedback(self):
        """Lit et décode les messages venant de l'ECU (Output et Télémétrie)
        Capture également les logs texte ESP-IDF"""
        raw_buffer = b''

        while self.running:
            msg_type, payload, raw_text = self.frame_handler.receive_frame()
            
            # Handle raw text logs
            if raw_text:
                raw_buffer += raw_text
                if b'\n' in raw_buffer:
                    lines = raw_buffer.split(b'\n')
                    for line in lines[:-1]:
                        text = line.decode('utf-8', errors='ignore').strip()
                        if text:
                            self._log(f"[ESP32 LOG] {text}")
                    raw_buffer = lines[-1]
            
            # Handle structured frames
            if msg_type is not None:
                raw_buffer = b''
                
                if msg_type == MSG_OUTPUT:
                    self.last_output = struct.unpack('<f', payload)[0]
                    print(f"[SPEED] ({self.current_speed:.2f} km/h) | Commande: {self.last_output:.2f}")
                elif msg_type == MSG_STATS:
                    self.stats_count += 1
                    print(f"[TELEMETRIE] Reçue ({self.stats_count}s)")
                elif msg_type == MSG_ALARM:
                    print(f"\n[ALERTE ECU] : {payload.decode(errors='ignore')}")

    def run_normal_operation(self, duration):  
        """Phase de fonctionnement normal (100ms cycle) """
        print(f"--- DÉBUT PHASE NORMALE ({duration}s) ---")
        self.frame_handler.send_frame(MSG_SETPOINT, struct.pack('<f', self.target_setpoint))
        self.frame_handler.send_frame(MSG_MODE_SET, struct.pack('B', 2)) # Mode AUTO
    
        end_time = time.time() + duration
        last_tick = time.time()
        while time.time() < end_time:
            now = time.time()
            dt = now - last_tick
            last_tick = now

            self.update_vehicle_model(dt)
            # Envoi de la vitesse mesurée toutes les 100ms
            self.frame_handler.send_frame(MSG_SPEED, struct.pack('<f', self.current_speed))
            time.sleep(SPEED_TX_PERIOD_S)

    def run_stress_test(self):
        """Phase de stress : messages corrompus, fragmentés et flood"""
        print("\n--- DÉBUT PHASE DE STRESS ---")

        # 1. Flood de messages (Surcharge CPU)
        print("Action: Surcharge CPU (Flood)...")
        for _ in range(50):
            self.frame_handler.send_frame(random.randint(0, 0xFF), os.urandom(4))

        # 2. Trames fragmentées
        print("Action: Envoi de trame fragmentée...")
        partial_frame = struct.pack('B', 0xAA) + struct.pack('<HB', 5, MSG_SPEED)
        self.ser.write(partial_frame)
        time.sleep(0.5) # Pause au milieu du message
        self.ser.write(struct.pack('<f', 100.0) + b'\x00') # Fin de trame

        # 3. Erreur de CRC
        print("Action: Envoi erreur CRC...")
        self.ser.write(b'\xAA\x05\x00\x02\x00\x00\x00\x00\xFF')

    def test_failsafe(self):
        """Vérifie si l'ECU passe en Failsafe après 2s de silence [cite: 53]"""
        print("\n--- TEST FAILSAFE (Silence radio 2s) ---")
        time.sleep(2.5)
        if abs(self.last_output) < 1e-3:
            print("VÉRIFICATION RÉUSSIE : L'ECU a coupé la commande moteur (Output=0)")
        else:
            print("ÉCHEC : L'ECU n'est pas passé en mode Failsafe")

    def start(self):
        # Lancement du thread de lecture
        reader = threading.Thread(target=self.receive_feedback, daemon=True)
        reader.start()

        try:
            self.run_normal_operation(30) # 30s de fonctionnement normal
            self.run_stress_test()
            self.run_normal_operation(5)
            self.test_failsafe()
        except KeyboardInterrupt:
            pass
        finally:
            self.running = False
            self.log_file.close()
            self.ser.close()
            print("\nTest terminé.")

if __name__ == "__main__":
    tester = ECUTester()
    tester.start()
