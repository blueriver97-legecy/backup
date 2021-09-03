import RPi.GPIO as GPIO
import sys

ledPin = int(sys.argv[1])

if __name__ == "__main__":
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(ledPin, GPIO.OUT)
    print("ledPin %d, mode = %s" %(ledPin, sys.argv[2]))

    if(sys.argv[2] == 'on' or sys.argv[2] == 'ON'):
        GPIO.output(ledPin, GPIO.HIGH)
    elif(sys.argv[2] == 'off' or sys.argv[2] == 'OFF'):
        GPIO.output(ledPin, GPIO.LOW)
    elif(sys.argv[2] == 'clear' or sys.argv[2] == 'CLEAR'):
        GPIO.cleanup()


