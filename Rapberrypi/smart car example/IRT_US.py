import wiringpi
import threading


# 상수 초기화
HIGH    = 1
LOW     = 0
INPUT   = 0
OUTPUT  = 1
USING_US = True

# 모터 연결 핀 번혼
IN1_PIN = 1
IN2_PIN = 4
IN3_PIN = 5
IN4_PIN = 6

# 적외선 연결 핀 번혼
LEFT_TRACER_PIN     = 10
RIGHT_TRACER_PIN    = 11

# 초음파 연결 핀 번호
TRIG_PIN	= 28
ECHO_PIN	= 29

# 모터 속도 상수
MAX_SPEED   = 50
MIN_SPEED   = 0
AVG_SPEED   = 15
AVG_SPEED2  = 20

# 모터 동작 값 상수
INIT_VALUE  = (MAX_SPEED, MAX_SPEED, MAX_SPEED, MAX_SPEED, 'INIT')
GO_VALUE    = (AVG_SPEED, MIN_SPEED, AVG_SPEED, MIN_SPEED, 'GO')
BACK_VALUE  = (MIN_SPEED, AVG_SPEED, MIN_SPEED, AVG_SPEED, 'BACK')
LEFT_VALUE  = (MIN_SPEED, AVG_SPEED2, AVG_SPEED2, MIN_SPEED, 'LEFT')
RIGHT_VALUE = (AVG_SPEED2, MIN_SPEED, MIN_SPEED, AVG_SPEED2, 'RIGHT')
STOP_VALUE  = (MIN_SPEED, MIN_SPEED, MIN_SPEED, MIN_SPEED, 'STOP')

# 회전 시간
TURN_TIME   = 2000

# 거리 제한 상수
LIMIT_DISTANCE	= 10

# 리스트 최대 길이 상수
MAX_LEN = 10

state_L = [ i*0 for i in range(MAX_LEN) ]
state_R = [ i*0 for i in range(MAX_LEN) ]

# 동기화
lock = threading.Lock()

def controlMotor(value):

    wiringpi.softPwmWrite(IN1_PIN, value[0])
    wiringpi.softPwmWrite(IN2_PIN, value[1])
    wiringpi.softPwmWrite(IN3_PIN, value[2])
    wiringpi.softPwmWrite(IN4_PIN, value[3])
    print('control - %s' % value[4])
    wiringpi.delay(30)

def init():

    # 적외선 트레이서 초기화
    wiringpi.pinMode(LEFT_TRACER_PIN, INPUT)
    wiringpi.pinMode(RIGHT_TRACER_PIN, INPUT)

    wiringpi.pinMode(TRIG_PIN, OUTPUT)
    wiringpi.pinMode(ECHO_PIN, INPUT)

    # DC모터 초기화
    initMotor()
    wiringpi.delay(1000)


def initMotor():

    wiringpi.pinMode(IN1_PIN, OUTPUT)
    wiringpi.pinMode(IN2_PIN, OUTPUT)
    wiringpi.pinMode(IN3_PIN, OUTPUT)
    wiringpi.pinMode(IN4_PIN, OUTPUT)

    wiringpi.softPwmCreate(IN1_PIN, MIN_SPEED, MAX_SPEED)
    wiringpi.softPwmCreate(IN2_PIN, MIN_SPEED, MAX_SPEED)
    wiringpi.softPwmCreate(IN3_PIN, MIN_SPEED, MAX_SPEED)
    wiringpi.softPwmCreate(IN4_PIN, MIN_SPEED, MAX_SPEED)

def getDistance():

    wiringpi.digitalWrite(TRIG_PIN, LOW)
    wiringpi.delay(50)
    wiringpi.digitalWrite(TRIG_PIN, HIGH)
    wiringpi.delayMicroseconds(10)
    wiringpi.digitalWrite(TRIG_PIN, LOW)

    while (wiringpi.digitalRead(ECHO_PIN) == 0) : 
        pass
    start_time = wiringpi.micros()

    while (wiringpi.digitalRead(ECHO_PIN) == 1) :
        pass
    end_time = wiringpi.micros()

    distance = (end_time - start_time) / 29.0 / 2.0

    return distance

def checkUltraSonic(tm):
    while USING_US:
        if (getDistance() <= LIMIT_DISTANCE) :
            lock.acquire()
            print('Turn Around')
            controlMotor(STOP_VALUE)
            controlMotor(BACK_VALUE)
            controlMotor(LEFT_VALUE)
            wiringpi.delay(tm)
            controlMotor(STOP_VALUE)
            controlMotor(GO_VALUE)
            lock.release()

def checkControl(idx):

    leftSum = 0
    rightSum = 0

    nLValue = wiringpi.digitalRead(LEFT_TRACER_PIN)
    nRValue = wiringpi.digitalRead(RIGHT_TRACER_PIN)
    #print('LTracer - %d, RTracer - %d' % (nLValue, nRValue))
    lock.acquire()
    if nLValue == HIGH and nRValue == HIGH:
        #print('ALL detect')
        controlMotor(GO_VALUE)
        state_L[idx] = 1
        state_R[idx] = 1
    elif nLValue == HIGH:
        #print('LEFT_detect')
        controlMotor(LEFT_VALUE)
        state_L[idx] = 1
        state_R[idx] = 0
    elif nRValue == HIGH:
        #print('RIGHT_detect')
        controlMotor(RIGHT_VALUE)
        state_L[idx] = 0
        state_R[idx] = 1
    else:
        state_L[idx] = 0
        state_R[idx] = 0

        leftSum = sum(state_L)
        rightSum = sum(state_R)

        if leftSum > rightSum:
            controlMotor(LEFT_VALUE)
        elif rightSum > leftSum:
            controlMotor(RIGHT_VALUE)
        else:
            controlMotor(GO_VALUE)
    lock.release()

if __name__ == '__main__':

    idx = 0
    try:
        wiringpi.wiringPiSetup()
        init()
        t1 = threading.Thread(target=checkUltraSonic, args=(TURN_TIME,))
        t1.start()
        while True:           
            checkControl(idx)
            idx = (++idx) % MAX_LEN

    except KeyboardInterrupt:
        USING_US = False
        t1.join()
        controlMotor(STOP_VALUE)
