#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// PINOS
const int PINO_BOTAO = 2;
const int PINO_LED = 13;
const int PINO_BUZZER = 8;
const int PINO_RELE = 7;

// CONFIGURAÇÕES MORSE
const unsigned long TEMPO_PONTO_TRACO = 300;
const unsigned long TEMPO_FIM_LETRA = 800;
const unsigned long TEMPO_RELE = 6000;

// PALAVRAS - FÁCIL
String palavrasFacil[] = {
  "CASA", "SOL", "LUA", "GATO", "BOLA",
  "BEM", "OI", "ANA", "FLOR", "RUA",
  "LUZ", "RIO", "SOS", "DADO", "FOGO"
};

// PALAVRAS - MÉDIO
String palavrasMedio[] = {
  "ESCOLA", "DANIEL", "JANELA", "LUIZA", "BEATRIZ",
  "RAFAEL", "CADERNO", "PEDRO", "BICICLETA", "MONTANHA",
  "LUCAS", "MARIA", "GABRIEL", "FABRICIO", "DINOSSAURO"
};

// PALAVRAS - DIFÍCIL
// Todas com até 16 caracteres
String palavrasDificil[] = {
  "PREPARADO",
  "GUILHERME",
  "ESTUDIOSO",
  "PARALELEPIPEDO",
  "EXTRAORDINARIO",
  "RESPONSAVEL",
  "DESENVOLVER",
  "MICROCONTROLADOR",
  "TELECOMUNICACAO",
  "AUTOMACAO",
  "PROGRAMADOR",
  "ELETRONICA",
  "COMPUTADOR",
  "TECNOLOGIA",
  "INTELIGENCIA"
};

// VARIÁVEIS
String palavraSorteada = "";
String codigoAtual = "";
String resposta = "";

bool botaoPressionado = false;
unsigned long tempoInicioPressionado = 0;
unsigned long tempoSoltou = 0;

int dificuldade = 0;
bool usouAjuda = false;

enum EstadoJogo {
  ESCOLHER_DIFICULDADE,
  JOGANDO
};

EstadoJogo estado = ESCOLHER_DIFICULDADE;

void setup() {
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_RELE, OUTPUT);

  digitalWrite(PINO_LED, LOW);
  digitalWrite(PINO_BUZZER, LOW);
  digitalWrite(PINO_RELE, LOW);

  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(A0));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("COFRE MORSE");
  lcd.setCursor(0, 1);
  lcd.print("BY LC SISTEMAS");
  delay(3000);

  mostrarDificuldade();
}

void loop() {
  if (estado == ESCOLHER_DIFICULDADE) {
    escolherDificuldade();
  }

  if (estado == JOGANDO) {
    jogarMorse();
  }
}

// ==========================
// DIFICULDADE
// ==========================

void mostrarDificuldade() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Escolha nivel");
  lcd.setCursor(0, 1);
  lcd.print("1F 2M 3D toque");
}

void escolherDificuldade() {
  int toques = contarToques();

  if (toques >= 1 && toques <= 3) {
    dificuldade = toques;
    resposta = "";
    codigoAtual = "";
    usouAjuda = false;

    lcd.clear();
    lcd.setCursor(0, 0);

    if (dificuldade == 1) lcd.print("Nivel Facil");
    else if (dificuldade == 2) lcd.print("Nivel Medio");
    else lcd.print("Nivel Dificil");

    lcd.setCursor(0, 1);
    lcd.print("Sorteando...");
    delay(1500);

    sortearPalavra();
    estado = JOGANDO;
    mostrarJogo();
  }
}

int contarToques() {
  static int contador = 0;
  static unsigned long ultimoToque = 0;
  static bool botaoAnterior = HIGH;

  bool leitura = digitalRead(PINO_BOTAO);

  if (botaoAnterior == HIGH && leitura == LOW) {
    contador++;
    ultimoToque = millis();

    tone(PINO_BUZZER, 1000, 80);
    delay(200);
  }

  botaoAnterior = leitura;

  if (contador > 0 && millis() - ultimoToque > 1200) {
    int total = contador;
    contador = 0;
    return total;
  }

  return 0;
}

// ==========================
// SORTEIO
// ==========================

void sortearPalavra() {
  if (dificuldade == 1) {
    palavraSorteada = palavrasFacil[random(15)];
  } else if (dificuldade == 2) {
    palavraSorteada = palavrasMedio[random(15)];
  } else {
    palavraSorteada = palavrasDificil[random(15)];
  }
}

// ==========================
// TELAS
// ==========================

void mostrarJogo() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(palavraSorteada);

  lcd.setCursor(0, 1);
  if (resposta.length() <= 16) {
    lcd.print(resposta);
  } else {
    lcd.print(resposta.substring(resposta.length() - 16));
  }
}

void mostrarMorseAtual() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(palavraSorteada);

  lcd.setCursor(0, 1);
  lcd.print(codigoAtual);
}

// ==========================
// JOGO MORSE
// ==========================

void jogarMorse() {
  bool estadoBotao = digitalRead(PINO_BOTAO) == LOW;

  if (estadoBotao && !botaoPressionado) {
    botaoPressionado = true;
    tempoInicioPressionado = millis();

    digitalWrite(PINO_LED, HIGH);
    tone(PINO_BUZZER, 800);
  }

  if (!estadoBotao && botaoPressionado) {
    botaoPressionado = false;

    unsigned long duracao = millis() - tempoInicioPressionado;

    digitalWrite(PINO_LED, LOW);
    noTone(PINO_BUZZER);

    if (duracao < TEMPO_PONTO_TRACO) {
      codigoAtual += ".";
    } else {
      codigoAtual += "-";
    }

    tempoSoltou = millis();
    mostrarMorseAtual();
  }

  if (!botaoPressionado && codigoAtual.length() > 0) {
    unsigned long pausa = millis() - tempoSoltou;

    if (pausa > TEMPO_FIM_LETRA) {
      char letra = decodificarMorse(codigoAtual);

      resposta += letra;
      codigoAtual = "";

      mostrarJogo();
      verificarResposta();
    }
  }
}

