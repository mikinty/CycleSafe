import serial
import RPi.GPIO as GPIO
import time


Q_d = [0, 0, 0, 0, 0]
avg_d = 0
Q_v = [0, 0, 0, 0, 0]
avg_v = 0


ser=serial.Serial("/dev/ttyACM0", 9600)
ser.baudrate=9600

GPIO.setmode(GPIO.BCM)
LED_PIN = 21

GPIO.setup(LED_PIN, GPIO.OUT)



while True:
  msg = ser.readline()
  dist = int(msg)

  vel = dist - Q_d[-1]
  if dist < 5 or abs(vel) > 50:
    Q_d = [0, 0, 0, 0, 0]
    avg_d = 0
    Q_v = [0, 0, 0, 0, 0]
    avg_v = 0
    continue
  avg_v -= Q_v.pop(0)
  avg_v += vel
  Q_v.append(vel)
  avg_d -= Q_d.pop(0)
  avg_d += dist
  Q_d.append(dist)

  if (avg_v < 0 and
      avg_d + 10 * avg_v < 0):
    GPIO.output(LED_PIN, True)
    #print("ON")
  else: GPIO.output(LED_PIN, False)

