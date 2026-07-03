#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Troque para 0x3F se o LCD não aparecer
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================
// PINOS
// ==========================
const int PINO_BOTAO = 2;
const int PINO_LED = 13;
const int PINO_BUZZER = 8;
const int PINO_RELE = 7;

// ==========================
// CONFIGURAÇÕES MORSE
// ==========================
const unsigned long TEMPO_PONTO_TRACO = 300;   // menor que isso = ponto, maior = traço
const unsigned long TEMPO_FIM_LETRA = 800;     // pausa para finalizar letra
const unsigned long TEMPO_RELE = 6000;         // tempo do relé ligado

// ==========================
// PALAVRAS - FÁCIL (15)
// ==========================
String palavrasFacil[] = {
  "CASA",
  "SOL",
  "LUA",
  "GATO",
  "BOLA",
  "BEM",
  "OI",
  "ANA",
  "FLOR",
  "RUA",
  "LUZ",
  "RIO",
  "SOS",
  "DADO",
  "FOGO"
};

// ==========================
// PALAVRAS - MÉDIO (15)
// ==========================
String palavrasMedio[] = {
  "ESCOLA",
  "DANIEL",
  "JANELA",
  "LUIZA",
  "BEATRIZ",
  "RAFAEL",
  "CADERNO",
  "PEDRO",
  "BICICLETA",
  "MONTANHA",
  "LUCAS",
  "MARIA",
  "GABRIEL",
  "FABRICIO",
  "DINOSSAURO"
};

// ==========================
// PALAVRAS - DIFÍCIL (15)
// ==========================
String palavrasDificil[] = {
  "PREPARADO",
  "GUILHERME",
  "ESTUDIOSO",
  "PARALELEPIPEDO",
  "EXTRAORDINARIO",
  "RESPONSABILIDADE",
  "DESENVOLVIMENTO",
  "ELETRODOMESTICO",
  "OTORRINOLARINGOLOGISTA",
  "INCONSTITUCIONALIDADE",
  "INTERNACIONALIZACAO",
  "MICROCONTROLADOR",
  "TELECOMUNICACAO",
  "MULTIPROCESSAMENTO",
  "AUTOMATICAMENTE"
};

// ==========================
// VARIÁVEIS DO JOGO
// ==========================
String palavraSorteada = "";
String codigoAtual = "";
String resposta = "";

bool botaoPressionado = false;
unsigned long tempoInicioPressionado = 0;
unsigned long tempoSoltou = 0;

int dificuldade = 0;

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
// TELA DE DIFICULDADE
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

    lcd.clear();
    lcd.setCursor(0, 0);

    if (dificuldade == 1) {
      lcd.print("Nivel Facil");
    } else if (dificuldade == 2) {
      lcd.print("Nivel Medio");
    } else {
      lcd.print("Nivel Dificil");
    }

    lcd.setCursor(0, 1);
    lcd.print("Sorteando...");
    delay(1500);

    sortearPalavra();

    estado = JOGANDO;
    mostrarJogo();
  }
}

// Conta 1, 2 ou 3 toques para escolher a dificuldade
int contarToques() {
  static int contador = 0;
  static unsigned long ultimoToque = 0;
  static bool botaoAnterior = HIGH;

  bool leitura = digitalRead(PINO_BOTAO);

  if (botaoAnterior == HIGH && leitura == LOW) {
    contador++;
    ultimoToque = millis();

    tone(PINO_BUZZER, 1000, 80);
    delay(200); // debounce simples
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
// TELA DO JOGO
// ==========================
void mostrarJogo() {
  lcd.clear();

  lcd.setCursor(0, 0);
  if (palavraSorteada.length() <= 16) {
    lcd.print(palavraSorteada);
  } else {
    lcd.print(palavraSorteada.substring(0, 16));
  }

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
  if (palavraSorteada.length() <= 16) {
    lcd.print(palavraSorteada);
  } else {
    lcd.print(palavraSorteada.substring(0, 16));
  }

  lcd.setCursor(0, 1);
  lcd.print(codigoAtual);
}

// ==========================
// LÓGICA MORSE
// ==========================
void jogarMorse() {
  bool estadoBotao = digitalRead(PINO_BOTAO) == LOW;

  // Botão apertado
  if (estadoBotao && !botaoPressionado) {
    botaoPressionado = true;
    tempoInicioPressionado = millis();

    digitalWrite(PINO_LED, HIGH);
    tone(PINO_BUZZER, 800);
  }

  // Botão solto
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

  // Finaliza uma letra após pausa
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
  // Se a resposta atual não for o começo correto da palavra,
  // já considera erro imediatamente.
  if (!palavraSorteada.startsWith(resposta)) {
    somErro();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROU!");
    lcd.setCursor(0, 1);
    lcd.print("Nova palavra");
    delay(1500);

    sortearPalavra();
    resposta = "";
    codigoAtual = "";
    mostrarJogo();
    return;
  }

  // Se completou corretamente a palavra
  if (resposta == palavraSorteada) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACERTOU!");
    lcd.setCursor(0, 1);
    lcd.print("PORTA ABERTA");

    somSucesso();

    digitalWrite(PINO_RELE, HIGH);
    delay(TEMPO_RELE);
    digitalWrite(PINO_RELE, LOW);

    reiniciarJogo();
  }
}

void reiniciarJogo() {
  resposta = "";
  codigoAtual = "";
  dificuldade = 0;
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

  if (m == "-----") return '0';
  if (m == ".----") return '1';
  if (m == "..---") return '2';
  if (m == "...--") return '3';
  if (m == "....-") return '4';
  if (m == ".....") return '5';
  if (m == "-....") return '6';
  if (m == "--...") return '7';
  if (m == "---..") return '8';
  if (m == "----.") return '9';

  return '?';
}
