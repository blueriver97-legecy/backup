import wiringpi

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

# 센서 연결 핀 번혼
LEFT_TRACER_PIN     = 10
RIGHT_TRACER_PIN    = 11

# 모터 속도 상수
MAX_SPEED   = 50
MIN_SPEED   = 0
AVG_SPEED   = 15
AVG_SPEED2  = 35

# 모터 동작 값 상수
INIT_VALUE  = (MAX_SPEED, MAX_SPEED, MAX_SPEED, MAX_SPEED, 'INIT')
GO_VALUE    = (AVG_SPEED, MIN_SPEED, AVG_SPEED, MIN_SPEED, 'GO')
BACK_VALUE  = (MIN_SPEED, AVG_SPEED, MIN_SPEED, AVG_SPEED, 'BACK')
LEFT_VALUE  = (MIN_SPEED, AVG_SPEED2, AVG_SPEED2, MIN_SPEED, 'LEFT')
RIGHT_VALUE = (AVG_SPEED2, MIN_SPEED, MIN_SPEED, AVG_SPEED2, 'RIGHT')
STOP_VALUE  = (MIN_SPEED, MIN_SPEED, MIN_SPEED, MIN_SPEED, 'STOP')

# 리스트 최대 길이 상수
MAX_LEN = 20

state_L = [ i*0 for i in range(MAX_LEN) ]
state_R = [ i*0 for i in range(MAX_LEN) ]

def controlMotor(value):

    wiringpi.softPwmWrite(IN1_PIN, value[0])
    wiringpi.softPwmWrite(IN2_PIN, value[1])
    wiringpi.softPwmWrite(IN3_PIN, value[2])
    wiringpi.softPwmWrite(IN4_PIN, value[3])
    print('control - %s\n' % value[4])
    wiringpi.delay(30)
    
def initSensor():
    
    # 적외선 트레이서 초기화
    wiringpi.pinMode(LEFT_TRACER_PIN, INPUT)
    wiringpi.pinMode(RIGHT_TRACER_PIN, INPUT)
    
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
    
    
def checkControl(idx):
    
    nLValue = wiringpi.digitalRead(LEFT_TRACER_PIN)
    nRValue = wiringpi.digitalRead(RIGHT_TRACER_PIN)
    leftSum = 0
    rightSum = 0
    
    print('LTracer - %d, RTracer - %d\n' % (nLValue, nRValue))
    
    if nLValue == HIGH and nRValue == HIGH:
        print('ALL detect\n')
        controlMotor(GO_VALUE)
        state_L[idx] = 1
        state_R[idx] = 1
    elif nLValue == HIGH:
        print('LEFT_detect\n')
        controlMotor(LEFT_VALUE)
        state_L[idx] = 1
        state_R[idx] = 0
    elif nRValue == HIGH:
        print('RIGHT_detect\n')
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

# 메인 함수
if __name__ == '__main__':
    
    idx = 0
    
    try:
        wiringpi.wiringPiSetup()
        initSensor()
        
        while True:           
            checkControl(idx)
            idx = (++idx) % MAX_LEN
 
    except KeyboardInterrupt:
        controlMotor(STOP_VALUE)
        # 모터 멈추도록 PWM 설정하는 부분 후 리턴
    