// ==========================
// VERIFICAÇÃO
// ==========================

void verificarResposta() {
  if (!palavraSorteada.startsWith(resposta)) {
    usouAjuda = true;

    int posicaoErro = resposta.length() - 1;
    char letraCorreta = palavraSorteada[posicaoErro];

    // Remove a letra errada para permitir tentar novamente
    resposta.remove(resposta.length() - 1);

    ajudaLetra(letraCorreta);

    mostrarJogo();
    return;
  }

  if (resposta == palavraSorteada) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACERTOU!");

    somSucesso();

    if ((dificuldade == 2 || dificuldade == 3) && !usouAjuda) {
      lcd.setCursor(0, 1);
      lcd.print("PORTA ABERTA");

      digitalWrite(PINO_RELE, HIGH);
      delay(TEMPO_RELE);
      digitalWrite(PINO_RELE, LOW);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("SEM ABRIR");
      delay(3000);
    }

    reiniciarJogo();
  }
}

// ==========================
// AJUDA
// ==========================

void ajudaLetra(char letra) {
  String morse = morseDaLetra(letra);

  somErro();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Letra correta:");
  lcd.print(letra);

  lcd.setCursor(0, 1);
  lcd.print("Morse: ");
  lcd.print(morse);
  delay(2500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Veja no botao");
  lcd.setCursor(0, 1);
  lcd.print(letra);
  lcd.print(" = ");
  lcd.print(morse);
  delay(1500);

  tocarMorse(morse);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tente de novo");
  lcd.setCursor(0, 1);
  lcd.print("mesma letra");
  delay(2000);
}

void tocarMorse(String morse) {
  for (int i = 0; i < morse.length(); i++) {
    digitalWrite(PINO_LED, HIGH);

    if (morse[i] == '.') {
      tone(PINO_BUZZER, 900);
      delay(200);
    } else {
      tone(PINO_BUZZER, 900);
      delay(600);
    }

    noTone(PINO_BUZZER);
    digitalWrite(PINO_LED, LOW);
    delay(300);
  }
}

// ==========================
// REINICIAR
// ==========================

void reiniciarJogo() {
  resposta = "";
  codigoAtual = "";
  dificuldade = 0;
  usouAjuda = false;
  estado = ESCOLHER_DIFICULDADE;

  delay(1000);
  mostrarDificuldade();
}

// ==========================
// SONS
// ==========================

void somErro() {
  for (int i = 0; i < 3; i++) {
    tone(PINO_BUZZER, 200, 200);
    delay(250);
  }
  noTone(PINO_BUZZER);
}

void somSucesso() {
  tone(PINO_BUZZER, 1200, 200);
  delay(250);
  tone(PINO_BUZZER, 1500, 200);
  delay(250);
  tone(PINO_BUZZER, 1800, 300);
  delay(350);
  noTone(PINO_BUZZER);
}

// ==========================
// DECODIFICADOR MORSE
// ==========================

char decodificarMorse(String m) {
  if (m == ".-") return 'A';
  if (m == "-...") return 'B';
  if (m == "-.-.") return 'C';
  if (m == "-..") return 'D';
  if (m == ".") return 'E';
  if (m == "..-.") return 'F';
  if (m == "--.") return 'G';
  if (m == "....") return 'H';
  if (m == "..") return 'I';
  if (m == ".---") return 'J';
  if (m == "-.-") return 'K';
  if (m == ".-..") return 'L';
  if (m == "--") return 'M';
  if (m == "-.") return 'N';
  if (m == "---") return 'O';
  if (m == ".--.") return 'P';
  if (m == "--.-") return 'Q';
  if (m == ".-.") return 'R';
  if (m == "...") return 'S';
  if (m == "-") return 'T';
  if (m == "..-") return 'U';
  if (m == "...-") return 'V';
  if (m == ".--") return 'W';
  if (m == "-..-") return 'X';
  if (m == "-.--") return 'Y';
  if (m == "--..") return 'Z';

  return '?';
}

// ==========================
// MORSE DA LETRA
// ==========================

String morseDaLetra(char letra) {
  if (letra == 'A') return ".-";
  if (letra == 'B') return "-...";
  if (letra == 'C') return "-.-.";
  if (letra == 'D') return "-..";
  if (letra == 'E') return ".";
  if (letra == 'F') return "..-.";
  if (letra == 'G') return "--.";
  if (letra == 'H') return "....";
  if (letra == 'I') return "..";
  if (letra == 'J') return ".---";
  if (letra == 'K') return "-.-";
  if (letra == 'L') return ".-..";
  if (letra == 'M') return "--";
  if (letra == 'N') return "-.";
  if (letra == 'O') return "---";
  if (letra == 'P') return ".--.";
  if (letra == 'Q') return "--.-";
  if (letra == 'R') return ".-.";
  if (letra == 'S') return "...";
  if (letra == 'T') return "-";
  if (letra == 'U') return "..-";
  if (letra == 'V') return "...-";
  if (letra == 'W') return ".--";
  if (letra == 'X') return "-..-";
  if (letra == 'Y') return "-.--";
  if (letra == 'Z') return "--..";

  return "?";
}
