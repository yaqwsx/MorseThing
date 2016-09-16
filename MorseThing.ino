#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <toneAC.h>
#include <string.h>

const int IN_BUT = 3;
const int IN_BUT_GND = 5;
const int DEL_BUT = 2;
const int DEL_BUT_GND = 6;
const int ENTER_BUT = 4;
const int ENTER_BUT_GND = 7;

const int SHORT = 150;
const int LONG = 750;
const int TOLERANCE = 300;

const char* morse_letter[] =
{
  ".-"  , // A
  "-...", // B
  "-.-.", // C
  "-.." , // D
  "."   , // E
  "..-.", // F
  "--." , // G
  "....", // H
  ".."  , // I
  ".---", // J
  "-.-" , // K
  ".-..", // L
  "--"  , // M
  "-."  , // N
  "---" , // O
  ".--.", // P
  "--.-", // Q
  ".-." , // R
  "..." , // S
  "-"   , // T
  "..-" , // U
  "...-", // V
  ".--" , // W
  "-..-", // X
  "-.--", // Y
  "--.."  // Z
};

const char* morse_num[] =
{
  "-----", // 0
  ".----", // 1
  "..---", // 2
  "...--", // 3
  "....-", // 4
  ".....", // 5
  "-....", // 6
  "--...", // 7
  "---..", // 8
  "----."  // 9
};

struct MorseIn {
    MorseIn(LiquidCrystal_I2C& lcd) : _pos(0), _press_time(0), _last_state(false), _lcd(lcd) {
        clear_buff();
    }

    void update() {
        if (!digitalRead(IN_BUT) && !_last_state) {
            _press_time = millis();
            _cur_time = millis();
            _last_state = true;
        }
        else if (digitalRead(IN_BUT) && _last_state) {
            _last_state = false;
            // Parse character
            unsigned long t = millis() - _press_time;
            if (/*t > SHORT - TOLERANCE && t < SHORT + TOLERANCE*/ t < 300) {
                _buff[_pos++] = '.';
                _buff[_pos] = '_';
            }
            else /*if (t > LONG - TOLERANCE && t < LONG + TOLERANCE)*/ {
                _buff[_pos++] = '-';
                _buff[_pos] = '_';
            }

            if (_pos == 16) {
                clear_buff();
                (*result)('\0');
            }
        }

        // Delete
        if (!digitalRead(DEL_BUT)) {
            step_back();
            display();
            delay(500);
        }

        // Parse
        if (!digitalRead(ENTER_BUT)) {
            _buff[_pos] = 'X';
            Serial.println(_buff);
            char c = parse();
            clear_buff();
            display();
            (*result)(c);
            delay(500);
            return;
        }

        // Blink cursor
        if (millis() - _cur_time > 300) {
            _cur_time = millis();
            change_cur();
        }
        display();
    }

    void clear_buff() {
        for (int i = 0; i != 17; i++) {
            _buff[i] = ' ';
        }
        _buff[16] = 0;
        _buff[0] = '_';
        _pos = 0;
    }

    void step_back() {
        if (_pos != 0) {
            _buff[_pos] = ' ';
            _buff[--_pos] = '_';
        }
        else {
            (*del)();
        }
        display();
    }

    char parse() {
        char MORSE_TREE[] = "_TEMNAIOGKDWRUS_OQZYCXBJPALUFVH09_8___7_______61_______2___3_45__________";
        int pos = 0;
        for (int i = 0; i != 6; i++) {
            if (_buff[i] == '-')
                pos = 2 * pos + 1;
            else if (_buff[i] == '.')
                pos = 2 * pos + 2;
            else {
                if (MORSE_TREE[pos] == '_')
                    return 0;
                else
                    return MORSE_TREE[pos];
            }
        }
        return 0;
    }

    void change_cur() {
        if (_buff[_pos] == ' ')
            _buff[_pos] = '_';
        else
            _buff[_pos] = ' ';
    }

    void display() {
        _lcd.setCursor(0,1);
        _lcd.print(_buff);
    }

    void (*result)(char);
    void (*del)();
    char _buff[17];
    int _pos;
    unsigned long _press_time;
    unsigned long _cur_time;
    bool _last_state;

    LiquidCrystal_I2C& _lcd;
};

struct Entry {
    const char* key;
    const char* value;
    const char* morse;
};

struct Message {
    Message(LiquidCrystal_I2C& lcd) :
        _pos(0), _lcd(lcd) {}


    void display() {
        _lcd.setCursor(0,0);
        _lcd.print(_buff);
    }

