#include <Arduino.h>
#include <AS5600.h> //Библиотека для датчика угла AS5600
#include <math.h>
#include <Wire.h>
#include <pinout.h>

#define PIDt 100
#define MIN_SPEED 75
#define PORT_SPEED 9600
#define MINOUT -150
#define MAXOUT 180
#define MAX 40
#define MIN -40
#define KP 15
#define KI 7
#define KD 10
#define PWM_FREQUENCY 1500

AS5600 as5600; // Класс AS5600, объект as5600, используется по умолчанию

void comandvalue(String);
void turn_off();
void show_angle();
void set_angle(int);
void set_speed(int);
float GetAngle();
bool SetupAngle();
void MotorControl(int);
void MotorSetup(void);
void PIDreg(void);

String ans = "";
int targetangle;
int timer = 0;

void setup()
{
    Serial.begin(PORT_SPEED);
    SetupAngle();
    MotorSetup();
}

void loop()
{   
    ans = Serial.readString(); // Считываем  строку с монитора порта
    Serial.println(ans);       // Выводим
    comandvalue(ans);          // Выполняем команду
    ans = "";

    if (millis() - timer >= PIDt)
    {
        timer = millis();
        PIDreg();
    }
}

void turn_off() // Пока что пустая функция
{
    digitalWrite(EN, LOW);
}
void show_angle() // Функция для вывода значений с датчика угла
{
    Serial.println(GetAngle()); // Вывод значения в Serial Monitor
}
void set_angle(int x) // Пока что пустая функция
{
    digitalWrite(EN, HIGH);
    Serial.println(x);
    if (x > 29.0)
    {
        x = 29.0;
    }
    else if (x < -20)
    {
        x = -20;
    }
    targetangle = x;
}
void set_speed(int x) // Пока что пустая функция
{
    if (x <= MAX && x >= MIN)
    {
        x = 0;
        MotorControl(x);
    }
    else
        MotorControl(x);
    Serial.println(x);
}

float GetAngle() // Функция, получающая значения с датчика угла
{
    float angledat, convdat;

    if (SetupAngle() == true)
    {                                              // Если датчик подключен, выполняется функция
        angledat = as5600.getCumulativePosition(); // Переменной angeldat присваивается значение с датчика угла с интервалом в 100 миллисекунд
        convdat = (angledat / 11.378) - 22.59;     // Переменная convdat получает конвертированное в градусы значение угла(1 градус = 11.378 бит)
    }
    else
    {
        Serial.println("StopMotor"); // Если нет, то выводится сообщение
    }

    return (convdat); // Функция возвращает положение угла в градусах, число вещественное(float)
}

bool SetupAngle() // Setup-функция для датчика угла (AS5600)
{
    bool b;

    as5600.begin();                         // Установка пина DIR, не изменять, нам оно не надо)
    as5600.setDirection(AS5600_CLOCK_WISE); // В документации к библиотеке написано, что это нужно прописывать по умолчанию
    b = as5600.isConnected();               // В переменную записывается результат проверки подключения к контроллеру тип переменной - bool

    Serial.print("Connect: ");
    Serial.println(b); // Вывод в Serial Monitor состояние подключения к датчику угла(1/0)

    return (b); // Возвращает значение True/False(1/0), в зависимости от того подключен ли контроллер или нет
}
void comandvalue(String a)
{
    int comma = a.indexOf(',');                             // Принимаем строку, разделяем на команду и число (если есть)
    String command = a.substring(0, comma);                 // Получаем команду
    int value = a.substring(comma + 1, a.length()).toInt(); // Получаем число (если есть)

    if (command == "Turn off")
    {
        turn_off();
    }
    if (command == "Show angle")
    {
        show_angle();
    }

    if (command == "Set angle")
    {
        set_angle(value);
    }
    if (command == "Set speed")
    {
        set_speed(value);
    }
}

void MotorSetup(void)
{
    pinMode(EN, OUTPUT);
    pinMode(L_PWM, OUTPUT);
    pinMode(R_PWM, OUTPUT);
}

void MotorControl(int x)
{
    Serial.println(x);
    if (x > 0)
    {
        analogWrite(R_PWM, x); // Движение против чассовой
        analogWrite(L_PWM, 0); // Движение против чассовой
    }
    else
    {
        if (x < 0)
        {
            analogWrite(L_PWM, -x); // Движение по часовой
            analogWrite(R_PWM, 0);
        }
        if (x == 0)
        {
            analogWrite(L_PWM, 0);
            analogWrite(R_PWM, 0);
        }
    }
}

void PIDreg(void)
{
    static float input, setpoint, dt; // input,setpoint - Входные значения, остальное - коэффиценты
    static int I = 0;
    float prevErr = 0;
    input = GetAngle();
    setpoint = targetangle;
    int P = setpoint - input;
    I = I + (setpoint - input) * PIDt;
    int err = setpoint - input;
    float D = (err - prevErr) / PIDt;
    prevErr = err;
    float integral = 0;
    integral = constrain(integral + (float)err * dt * KI, MINOUT, MAXOUT);
    set_speed(constrain(err * KP + integral + D * KD, MINOUT, MAXOUT));
}