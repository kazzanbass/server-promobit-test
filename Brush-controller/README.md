# Brush-controller
Функция `Turn off` выключает прибор.
```c++
void turn_off()
{
  Serial.println("Turned off");
}
```
Функция `Show angle` запускает функцию GetAngle(), которая возвращает в монитор порта угол поворота датчика.
```c++
void show_angle() //Функция для вывода значений с датчика угла                       
{
  Serial.println(GetAngle()); //Вывод значения в Serial Monitor
}
```
Функция `Set angle,num` (num - угол поворота) поворачивает прибор на заданный угол.
```c++
void set_angle(int x)
{
  Serial.println(x);
}
```
Функция `Set speed,num` (num - скорость) задает скорость.
```c++
void set_speed(int x)
{
  Serial.println(x);
}
```
Функция `comandvalue(ans)` определяет команду и выполняет её.
```c++
void comandvalue(String a)
{
  int comma = a.indexOf(','); //Нужен комменарий
  String command = a.substring(0, comma); //Нужен комментарий
  int value = a.substring(comma+1, a.length()).toInt(); //Нужен комментарий


  if (command == "Turn off"){
    turn_off();
  }
  if (command == "Show angle"){
      show_angle();
    } 
  
  if (command == "Set angle"){
    set_angle(value);
  }
  if (command == "Set speed"){
    set_speed(value);
  }
}
```
