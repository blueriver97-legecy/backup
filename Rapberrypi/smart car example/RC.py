import wiringpi
import os
import sys
import termios
import atexit
from select import select

# 상수 초기화
HIGH    = 1
LOW     = 0
INPUT   = 0
OUTPUT  = 1

# 모터 연결 핀 번혼
IN1_PIN = 1
IN2_PIN = 4
IN3_PIN = 5
IN4_PIN = 6

# 모터 속도 상수
MAX_SPEED   = 50
AVG_SPEED   = 30
MIN_SPEED   = 0

# 모터 동작 값 상수
INIT_VALUE  = (MAX_SPEED, MAX_SPEED, MAX_SPEED, MAX_SPEED, 'INIT')
GO_VALUE    = (AVG_SPEED, MIN_SPEED, AVG_SPEED, MIN_SPEED, 'GO')
BACK_VALUE  = (MIN_SPEED, AVG_SPEED, MIN_SPEED, AVG_SPEED, 'BACK')
LEFT_VALUE  = (MIN_SPEED, AVG_SPEED, AVG_SPEED, MIN_SPEED, 'LEFT')
RIGHT_VALUE = (AVG_SPEED, MIN_SPEED, MIN_SPEED, AVG_SPEED, 'RIGHT')
STOP_VALUE  = (MIN_SPEED, MIN_SPEED, MIN_SPEED, MIN_SPEED, 'STOP')

class KBHit:
    
    def __init__(self):
        '''Creates a KBHit object that you can call to do various keyboard things.
        '''
        # Save the terminal settings
        self.fd = sys.stdin.fileno()
        self.new_term = termios.tcgetattr(self.fd)
        self.old_term = termios.tcgetattr(self.fd)
    
        # New terminal setting unbuffered
        self.new_term[3] = (self.new_term[3] & ~termios.ICANON & ~termios.ECHO)
        termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.new_term)
    
        # Support normal-terminal reset at exit
        atexit.register(self.set_normal_term)
    
    def set_normal_term(self):
        ''' Resets to normal terminal.  On Windows this is a no-op.
        '''
        termios.tcsetattr(self.fd, termios.TCSAFLUSH, self.old_term)

    def getch(self):
        ''' Returns a keyboard character after kbhit() has been called.
            Should not be called in the same program as getarrow().
        '''
        s = ''
        return sys.stdin.read(1)
                        

    def getarrow(self):
        ''' Returns an arrow-key code after kbhit() has been called. Codes are
        0 : up
        1 : right
        2 : down
        3 : left
        Should not be called in the same program as getch().
        '''
        c = sys.stdin.read(3)[2]
        vals = [65, 67, 66, 68]
        return vals.index(ord(c.encode('utf-8')))
        

    def kbhit(self):
        ''' Returns True if keyboard character was hit, False otherwise.
        '''
        dr,dw,de = select([sys.stdin], [], [], 0)
        return dr != []



def controlMotor(value):
    wiringpi.softPwmWrite(IN1_PIN, value[0])
    wiringpi.softPwmWrite(IN2_PIN, value[1])
    wiringpi.softPwmWrite(IN3_PIN, value[2])
    wiringpi.softPwmWrite(IN4_PIN, value[3])
    print('control - %s\n' % value[4])
    wiringpi.delay(30)
    
    
def initMotor():
    wiringpi.pinMode(IN1_PIN, OUTPUT)
    wiringpi.pinMode(IN2_PIN, OUTPUT)
    wiringpi.pinMode(IN3_PIN, OUTPUT)
    wiringpi.pinMode(IN4_PIN, OUTPUT)
    
    wiringpi.softPwmCreate(IN1_PIN, MIN_SPEED, MAX_SPEED)
    wiringpi.softPwmCreate(IN2_PIN, MIN_SPEED, MAX_SPEED)
    wiringpi.softPwmCreate(IN3_PIN, MIN_SPEED, MAX_SPEED)
    wiringpi.softPwmCreate(IN4_PIN, MIN_SPEED, MAX_SPEED)
    
    wiringpi.delay(1000)
    
    # 메인 함수
if __name__ == '__main__':
    
    try:
        wiringpi.wiringPiSetup()
        kb = KBHit()
        initMotor()
        
        # PPT 21페이지 작성
        while True:
            if kb.kbhit():
                termios.tcflush(sys.stdin, termios.TCIFLUSH)
                c = kb.getarrow()
                if c == 0: 
                    controlMotor(GO_VALUE)
                    wiringpi.delay(100)
                    controlMotor(STOP_VALUE)
                elif c == 1:
                    controlMotor(RIGHT_VALUE)
                    wiringpi.delay(100)
                    controlMotor(STOP_VALUE)
                elif c == 2:
                    controlMotor(BACK_VALUE)
                    wiringpi.delay(100)
                    controlMotor(STOP_VALUE)
                elif c == 3:
                    controlMotor(LEFT_VALUE)
                    wiringpi.delay(100)
                    controlMotor(STOP_VALUE)
                else:
                    pass
        
        kb.set_normal_term()

    except KeyboardInterrupt:
        controlMotor(STOP_VALUE)
        kb.set_normal_term()
        # 모터 멈추도록 PWM 설정하는 부분 후 리턴
    
