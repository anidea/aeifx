#define RELAY0  2
#define RELAY1  3

#define OUT0    4
#define OUT1    5
#define OUT2    6
#define OUT3    7
#define OUT4    8
#define OUT5    9

#define INPUT0  14
#define INPUT1  15
#define INPUT2  16
#define INPUT3  17
#define INPUT4  18
#define INPUT5  19

#define LED     LED_BUILTIN

#define HALL    6


int val;

void setup() {
  // put your setup code here, to run once:

  pinMode(RELAY0, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(OUT0, OUTPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(OUT5, OUTPUT);
  pinMode(LED, OUTPUT);
  

  pinMode(INPUT0, INPUT);
  pinMode(INPUT1, INPUT);
  pinMode(INPUT2, INPUT);
  pinMode(INPUT3, INPUT);
  pinMode(INPUT4, INPUT);
  pinMode(INPUT5, INPUT);

  Serial.begin(9600);          //  setup serial
  
}

void loop() {
  // put your main code here, to run repeatedly:
#if 0
  digitalWrite(RELAY1, HIGH);
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(RELAY1, LOW);
  digitalWrite(LED, LOW);
  delay(1000);

#endif

#if 1

  val = analogRead(HALL);    // read the input pin

  Serial.println(val);             // debug value

  
#endif

}