    void push(char c) {
        if (_pos == 15) {
            toneAC(2000);
            delay(200);
            toneAC(500);
            delay(200);
            toneAC(0);
            toneAC(2000);
            delay(200);
            toneAC(500);
            delay(200);
            toneAC(0);
            clear();
            display();
            return;
        }
        toneAC(500);
        delay(100);
        toneAC(4000);
        delay(100);
        toneAC(0);
        _buff[_pos++] = c;
        display();
        test_match();
    }

    void del() {
        if (_pos != 0) {
            _buff[--_pos] = ' ';
        }
        display();
    }

    static bool equal(const char* a, const char* b) {
        while (*b != 0) {
            if (*a != *b) {
                return false;
            }
            a++; b++;
        }
        return *b == 0;
    }

    static void cpy(char* dst, const char* src) {
        while (*src) {
            *dst = *src;
            dst++;
            src++;
        }
    }

    void test_match() {
        Entry entries[] = {
            {"AHOJ", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"CAU", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"ZDRASTVUJ", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"ALOHA", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"CUS", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"ZDAR", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"ZDAREC", "SLYS A ODPOVEZ", "CO JE PRED TEBOU"},
            {"DUKATY", "DEBUG MSG", "FW V2.5 MORSE THING KSCUK"},
            {"DUKOVANY", "SPRAVNE", "KOLIK MA KRAVA NOHOU"},
            {"CTYRI", "SPRAVNE", "ORANZOVA NEBO ZELENA"},
            {"CTYRY", "SPRAVNE", "ORANZOVA NEBO ZELENA"},
            {"4", "SPRAVNE", "ORANZOVA NEBO ZELENA"},
            {"ORANZOVA", "", "VRAT SE POKRACUJ PO CESTE DOPRAVA"},
            {"ZELENA", "", "VRAT SE POKRACUJ PO CESTE DOPRAVA"},
            {"SOS", "POMOC NENI NA CESTE", ""},
            {"42", "MATE RUCNIK?", ""},
            {"IDDQD", "CHEAT DISABLED", ""}
        };
        int count = sizeof(entries) / sizeof(Entry);

        for (int i = 0; i != count; i++) {
            if (equal(_buff, entries[i].key)) {
                show_match(entries[i].value, entries[i].morse);
                return;
            }
        }
    }

    void show_match(const char* m, const char* b) {
        for(int j = 0; j != 10; j++) {
            toneAC(500 * j);
            delay(100);
        }
        toneAC();
        clear();
        cpy(_buff, m);
        display();
        delay(500);
        beep_message(b);

        while(digitalRead(ENTER_BUT) &&
              digitalRead(DEL_BUT) &&
              digitalRead(IN_BUT));

        clear();
        display();
    }

    void clear() {
        for(int i = 0; i != 17; i++)
            _buff[i] = ' ';
        _buff[16] = 0;
        _pos = 0;
    }

    void beep_message(const char* s) {
        while(*s) {
            if (*s == ' ') {
               delay(2500);
               s++;
               continue;
            }
            beep_letter(*s);
            s++;
        }
        for (int i = 5; i != 0; i--) {
          toneAC(i * 1000, 10, 100);
        }
    }

    void beep_letter(char c) {
        const char* s;
        if (c >= 'A' && c <= 'Z')
            s = morse_letter[c - 'A'];
        else
            s = morse_num[c - '0'];
        while(*s) {
            toneAC(2000);
            if (*s == '.')
                delay(300);
            else
                delay(800);
            toneAC();
            delay(700);
            s++;
        }
        delay(1500);
    }

    char _buff[17];
    int _pos;
    LiquidCrystal_I2C& _lcd;
};

LiquidCrystal_I2C lcd(0x27,20,4);
MorseIn morse(lcd);
Message message(lcd);

void handle(char c) {
    if (c == 0) {
        toneAC(2000);
        delay(200);
        toneAC(500);
        delay(200);
        toneAC(0);
    }
    else {
        message.push(c);
    }
}

void del() {
    message.del();
}

void setup() {
    Serial.begin(9600);
    Serial.println("Started");

    pinMode(IN_BUT, INPUT_PULLUP);
    pinMode(IN_BUT_GND, OUTPUT);
    pinMode(DEL_BUT, INPUT_PULLUP);
    pinMode(DEL_BUT_GND, OUTPUT);
    pinMode(ENTER_BUT, INPUT_PULLUP);
    pinMode(ENTER_BUT_GND, OUTPUT);

    lcd.init();
    lcd.backlight();

    morse.result = handle;
    morse.del = del;
}

void loop() {
    morse.update();
    delay(50);
}
