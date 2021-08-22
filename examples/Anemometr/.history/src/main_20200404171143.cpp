volatile bool p = false;

// globalni promenna s poctem pulzu
int pulzy = 0;

void preruseni(){
  // Pokud doslo k preruseni, nastav pulz na true
 p = true;
}

// Hlavni funkce, ktera se spusti po startu
void setup() {
  // Nastartuj seriovou linku co nejvyssi rychlosti,
  // aby seriova linka neblokovala dlouho ridici cip
  Serial.begin(115200);
  // Anemometr je pripojeny na pin 21 (Arduino Mega)
  pinMode(21, INPUT);
  // Nastav interrupt: Pokud se bude menit stav z HIGH na LOW,
  // spust funkci preruseni
  attachInterrupt(digitalPinToInterrupt(21), preruseni, FALLING);
}

// Smycka loop, ktera se opakuje stale dokola
void loop() {
  // Pokud eviduji preruseni
  if(p){
    // Pozastav detekci preruseni
    cli();
    // Navys pocet pulzu
    pulzy++;
    // Resetuj informaci o preruseni
    p = false;
    // Vypis aktualni pocet pulzu do seriove linky
    Serial.print(c);
    Serial.println(" preruseni");
    // Opet spust detekci preruseni
    sei();
  }
} Více na: https://www.zive.cz/clanky/pojdme-programovat-elektroniku-postavime-si-vetromer--redakcni-tornadomer/sc-3-a-196075/default.aspx