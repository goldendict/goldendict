/* This file is (c) 2013 Maksim Tamkovicz <quendimax@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "belarusiantranslit.hh"
#include "transliteration.hh"
#include <QCoreApplication>

namespace BelarusianTranslit {

class BelarusianLatinToClassicTable: public Transliteration::Table
{
public:
  BelarusianLatinToClassicTable()
  {
    // Utf8ins()

    // latin to cyrillic
    ins( "a", "а" );
    ins( "b", "б" );
    ins( "c", "ц" );
    ins( "ć", "ць" );
    ins( "č", "ч" );
    ins( "cz", "ч" );
    ins( "d", "д" );
    ins( "e", "э" );
    ins( "f", "ф" );
    ins( "g", "ґ" );
    ins( "h", "г" );
    ins( "i", "і" );
    ins( "j", "й" );
    ins( "k", "к" );
    ins( "l", "ль" );
    ins( "ł", "л" );
    ins( "m", "м" );
    ins( "n", "н" );
    ins( "ń", "нь" );
    ins( "o", "о" );
    ins( "p", "п" );
    ins( "r", "р" );
    ins( "s", "с" );
    ins( "ś", "сь" );
    ins( "š", "ш" );
    ins( "sz", "ш" );
    ins( "t", "т" );
    ins( "u", "у" );
    ins( "ŭ", "ў" );
    ins( "v", "в" );
    ins( "w", "в" );
    ins( "y", "ы" );
    ins( "z", "з" );
    ins( "ź", "зь" );
    ins( "ž", "ж" );
    ins( "ż", "ж" );
    ins( "ch", "х" );

    ins( "ja", "я" );   ins( "je", "е" );   ins( "jo", "ё" );   ins( "ju", "ю" );
    ins( "la", "ля" );  ins( "le", "ле" );  ins( "lo", "лё" );  ins( "lu", "лю" );  ins( "li", "лі" );

    ins( "bia", "бя" );  ins( "bie", "бе" );  ins( "bio", "бё" );  ins( "biu", "бю" );
    ins( "cia", "ця" );  ins( "cie", "це" );  ins( "cio", "цё" );  ins( "ciu", "цю" );
    ins( "dzia","дзя" ); ins( "dzie","дзе" ); ins( "dzio","дзё" ); ins( "dziu","дзю" );
    ins( "fia", "фя" );  ins( "fie", "фе" );  ins( "fio", "фё" );  ins( "fiu", "фю" );
    ins( "gia", "ґя" );  ins( "gie", "ґе" );  ins( "gio", "ґё" );  ins( "giu", "ґю" );
    ins( "hia", "гя" );  ins( "hie", "ге" );  ins( "hio", "гё" );  ins( "hiu", "гю" );
    ins( "kia", "кя" );  ins( "kie", "ке" );  ins( "kio", "кё" );  ins( "kiu", "кю" );
    ins( "lia", "ліa" ); ins( "lie", "ліэ" ); ins( "lio", "ліо" ); ins( "liu", "ліу" );
    ins( "mia", "мя" );  ins( "mie", "ме" );  ins( "mio", "мё" );  ins( "miu", "мю" );
    ins( "nia", "ня" );  ins( "nie", "не" );  ins( "nio", "нё" );  ins( "niu", "ню" );
    ins( "pia", "пя" );  ins( "pie", "пе" );  ins( "pio", "пё" );  ins( "piu", "пю" );
    ins( "sia", "ся" );  ins( "sie", "се" );  ins( "sio", "сё" );  ins( "siu", "сю" );
    ins( "via", "вя" );  ins( "vie", "ве" );  ins( "vio", "вё" );  ins( "viu", "вю" );
    ins( "wia", "вя" );  ins( "wie", "ве" );  ins( "wio", "вё" );  ins( "wiu", "вю" );
    ins( "zia", "зя" );  ins( "zie", "зе" );  ins( "zio", "зё" );  ins( "ziu", "зю" );
    ins( "chia", "хя" ); ins( "chie", "хе" ); ins( "chio", "хё" ); ins( "chiu", "хю" );

    ins( "bja", "б'я" );  ins( "bje", "б'е" );  ins( "bjo", "б'ё" );  ins( "bju", "б'ю" );
    ins( "cia", "ц'я" );  ins( "cje", "ц'е" );  ins( "cjo", "ц'ё" );  ins( "cju", "ц'ю" );
    ins( "fja", "ф'я" );  ins( "fje", "ф'е" );  ins( "fjo", "ф'ё" );  ins( "fju", "ф'ю" );
    ins( "hja", "г'я" );  ins( "hje", "г'е" );  ins( "hjo", "г'ё" );  ins( "hju", "г'ю" );
    ins( "kja", "к'я" );  ins( "kje", "к'е" );  ins( "kjo", "к'ё" );  ins( "kju", "к'ю" );
    ins( "łja", "л'я" );  ins( "łje", "л'е" );  ins( "łjo", "л'ё" );  ins( "łju", "л'ю" );
    ins( "mja", "м'я" );  ins( "mje", "м'е" );  ins( "mjo", "м'ё" );  ins( "mju", "м'ю" );
    ins( "nja", "н'я" );  ins( "nje", "н'е" );  ins( "njo", "н'ё" );  ins( "nju", "н'ю" );
    ins( "pja", "п'я" );  ins( "pje", "п'е" );  ins( "pjo", "п'ё" );  ins( "pju", "п'ю" );
    ins( "sja", "с'я" );  ins( "sje", "с'е" );  ins( "sjo", "с'ё" );  ins( "sju", "с'ю" );
    ins( "vja", "в'я" );  ins( "vje", "в'е" );  ins( "vjo", "в'ё" );  ins( "vju", "в'ю" );
    ins( "wja", "в'я" );  ins( "wje", "в'е" );  ins( "wjo", "в'ё" );  ins( "wju", "в'ю" );
    ins( "zja", "з'я" );  ins( "zje", "з'е" );  ins( "zjo", "з'ё" );  ins( "zju", "з'ю" );
    ins( "chja", "х'я" ); ins( "chje", "х'е" ); ins( "chjo", "х'ё" ); ins( "chju", "х'ю" );

    ins( "nnia", "ньня" );  ins( "nnie", "ньне" );  ins( "nnio", "ньнё" );  ins( "nniu", "ньню" );ins( "nni", "ньні" );

    // cyrillic to latin
    ins( "а", "a" );   ins( "б", "b" );   ins( "в", "v" );   ins( "г", "h" );
    ins( "ґ", "g" );   ins( "д", "d" );   ins( "е", "je" );  ins( "ё", "jo" );
    ins( "ж", "ž" );   ins( "з", "z" );   ins( "і", "i" );   ins( "ї", "ï" );
    ins( "й", "j" );   ins( "к", "k" );   ins( "л", "ł" );   ins( "м", "m" );
    ins( "н", "n" );   ins( "о", "o" );   ins( "п", "p" );   ins( "р", "r" );
    ins( "с", "s" );   ins( "т", "t" );   ins( "у", "u" );   ins( "ў", "ŭ" );
    ins( "ф", "f" );   ins( "х", "ch" );  ins( "ц", "c" );   ins( "ч", "č" );
    ins( "ш", "š" );   ins( "ы", "y" );   ins( "э", "e" );   ins( "ю", "ju" );
    ins( "я", "ja" );  ins( "кг", "g" );
    ins( "бе", "bie" );  ins( "бё", "bio" );  ins( "бю", "biu" );  ins( "бя", "bia" );
    ins( "ве", "vie" );  ins( "вё", "vio" );  ins( "вю", "viu" );  ins( "вя", "via" );
    ins( "ге", "hie" );  ins( "гё", "hio" );  ins( "гю", "hiu" );  ins( "гя", "hia" );
    ins( "ґе", "gie" );  ins( "ґё", "gio" );  ins( "ґю", "giu" );  ins( "ґя", "gia" );
    ins( "кге", "gie" ); ins( "кгё", "gio" ); ins( "кгю", "giu" ); ins( "кгя", "gia" );
    ins( "зе", "zie" );  ins( "зё", "zio" );  ins( "зю", "ziu" );  ins( "зя", "zia" );  ins( "зь", "ź" );
    ins( "ке", "kie" );  ins( "кё", "kio" );  ins( "кю", "kiu" );  ins( "кя", "kia" );
    ins( "ле", "le" );   ins( "лё", "lo" );   ins( "лю", "lu" );   ins( "ля", "la" );   ins( "ль", "l" );   ins( "лі", "li" );
    ins( "ме", "mie" );  ins( "мё", "mio" );  ins( "мю", "miu" );  ins( "мя", "mia" );  ins( "мь", "m" );
    ins( "не", "nie" );  ins( "нё", "nio" );  ins( "ню", "niu" );  ins( "ня", "nia" );  ins( "нь", "ń" );
    ins( "пе", "pie" );  ins( "пё", "pio" );  ins( "пю", "piu" );  ins( "пя", "pia" );  ins( "пь", "p" );
    ins( "се", "sie" );  ins( "сё", "sio" );  ins( "сю", "siu" );  ins( "ся", "sia" );  ins( "сь", "ś" );
    ins( "фе", "fie" );  ins( "фё", "fio" );  ins( "фю", "fiu" );  ins( "фя", "fia" );  ins( "фь", "f" );
    ins( "хе", "chie" ); ins( "хё", "chio" ); ins( "хю", "chiu" ); ins( "хя", "chia" );
    ins( "це", "cie" );  ins( "цё", "cio" );  ins( "цю", "ciu" );  ins( "ця", "cia" );  ins( "ць", "ć" );
    ins( "бʼ", "b" );    ins( "б'", "b" );    ins( "б’", "b" );
    ins( "вʼ", "v" );    ins( "в'", "v" );    ins( "в’", "v" );
    ins( "гʼ", "h" );    ins( "г'", "h" );    ins( "г’", "h" );
    ins( "ґʼ", "g" );    ins( "ґ'", "g" );    ins( "ґ’", "g" );
    ins( "дʼ", "d" );    ins( "д'", "d" );    ins( "д’", "d" );
    ins( "жʼ", "ž" );    ins( "ж'", "ž" );    ins( "ж’", "ž" );
    ins( "зʼ", "z" );    ins( "з'", "z" );    ins( "з’", "z" );
    ins( "кʼ", "k" );    ins( "к'", "k" );    ins( "к’", "k" );
    ins( "лʼ", "ł" );    ins( "л'", "ł" );    ins( "л’", "ł" );
    ins( "мʼ", "m" );    ins( "м'", "m" );    ins( "м’", "m" );
    ins( "нʼ", "n" );    ins( "н'", "n" );    ins( "н’", "n" );
    ins( "пʼ", "p" );    ins( "п'", "p" );    ins( "п’", "p" );
    ins( "рʼ", "r" );    ins( "р'", "r" );    ins( "р’", "r" );
    ins( "сʼ", "s" );    ins( "с'", "s" );    ins( "с’", "s" );
    ins( "тʼ", "t" );    ins( "т'", "t" );    ins( "т’", "t" );
    ins( "фʼ", "f" );    ins( "ф'", "f" );    ins( "ф’", "f" );
    ins( "хʼ", "ch" );   ins( "х'", "ch" );   ins( "х’", "ch" );
    ins( "цʼ", "c" );    ins( "ц'", "c" );    ins( "ц’", "c" );
    ins( "чʼ", "č" );    ins( "ч'", "č" );    ins( "ч’", "č" );
    ins( "шʼ", "š" );    ins( "ш'", "š" );    ins( "ш’", "š" );
  }
};

class BelarusianLatinToSchoolTable: public Transliteration::Table
{
public:
  BelarusianLatinToSchoolTable()
  {
    // Utf8ins()

    // latin to cyrillic
    ins( "a", "а" );
    ins( "b", "б" );
    ins( "c", "ц" );
    ins( "ć", "ць" );
    ins( "č", "ч" );
    ins( "cz", "ч" );
    ins( "d", "д" );
    ins( "e", "э" );
    ins( "f", "ф" );
    ins( "g", "г" );
    ins( "h", "г" );
    ins( "i", "і" );
    ins( "j", "й" );
    ins( "k", "к" );
    ins( "l", "ль" );
    ins( "ł", "л" );
    ins( "m", "м" );
    ins( "n", "н" );
    ins( "ń", "нь" );
    ins( "o", "о" );
    ins( "p", "п" );
    ins( "r", "р" );
    ins( "s", "с" );
    ins( "ś", "сь" );
    ins( "š", "ш" );
    ins( "sz", "ш" );
    ins( "t", "т" );
    ins( "u", "у" );
    ins( "ŭ", "ў" );
    ins( "v", "в" );
    ins( "w", "в" );
    ins( "y", "ы" );
    ins( "z", "з" );
    ins( "ź", "зь" );
    ins( "ž", "ж" );
    ins( "ż", "ж" );
    ins( "ch", "х" );

    ins( "ja", "я" );   ins( "je", "е" );   ins( "jo", "ё" );   ins( "ju", "ю" );
    ins( "la", "ля" );  ins( "le", "ле" );  ins( "lo", "лё" );  ins( "lu", "лю" );  ins( "li", "лі" );

    ins( "bia", "бя" );  ins( "bie", "бе" );  ins( "bio", "бё" );  ins( "biu", "бю" );
    ins( "cia", "ця" );  ins( "cie", "це" );  ins( "cio", "цё" );  ins( "ciu", "цю" );
    ins( "dzia","дзя" ); ins( "dzie","дзе" ); ins( "dzio","дзё" ); ins( "dziu","дзю" );
    ins( "fia", "фя" );  ins( "fie", "фе" );  ins( "fio", "фё" );  ins( "fiu", "фю" );
    ins( "gia", "гя" );  ins( "gie", "ге" );  ins( "gio", "гё" );  ins( "giu", "гю" );
    ins( "hia", "гя" );  ins( "hie", "ге" );  ins( "hio", "гё" );  ins( "hiu", "гю" );
    ins( "kia", "кя" );  ins( "kie", "ке" );  ins( "kio", "кё" );  ins( "kiu", "кю" );
    ins( "lia", "ліa" ); ins( "lie", "ліэ" ); ins( "lio", "ліо" ); ins( "liu", "ліу" );
    ins( "mia", "мя" );  ins( "mie", "ме" );  ins( "mio", "мё" );  ins( "miu", "мю" );
    ins( "nia", "ня" );  ins( "nie", "не" );  ins( "nio", "нё" );  ins( "niu", "ню" );
    ins( "pia", "пя" );  ins( "pie", "пе" );  ins( "pio", "пё" );  ins( "piu", "пю" );
    ins( "sia", "ся" );  ins( "sie", "се" );  ins( "sio", "сё" );  ins( "siu", "сю" );
    ins( "via", "вя" );  ins( "vie", "ве" );  ins( "vio", "вё" );  ins( "viu", "вю" );
    ins( "wia", "вя" );  ins( "wie", "ве" );  ins( "wio", "вё" );  ins( "wiu", "вю" );
    ins( "zia", "зя" );  ins( "zie", "зе" );  ins( "zio", "зё" );  ins( "ziu", "зю" );
    ins( "chia", "хя" ); ins( "chie", "хе" ); ins( "chio", "хё" ); ins( "chiu", "хю" );

    ins( "bja", "б'я" );  ins( "bje", "б'е" );  ins( "bjo", "б'ё" );  ins( "bju", "б'ю" );
    ins( "cia", "ц'я" );  ins( "cje", "ц'е" );  ins( "cjo", "ц'ё" );  ins( "cju", "ц'ю" );
    ins( "fja", "ф'я" );  ins( "fje", "ф'е" );  ins( "fjo", "ф'ё" );  ins( "fju", "ф'ю" );
    ins( "hja", "г'я" );  ins( "hje", "г'е" );  ins( "hjo", "г'ё" );  ins( "hju", "г'ю" );
    ins( "kja", "к'я" );  ins( "kje", "к'е" );  ins( "kjo", "к'ё" );  ins( "kju", "к'ю" );
    ins( "łja", "л'я" );  ins( "łje", "л'е" );  ins( "łjo", "л'ё" );  ins( "łju", "л'ю" );
    ins( "mja", "м'я" );  ins( "mje", "м'е" );  ins( "mjo", "м'ё" );  ins( "mju", "м'ю" );
    ins( "nja", "н'я" );  ins( "nje", "н'е" );  ins( "njo", "н'ё" );  ins( "nju", "н'ю" );
    ins( "pja", "п'я" );  ins( "pje", "п'е" );  ins( "pjo", "п'ё" );  ins( "pju", "п'ю" );
    ins( "sja", "с'я" );  ins( "sje", "с'е" );  ins( "sjo", "с'ё" );  ins( "sju", "с'ю" );
    ins( "vja", "в'я" );  ins( "vje", "в'е" );  ins( "vjo", "в'ё" );  ins( "vju", "в'ю" );
    ins( "wja", "в'я" );  ins( "wje", "в'е" );  ins( "wjo", "в'ё" );  ins( "wju", "в'ю" );
    ins( "zja", "з'я" );  ins( "zje", "з'е" );  ins( "zjo", "з'ё" );  ins( "zju", "з'ю" );
    ins( "źja", "з'я" );  ins( "źje", "з'е" );  ins( "źjo", "з'ё" );  ins( "źju", "з'ю" );
    ins( "chja", "х'я" ); ins( "chje", "х'е" ); ins( "chjo", "х'ё" ); ins( "chju", "х'ю" );

    ins( "śbia", "сбя" );  ins( "śbie", "сбе" );  ins( "śbio", "сбё" );  ins( "śbiu", "сбю" );  ins( "śbi", "сбі" );
    ins( "ścia", "сця" );  ins( "ście", "сце" );  ins( "ścio", "сцё" );  ins( "ściu", "сцю" );  ins( "ści", "сці" );  ins( "ść", "сць" );
    ins( "śdzia","сдзя" ); ins( "śdzie","сдзе" ); ins( "śdzio","сдзё" ); ins( "śdziu","сдзю" ); ins( "śdzi","сдзі" ); ins( "śdź","сдзь" );
    ins( "śfia", "сфя" );  ins( "śfie", "сфе" );  ins( "śfio", "сфё" );  ins( "śfiu", "сфю" );  ins( "śfi", "сфі" );
    ins( "ślia", "сліa" ); ins( "ślie", "сліэ" ); ins( "ślio", "сліо" ); ins( "śliu", "сліу" ); ins( "śli", "слі" );  ins( "śl", "сль" );
    ins( "śmia", "смя" );  ins( "śmie", "сме" );  ins( "śmio", "смё" );  ins( "śmiu", "смю" );  ins( "śmi", "смі" );
    ins( "śnia", "сня" );  ins( "śnie", "сне" );  ins( "śnio", "снё" );  ins( "śniu", "сню" );  ins( "śni", "сні" );  ins( "śń", "снь" );
    ins( "śpia", "спя" );  ins( "śpie", "спе" );  ins( "śpio", "спё" );  ins( "śpiu", "спю" );  ins( "śpi", "спі" );
    ins( "śsia", "сся" );  ins( "śsie", "ссе" );  ins( "śsio", "ссё" );  ins( "śsiu", "ссю" );  ins( "śsi", "ссі" );
    ins( "śvia", "свя" );  ins( "śvie", "све" );  ins( "śvio", "свё" );  ins( "śviu", "свю" );  ins( "śvi", "сві" );
    ins( "świa", "свя" );  ins( "świe", "све" );  ins( "świo", "свё" );  ins( "świu", "свю" );  ins( "świ", "сві" );
    ins( "śzia", "сзя" );  ins( "śzie", "сзе" );  ins( "śzio", "сзё" );  ins( "śziu", "сзю" );  ins( "śzi", "сзі" );  ins( "śź", "сзь" );

    ins( "ćbia", "цбя" );  ins( "ćbie", "цбе" );  ins( "ćbio", "цбё" );  ins( "ćbiu", "цбю" );  ins( "ćbi", "цбі" );
    ins( "ćcia", "цця" );  ins( "ćcie", "цце" );  ins( "ćcio", "ццё" );  ins( "ćciu", "ццю" );  ins( "ćci", "цці" );
    ins( "ćdzia","цдзя" ); ins( "ćdzie","цдзе" ); ins( "ćdzio","цдзё" ); ins( "ćdziu","цдзю" ); ins( "ćdzi","цдзі" );
    ins( "ćfia", "цфя" );  ins( "ćfie", "цфе" );  ins( "ćfio", "цфё" );  ins( "ćfiu", "цфю" );  ins( "ćfi", "цфі" );
    ins( "ćlia", "цліa" ); ins( "ćlie", "цліэ" ); ins( "ćlio", "цліо" ); ins( "ćliu", "цліу" ); ins( "ćli", "цлі" );  ins( "ćl", "цль" );
    ins( "ćmia", "цмя" );  ins( "ćmie", "цме" );  ins( "ćmio", "цмё" );  ins( "ćmiu", "цмю" );  ins( "ćmi", "цмі" );
    ins( "ćnia", "цня" );  ins( "ćnie", "цне" );  ins( "ćnio", "цнё" );  ins( "ćniu", "цню" );  ins( "ćni", "цні" );  ins( "ćń", "цнь" );
    ins( "ćpia", "цпя" );  ins( "ćpie", "цпе" );  ins( "ćpio", "цпё" );  ins( "ćpiu", "цпю" );  ins( "ćpi", "цпі" );
    ins( "ćsia", "цся" );  ins( "ćsie", "цсе" );  ins( "ćsio", "цсё" );  ins( "ćsiu", "цсю" );  ins( "ćsi", "цсі" );  ins( "ćś", "цсь" );
    ins( "ćvia", "цвя" );  ins( "ćvie", "цве" );  ins( "ćvio", "цвё" );  ins( "ćviu", "цвю" );  ins( "ćvi", "цві" );
    ins( "ćwia", "цвя" );  ins( "ćwie", "цве" );  ins( "ćwio", "цвё" );  ins( "ćwiu", "цвю" );  ins( "ćwi", "цві" );
    ins( "ćzia", "цзя" );  ins( "ćzie", "цзе" );  ins( "ćzio", "цзё" );  ins( "ćziu", "цзю" );  ins( "ćzi", "цзі" );  ins( "ćź", "цзь" );

    ins( "źbia", "збя" );  ins( "źbie", "збе" );  ins( "źbio", "збё" );  ins( "źbiu", "збю" );  ins( "źbi", "збі" );
    ins( "źcia", "зця" );  ins( "źcie", "зце" );  ins( "źcio", "зцё" );  ins( "źciu", "зцю" );  ins( "źci", "зці" );  ins( "źć", "зць" );
    ins( "ździa","здзя" ); ins( "ździe","здзе" ); ins( "ździo","здзё" ); ins( "ździu","здзю" ); ins( "ździ","здзі" ); ins( "źdź","здзь" );
    ins( "źfia", "зфя" );  ins( "źfie", "зфе" );  ins( "źfio", "зфё" );  ins( "źfiu", "зфю" );  ins( "źfi", "зфі" );
    ins( "źlia", "зліa" ); ins( "źlie", "зліэ" ); ins( "źlio", "зліо" ); ins( "źliu", "зліу" ); ins( "źli", "злі" );  ins( "źl", "зль" );
    ins( "źmia", "змя" );  ins( "źmie", "зме" );  ins( "źmio", "змё" );  ins( "źmiu", "змю" );  ins( "źmi", "змі" );
    ins( "źnia", "зня" );  ins( "źnie", "зне" );  ins( "źnio", "знё" );  ins( "źniu", "зню" );  ins( "źni", "зні" );
    ins( "źpia", "зпя" );  ins( "źpie", "зпе" );  ins( "źpio", "зпё" );  ins( "źpiu", "зпю" );  ins( "źpi", "зпі" );
    ins( "źsia", "зся" );  ins( "źsie", "зсе" );  ins( "źsio", "зсё" );  ins( "źsiu", "зсю" );  ins( "źsi", "зсі" );
    ins( "źvia", "звя" );  ins( "źvie", "зве" );  ins( "źvio", "звё" );  ins( "źviu", "звю" );  ins( "źvi", "зві" );
    ins( "źwia", "звя" );  ins( "źwie", "зве" );  ins( "źwio", "звё" );  ins( "źwiu", "звю" );  ins( "źwi", "зві" );
    ins( "źzia", "ззя" );  ins( "źzie", "ззе" );  ins( "źzio", "ззё" );  ins( "źziu", "ззю" );  ins( "źzi", "ззі" );

    ins( "ńnia", "ння" );  ins( "ńnie", "нне" );  ins( "ńnio", "ннё" );  ins( "ńniu", "нню" );  ins( "ńni", "нні" );
    ins( "dździa", "ддзя" );  ins( "dździe", "ддзе" );  ins( "dździo", "ддзё" );  ins( "dździu", "ддзю" );  ins( "dździ", "ддзі" );

    // cyrillic to latin
    ins( "а", "a" );   ins( "б", "b" );   ins( "в", "v" );   ins( "г", "h" );
    ins( "ґ", "g" );   ins( "д", "d" );   ins( "е", "je" );  ins( "ё", "jo" );
    ins( "ж", "ž" );   ins( "з", "z" );   ins( "і", "i" );   ins( "ї", "ï" );
    ins( "й", "j" );   ins( "к", "k" );   ins( "л", "ł" );   ins( "м", "m" );
    ins( "н", "n" );   ins( "о", "o" );   ins( "п", "p" );   ins( "р", "r" );
    ins( "с", "s" );   ins( "т", "t" );   ins( "у", "u" );   ins( "ў", "ŭ" );
    ins( "ф", "f" );   ins( "х", "ch" );  ins( "ц", "c" );   ins( "ч", "č" );
    ins( "ш", "š" );   ins( "ы", "y" );   ins( "э", "e" );   ins( "ю", "ju" );
    ins( "я", "ja" );  ins( "кг", "g" );
    ins( "бе", "bie" );  ins( "бё", "bio" );  ins( "бю", "biu" );  ins( "бя", "bia" );
    ins( "ве", "vie" );  ins( "вё", "vio" );  ins( "вю", "viu" );  ins( "вя", "via" );
    ins( "ге", "hie" );  ins( "гё", "hio" );  ins( "гю", "hiu" );  ins( "гя", "hia" );
    ins( "ґе", "gie" );  ins( "ґё", "gio" );  ins( "ґю", "giu" );  ins( "ґя", "gia" );
    ins( "кге", "gie" ); ins( "кгё", "gio" ); ins( "кгю", "giu" ); ins( "кгя", "gia" );
    ins( "зе", "zie" );  ins( "зё", "zio" );  ins( "зю", "ziu" );  ins( "зя", "zia" );  ins( "зь", "ź" );
    ins( "ке", "kie" );  ins( "кё", "kio" );  ins( "кю", "kiu" );  ins( "кя", "kia" );
    ins( "ле", "le" );   ins( "лё", "lo" );   ins( "лю", "lu" );   ins( "ля", "la" );   ins( "ль", "l" );   ins( "лі", "li" );
    ins( "ме", "mie" );  ins( "мё", "mio" );  ins( "мю", "miu" );  ins( "мя", "mia" );  ins( "мь", "m" );
    ins( "не", "nie" );  ins( "нё", "nio" );  ins( "ню", "niu" );  ins( "ня", "nia" );  ins( "нь", "ń" );
    ins( "пе", "pie" );  ins( "пё", "pio" );  ins( "пю", "piu" );  ins( "пя", "pia" );  ins( "пь", "p" );
    ins( "се", "sie" );  ins( "сё", "sio" );  ins( "сю", "siu" );  ins( "ся", "sia" );  ins( "сь", "ś" );
    ins( "фе", "fie" );  ins( "фё", "fio" );  ins( "фю", "fiu" );  ins( "фя", "fia" );  ins( "фь", "f" );
    ins( "хе", "chie" ); ins( "хё", "chio" ); ins( "хю", "chiu" ); ins( "хя", "chia" );
    ins( "це", "cie" );  ins( "цё", "cio" );  ins( "цю", "ciu" );  ins( "ця", "cia" );  ins( "ць", "ć" );
    ins( "бʼ", "b" );    ins( "б'", "b" );    ins( "б’", "b" );
    ins( "вʼ", "v" );    ins( "в'", "v" );    ins( "в’", "v" );
    ins( "гʼ", "h" );    ins( "г'", "h" );    ins( "г’", "h" );
    ins( "ґʼ", "g" );    ins( "ґ'", "g" );    ins( "ґ’", "g" );
    ins( "дʼ", "d" );    ins( "д'", "d" );    ins( "д’", "d" );
    ins( "жʼ", "ž" );    ins( "ж'", "ž" );    ins( "ж’", "ž" );
    ins( "зʼ", "z" );    ins( "з'", "z" );    ins( "з’", "z" );
    ins( "кʼ", "k" );    ins( "к'", "k" );    ins( "к’", "k" );
    ins( "лʼ", "ł" );    ins( "л'", "ł" );    ins( "л’", "ł" );
    ins( "мʼ", "m" );    ins( "м'", "m" );    ins( "м’", "m" );
    ins( "нʼ", "n" );    ins( "н'", "n" );    ins( "н’", "n" );
    ins( "пʼ", "p" );    ins( "п'", "p" );    ins( "п’", "p" );
    ins( "рʼ", "r" );    ins( "р'", "r" );    ins( "р’", "r" );
    ins( "сʼ", "s" );    ins( "с'", "s" );    ins( "с’", "s" );
    ins( "тʼ", "t" );    ins( "т'", "t" );    ins( "т’", "t" );
    ins( "фʼ", "f" );    ins( "ф'", "f" );    ins( "ф’", "f" );
    ins( "хʼ", "ch" );   ins( "х'", "ch" );   ins( "х’", "ch" );
    ins( "цʼ", "c" );    ins( "ц'", "c" );    ins( "ц’", "c" );
    ins( "чʼ", "č" );    ins( "ч'", "č" );    ins( "ч’", "č" );
    ins( "шʼ", "š" );    ins( "ш'", "š" );    ins( "ш’", "š" );
  }
};

class BelarusianSchoolToClassicTable: public Transliteration::Table
{
public:
  BelarusianSchoolToClassicTable()
  {
    ins( "ньне", "нне" );   ins( "ньня", "ння" );   ins( "ньню", "нню" );   ins( "ньнё", "ннё" );   ins( "ньні", "нні" );
    ins( "нне", "ньне" );   ins( "ння", "ньня" );   ins( "нню", "ньню" );   ins( "ннё", "ньнё" );   ins( "нні", "ньні" );

    ins( "льле", "лле" );   ins( "льля", "лля" );   ins( "льлю", "ллю" );   ins( "льлё", "ллё" );   ins( "льлі", "ллі" );
    ins( "лле", "льле" );   ins( "лля", "льля" );   ins( "ллю", "льлю" );   ins( "ллё", "льлё" );   ins( "ллі", "льлі" );

    ins( "зьбе", "збе" );   ins( "зьбя", "збя" );   ins( "зьбю", "збю" );   ins( "зьбё", "збё" );   ins( "зьбі", "збі" );
    ins( "збе", "зьбе" );   ins( "збя", "зьбя" );   ins( "збю", "зьбю" );   ins( "збё", "зьбё" );   ins( "збі", "зьбі" );
    ins( "зьве", "зве" );   ins( "зьвя", "звя" );   ins( "зьвю", "звю" );   ins( "зьвё", "звё" );   ins( "зьві", "зві" );
    ins( "зве", "зьве" );   ins( "звя", "зьвя" );   ins( "звю", "зьвю" );   ins( "звё", "зьвё" );   ins( "зві", "зьві" );
    ins( "зьдзе", "здзе" ); ins( "зьдзя", "здзя" ); ins( "зьдзю", "здзю" ); ins( "зьдзё", "здзё" ); ins( "зьдзі", "здзі" ); ins( "зьдзь", "здзь" );
    ins( "здзе", "зьдзе" ); ins( "здзя", "зьдзя" ); ins( "здзю", "зьдзю" ); ins( "здзё", "зьдзё" ); ins( "здзі", "зьдзі" ); ins( "здзь", "здзь" );
    ins( "зьзе", "ззе" );   ins( "зьзя", "ззя" );   ins( "зьзю", "ззю" );   ins( "зьзё", "ззё" );   ins( "зьзі", "ззі" );   ins( "зьзь", "ззь" );
    ins( "ззе", "зьзе" );   ins( "ззя", "зьзя" );   ins( "ззю", "зьзю" );   ins( "ззё", "зьзё" );   ins( "ззі", "зьзі" );   ins( "ззь", "зьзь" );
    ins( "зьле", "зле" );   ins( "зьля", "зля" );   ins( "зьлю", "злю" );   ins( "зьлё", "злё" );   ins( "зьлі", "злі" );   ins( "зьль", "зль" );
    ins( "зле", "зьле" );   ins( "зля", "зьля" );   ins( "злю", "зьлю" );   ins( "злё", "зьлё" );   ins( "злі", "зьлі" );   ins( "зль", "зьль" );
    ins( "зьме", "зме" );   ins( "зьмя", "змя" );   ins( "зьмю", "змю" );   ins( "зьмё", "змё" );   ins( "зьмі", "змі" );
    ins( "зме", "зьме" );   ins( "змя", "зьмя" );   ins( "змю", "зьмю" );   ins( "змё", "зьмё" );   ins( "змі", "зьмі" );
    ins( "зьне", "зне" );   ins( "зьня", "зня" );   ins( "зьню", "зню" );   ins( "зьнё", "знё" );   ins( "зьні", "зні" );   ins( "зьнь", "знь" );
    ins( "зне", "зьне" );   ins( "зня", "зьня" );   ins( "зню", "зьню" );   ins( "знё", "зьнё" );   ins( "зні", "зьні" );   ins( "знь", "зьнь" );
    ins( "зьпе", "зпе" );   ins( "зьпя", "зпя" );   ins( "зьпю", "зпю" );   ins( "зьпё", "зпё" );   ins( "зьпі", "зпі" );
    ins( "зпе", "зьпе" );   ins( "зпя", "зьпя" );   ins( "зпю", "зьпю" );   ins( "зпё", "зьпё" );   ins( "зпі", "зьпі" );
    ins( "зьсе", "зсе" );   ins( "зься", "зся" );   ins( "зьсю", "зсю" );   ins( "зьсё", "зсё" );   ins( "зьсі", "зсі" );   ins( "зьсь", "зсь" );
    ins( "зсе", "зьсе" );   ins( "зся", "зься" );   ins( "зсю", "зьсю" );   ins( "зсё", "зьсё" );   ins( "зсі", "зьсі" );   ins( "зсь", "зьсь" );
    ins( "зьфе", "зфе" );   ins( "зьфя", "зфя" );   ins( "зьфю", "зфю" );   ins( "зьфё", "зфё" );   ins( "зьфі", "зфі" );
    ins( "зфе", "зьфе" );   ins( "зфя", "зьфя" );   ins( "зфю", "зьфю" );   ins( "зфё", "зьфё" );   ins( "зфі", "зьфі" );
    ins( "зьхе", "зхе" );   ins( "зьхя", "зхя" );   ins( "зьхю", "зхю" );   ins( "зьхё", "зхё" );   ins( "зьхі", "зхі" );
    ins( "зхе", "зьхе" );   ins( "зхя", "зьхя" );   ins( "зхю", "зьхю" );   ins( "зхё", "зьхё" );   ins( "зхі", "зьхі" );
    ins( "зьце", "зце" );   ins( "зьця", "зця" );   ins( "зьцю", "зцю" );   ins( "зьцё", "зцё" );   ins( "зьці", "зці" );   ins( "зьць", "зць" );
    ins( "зце", "зьце" );   ins( "зця", "зьця" );   ins( "зцю", "зьцю" );   ins( "зцё", "зьцё" );   ins( "зці", "зьці" );   ins( "зць", "зьць" );

    ins( "сьбе", "сбе" );   ins( "сьбя", "сбя" );   ins( "сьбю", "сбю" );   ins( "сьбё", "сбё" );   ins( "сьбі", "сбі" );
    ins( "сбе", "сьбе" );   ins( "сбя", "сьбя" );   ins( "сбю", "сьбю" );   ins( "сбё", "сьбё" );   ins( "сбі", "сьбі" );
    ins( "сьве", "све" );   ins( "сьвя", "свя" );   ins( "сьвю", "свю" );   ins( "сьвё", "свё" );   ins( "сьві", "сві" );
    ins( "све", "сьве" );   ins( "свя", "сьвя" );   ins( "свю", "сьвю" );   ins( "свё", "сьвё" );   ins( "сві", "зьві" );
    ins( "сьдзе", "сдзе" ); ins( "сьдзя", "сдзя" ); ins( "сьдзю", "сдзю" ); ins( "сьдзё", "сдзё" ); ins( "сьдзі", "сдзі" ); ins( "сьдзь", "сдзь" );
    ins( "сдзе", "сьдзе" ); ins( "сдзя", "сьдзя" ); ins( "сдзю", "сьдзю" ); ins( "сдзё", "сьдзё" ); ins( "сдзі", "сьдзі" ); ins( "сдзь", "сьдзь" );
    ins( "сьзе", "сзе" );   ins( "сьзя", "сзя" );   ins( "сьзю", "сзю" );   ins( "сьзё", "сзё" );   ins( "сьзі", "сзі" );   ins( "сьзь", "сзь" );
    ins( "сзе", "сьзе" );   ins( "сзя", "сьзя" );   ins( "сзю", "сьзю" );   ins( "сзё", "сьзё" );   ins( "сзі", "сьзі" );   ins( "сзь", "сьзь" );
    ins( "сьле", "сле" );   ins( "сьля", "сля" );   ins( "сьлю", "слю" );   ins( "сьлё", "слё" );   ins( "сьлі", "слі" );   ins( "сьль", "сль" );
    ins( "сле", "сьле" );   ins( "сля", "сьля" );   ins( "слю", "сьлю" );   ins( "слё", "сьлё" );   ins( "слі", "сьлі" );   ins( "сль", "сьль" );
    ins( "сьме", "сме" );   ins( "сьмя", "смя" );   ins( "сьмю", "смю" );   ins( "сьмё", "смё" );   ins( "сьмі", "смі" );
    ins( "сме", "сьме" );   ins( "смя", "сьмя" );   ins( "смю", "сьмю" );   ins( "смё", "сьмё" );   ins( "смі", "сьмі" );
    ins( "сьне", "сне" );   ins( "сьня", "сня" );   ins( "сьню", "сню" );   ins( "сьнё", "снё" );   ins( "сьні", "сні" );   ins( "сьнь", "снь" );
    ins( "сне", "сьне" );   ins( "сня", "сьня" );   ins( "сню", "сьню" );   ins( "снё", "сьнё" );   ins( "сні", "сьні" );   ins( "снь", "сьнь" );
    ins( "сьпе", "спе" );   ins( "сьпя", "спя" );   ins( "сьпю", "спю" );   ins( "сьпё", "спё" );   ins( "сьпі", "спі" );
    ins( "спе", "сьпе" );   ins( "спя", "сьпя" );   ins( "спю", "сьпю" );   ins( "спё", "сьпё" );   ins( "спі", "сьпі" );
    ins( "сьсе", "ссе" );   ins( "сься", "сся" );   ins( "сьсю", "ссю" );   ins( "сьсё", "ссё" );   ins( "сьсі", "ссі" );   ins( "сьсь", "ссь" );
    ins( "ссе", "сьсе" );   ins( "сся", "сься" );   ins( "ссю", "сьсю" );   ins( "ссё", "сьсё" );   ins( "ссі", "сьсі" );   ins( "ссь", "сьсь" );
    ins( "сьфе", "сфе" );   ins( "сьфя", "сфя" );   ins( "сьфю", "сфю" );   ins( "сьфё", "сфё" );   ins( "сьфі", "сфі" );
    ins( "сфе", "сьфе" );   ins( "сфя", "сьфя" );   ins( "сфю", "сьфю" );   ins( "сфё", "сьфё" );   ins( "сфі", "сьфі" );
    ins( "сьхе", "схе" );   ins( "сьхя", "схя" );   ins( "сьхю", "схю" );   ins( "сьхё", "схё" );   ins( "сьхі", "схі" );
    ins( "схе", "сьхе" );   ins( "схя", "сьхя" );   ins( "схю", "сьхю" );   ins( "схё", "сьхё" );   ins( "схі", "сьхі" );
    ins( "сьце", "сце" );   ins( "сьця", "сця" );   ins( "сьцю", "сцю" );   ins( "сьцё", "сцё" );   ins( "сьці", "сці" );   ins( "сьць", "сць" );
    ins( "сце", "сьце" );   ins( "сця", "сьця" );   ins( "сцю", "сьцю" );   ins( "сцё", "сьцё" );   ins( "сці", "сьці" );   ins( "сць", "сьць" );

    ins( "цьбе", "цбе" );   ins( "цьбя", "цбя" );   ins( "цьбю", "цбю" );   ins( "цьбё", "цбё" );   ins( "цьбі", "цбі" );
    ins( "цбе", "цьбе" );   ins( "цбя", "цьбя" );   ins( "цбю", "цьбю" );   ins( "цбё", "цьбё" );   ins( "цбі", "цьбі" );
    ins( "цьве", "цве" );   ins( "цьвя", "цвя" );   ins( "цьвю", "цвю" );   ins( "цьвё", "цвё" );   ins( "цьві", "цві" );
    ins( "цве", "цьве" );   ins( "цвя", "цьвя" );   ins( "цвю", "цьвю" );   ins( "цвё", "цьвё" );   ins( "цві", "цьві" );
    ins( "цьдзе", "цдзе" ); ins( "цьдзя", "цдзя" ); ins( "цьдзю", "цдзю" ); ins( "цьдзё", "цдзё" ); ins( "цьдзі", "цдзі" ); ins( "цьдзь", "цдзь" );
    ins( "цдзе", "цьдзе" ); ins( "цдзя", "цьдзя" ); ins( "цдзю", "цьдзю" ); ins( "цдзё", "цьдзё" ); ins( "цдзі", "цьдзі" ); ins( "цдзь", "цьдзь" );
    ins( "цьзе", "цзе" );   ins( "цьзя", "цзя" );   ins( "цьзю", "цзю" );   ins( "цьзё", "цзё" );   ins( "цьзі", "цзі" );   ins( "цьзь", "цзь" );
    ins( "цзе", "цьзе" );   ins( "цзя", "цьзя" );   ins( "цзю", "цьзю" );   ins( "цзё", "цьзё" );   ins( "цзі", "цьзі" );   ins( "цзь", "цьзь" );
    ins( "цьле", "цле" );   ins( "цьля", "цля" );   ins( "цьлю", "цлю" );   ins( "цьлё", "цлё" );   ins( "цьлі", "цлі" );   ins( "цьль", "цль" );
    ins( "цле", "цьле" );   ins( "цля", "цьля" );   ins( "цлю", "цьлю" );   ins( "цлё", "цьлё" );   ins( "цлі", "цьлі" );   ins( "цль", "цьль" );
    ins( "цьме", "цме" );   ins( "цьмя", "цмя" );   ins( "цьмю", "цмю" );   ins( "цьмё", "цмё" );   ins( "цьмі", "цмі" );
    ins( "цме", "цьме" );   ins( "цмя", "цьмя" );   ins( "цмю", "цьмю" );   ins( "цмё", "цьмё" );   ins( "цмі", "цьмі" );
    ins( "цьне", "цне" );   ins( "цьня", "цня" );   ins( "цьню", "цню" );   ins( "цьнё", "цнё" );   ins( "цьні", "цні" );   ins( "цьнь", "цнь" );
    ins( "цне", "цьне" );   ins( "цня", "цьня" );   ins( "цню", "цьню" );   ins( "цнё", "цьнё" );   ins( "цні", "цьні" );   ins( "цнь", "цьнь" );
    ins( "цьпе", "цпе" );   ins( "цьпя", "цпя" );   ins( "цьпю", "цпю" );   ins( "цьпё", "цпё" );   ins( "цьпі", "цпі" );
    ins( "цпе", "цьпе" );   ins( "цпя", "цьпя" );   ins( "цпю", "цьпю" );   ins( "цпё", "цьпё" );   ins( "цпі", "цьпі" );
    ins( "цьсе", "цсе" );   ins( "цься", "цся" );   ins( "цьсю", "цсю" );   ins( "цьсё", "цсё" );   ins( "цьсі", "цсі" );   ins( "цьсь", "цсь" );
    ins( "цсе", "цьсе" );   ins( "цся", "цься" );   ins( "цсю", "цьсю" );   ins( "цсё", "цьсё" );   ins( "цсі", "цьсі" );   ins( "цсь", "цьсь" );
    ins( "цьфе", "цфе" );   ins( "цьфя", "цфя" );   ins( "цьфю", "цфю" );   ins( "цьфё", "цфё" );   ins( "цьфі", "цфі" );
    ins( "цфе", "цьфе" );   ins( "цфя", "цьфя" );   ins( "цфю", "цьфю" );   ins( "цфё", "цьфё" );   ins( "цфі", "цьфі" );
    ins( "цьце", "цце" );   ins( "цьця", "цця" );   ins( "цьцю", "ццю" );   ins( "цьцё", "ццё" );   ins( "цьці", "цці" );   ins( "цьць", "цць" );
    ins( "цце", "цьце" );   ins( "цця", "цьця" );   ins( "ццю", "цьцю" );   ins( "ццё", "цьцё" );   ins( "цці", "цьці" );   ins( "цць", "цьць" );

    ins( "дзьдзе", "ддзе" ); ins( "дзьдзя", "ддзя" ); ins( "дзьдзю", "ддзю" ); ins( "дзьдзё", "ддзё" ); ins( "дзьдзі", "ддзі" );
    ins( "ддзе", "дзьдзе" ); ins( "ддзя", "дзьдзя" ); ins( "ддзю", "дзьдзю" ); ins( "ддзё", "дзьдзё" ); ins( "ддзі", "дзьдзі" );

    ins( "з'е", "зье" );   ins( "з'я", "зья" );   ins( "з'ю", "зью" );   ins( "з'ё", "зьё" );   ins( "з'і", "зьі" );
    ins( "зье", "з'е" );   ins( "зья", "з'я" );   ins( "зью", "з'ю" );   ins( "зьё", "з'ё" );   ins( "зьі", "з'і" );
    ins( "зʼе", "зье" );   ins( "зʼя", "зья" );   ins( "зʼю", "зью" );   ins( "зʼё", "зьё" );   ins( "зʼі", "зьі" );
    ins( "зье", "зʼе" );   ins( "зья", "зʼя" );   ins( "зью", "зʼю" );   ins( "зьё", "зʼё" );   ins( "зьі", "зʼі" );
    ins( "з’е", "зье" );   ins( "з’я", "зья" );   ins( "з’ю", "зью" );   ins( "з’ё", "зьё" );   ins( "з’і", "зьі" );
    ins( "зье", "з’е" );   ins( "зья", "з’я" );   ins( "зью", "з’ю" );   ins( "зьё", "з’ё" );   ins( "зьі", "з’і" );
  }
};

std::vector< sptr< Dictionary::Class > > makeDictionaries() THROW_SPEC( std::exception )
{
  static BelarusianLatinToClassicTable t0;
  static BelarusianLatinToSchoolTable t1;
  static BelarusianSchoolToClassicTable t2;
  std::vector< sptr< Dictionary::Class > > dicts;

  dicts.push_back( new Transliteration::TransliterationDictionary( "c31b24abf412abc9b23bb40a898f1040",
                   QCoreApplication::translate( "BelarusianTranslit", "Belarusian transliteration from latin to cyrillic (classic orthography)" ).toUtf8().data(),
                   QIcon( ":/flags/by.png" ), t0, false ) );
  dicts.push_back( new Transliteration::TransliterationDictionary( "c31b24abf412abc9b23bb40a898f1041",
                   QCoreApplication::translate( "BelarusianTranslit", "Belarusian transliteration from latin to cyrillic (school orthography)" ).toUtf8().data(),
                   QIcon( ":/flags/by.png" ), t1, false ) );
  dicts.push_back( new Transliteration::TransliterationDictionary( "c31b24abf412abc9b23bb40a898f1042",
                   QCoreApplication::translate( "BelarusianTranslit", "Belarusian transliteration (smoothes out the difference\n"
                                                "between classic and school orthography in cyrillic)" ).toUtf8().data(),
                   QIcon( ":/flags/by.png" ), t2, false ) );
  return dicts;
}

}
