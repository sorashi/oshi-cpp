# oshi

A rewrite of GeekJDict in C++ ([previously a rewrite in Python](https://github.com/sorashi/oshi))

slovník japonské gramatiky

## Závislosti

- [grammar.rules](grammar.rules) (licence uvnitř souboru)
- googletest ([licence](https://github.com/google/googletest/blob/main/LICENSE))
- JMdict ([licence](https://www.edrdg.org/edrdg/licence.html)), japonský slovník komprimovaný gzip (~9 MB)
- zlib ([licence](https://www.zlib.net/zlib_license.html)) je C knihovna k (de)kompresi zlib/gzip, program ji používá k
  dekompresi slovníku, je dynamicky linkovaná, na Windows se DLL kopíruje do výstupního adresáře, na Linuxu se
  předpokládá, že je zlib předinstalován (kontrola `ldconfig -p | grep libz.so`)
- pugixml ([licence](https://pugixml.org/license.html)) je XML knihovna, program ji používá ke čtení slovníku, staticky
  linkovaná
- [glob-cpp](https://github.com/alexst07/glob-cpp) ([licence](https://github.com/alexst07/glob-cpp/blob/master/LICENSE))
  je glob knihovna, přímo překládaná ze složky `./glob-cpp`

## INSTALACE

Ověřeno v labu přes ssh.

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target oshi --config Release -- -j 6
```

Případně `--target tests` pro testy.

Při konfiguraci (viz [CMakeLists.txt](CMakeLists.txt)) se nastahují závislosti: googletest, JMdict, zlib, pugixml

**Varování**: Po spuštění program dekomprimuje `JMdict_e.gz` na disku do `JMdict_e.xml`. Dekomprimovaný soubor **má kolem 50MB**.
Soubor je poté načten v paměti (pomocí pugixml) a jsou z něj vyextrahována potřebná data do vlastních struktur, poté je
soubor zavřen a DOM smazán z paměti.

Slovník se dekomprimuje až po spuštění programu, protože CMake nepodporuje dekompresi samostatného gz (jen .tar.gz).
[CMake Archive Extract](https://cmake.org/cmake/help/latest/command/file.html#archive-extract),
případně [CMake command-line tools](https://cmake.org/cmake/help/latest/manual/cmake.1.html#run-a-command-line-tool)
obsahují jen tar.gz extraktory.

## DEMO

Program je připraven na vstup ve chvíli, kdy vypíše prompt `>`.

- Vstup `書いてた` (sloveso "psát" ve tvaru minulého hovorového průběhového času z minulé te-formy)
- Výstup

```text
書いてた is past for 書いてる
  書いてる is colloquial for 書いている
    書いている is continuous for 書いて
      書いて is て-form for 書いた
        書いた is past for 書く
書く [かく]: (v5k vt) to write, to compose, to pen; (v5k vt) to draw, to paint
```

- Vstup `良くなかった` (přídavné jméno "dobrý", "v pořádku", v záporném tvaru času minulého, tedy "nebyl dobrý")
- Výstup

```text
良くなかった is past for 良くない
  良くない is negative for 良い
良い 善い 好い 佳い 吉い 宜い [よい えい]: (adj-i) good, excellent, fine, nice, pleasant, agreeable; (adj-i) sufficient, enough, ready, prepared; (adj-i) profitable (deal, business offer, etc.), beneficial; (adj-i) OK, all right, fine, no problem
```

- Vstup: `知っていた` (sloveso "vědět/zjistit/rozumět", v zajímavém gramatickém
  tvaru, který *describes a past state which resulted from an earlier change of
  state*, [zdroj](https://japanese.stackexchange.com/a/93859/7174))
- Výstup

```text
知っていた
知っていた is past for 知ってく
  知ってく is colloquial for 知っていく
    知っていく is direction-away for 知って
      知って is て-form for 知った
        知った is past for 知る
知る 識る [しる]: (v5r vt) to know, to be aware (of), to be conscious (of), to learn (of), to find out, to discover; (v5r vt) to sense, to feel, to notice, to realize; (v5r vt) to understand, to comprehend, to grasp, to appreciate; (v5r vt) to remember, to be familiar with, to be acquainted with; (v5r vt) to experience, to go through, to know (e.g. hardship); (v5r vt) to get acquainted with (a person), to get to know; (v5r vt) to have to do with, to be concerned with, to be one's concern, to be one's responsibility
```

## Technická dokumentace

Program funguje tak, že na zadaný tvar rekurzivně aplikuje přes 200 předem
připravených gramatických pravidel, dokud nevznikne tvar, který nalezne ve
slovníku.

- `Grammar.cpp/h`: parsování a reprezentace gramatických pravidel, a reprezentace gramatických forem při hledání tvaru
- `Dictionary.cpp/h`: parsování, zpracování a prohledávání slovníku JMdict
- `Utilities.cpp/h`: pomocné funkce, operace se stringy, extrahování pomocí zlib
- `GrammarFormGuesser.cpp/h`: inference gramatického tvaru hledáním do hloubky, reprezentace (mezi)výsledků
- `test/tests.cpp`: unit testy

### grammar.rules

Tento soubor od Tomashe Brechka byl vytvořen podle [A Guide to Japanese Grammar](https://guidetojapanese.org/learn/grammar). Soubor obsahuje gramatická pravidla ve formátu, který je v hlavičce daného souboru popsán.

Formát:

```
RULE [ROLE] PATTERN [POS] for TARGET PATTERN POS-GLOB...
```

Příklad pravidla:

```
passive plain 〜アれる v1 for plain 〜ウ v5[^a]* vs-c
```

Toto pravidlo říká, že tvary končící na (a-zvuk)-re-ru jsou možná pasivní formou
některých
[godan](https://en.wikipedia.org/wiki/Japanese_godan_and_ichidan_verbs) (v5)
sloves, která získáme odtržením zmíněné koncovky a přidáním k "a-zvuku"
odpovídajícího "u-zvuku".

Například 読まれる \[yomareru\] (je čteno) je pasivní forma slovesa 読む \[yomu\] (číst).

Každé pravidlo je v projektu reprezentováno třídou `GrammarRule`. Tato pravidla
drží třída `Grammar`.

### JMdict

[JMDICT](http://www.edrdg.org/jmdict/j_jmdict.html) files are the property of
the [Electronic Dictionary Research and Development Group](http://www.edrdg.org), and are used in conformance with the
Group's [licence](http://www.edrdg.org/edrdg/licence.html).

Soubor [JMdict](https://www.edrdg.org/jmdict/j_jmdict.html) od skupiny EDRDG obsahuje data japonsko-anglického slovníku
ve formátu XML s kódováním UTF-8.

Ve stromě níže je znázorněna struktura jednoho záznamu z JMdict (zkrácena pouze na XML tagy, které používá program Oshi)
.

- tag `<entry>` (*záznam*)
  - tag `<r_ele>` (*reading element*) (1 a více)
    - tag `<reb>` (právě 1)
  - tag `<k_ele>` (*kanji element*) (0 a více)
    - tag `<keb>` (právě 1)
  - tag `<sense>` (*význam*) (1 a více)
    - tag `<pos>` (*part of speech*) (0 a více - pokud 0, platí `pos` z předchozího `sense`)
    - tag `<gloss>` (*překlad*) (1 a více)

Příklad elementu `entry` pro sloveso "psát" (pouze elementy zájmu):

```xml
<entry>
    <k_ele>
        <keb>書く</keb>
    </k_ele>
    <r_ele>
        <reb>かく</reb>
    </r_ele>
    <sense>
        <pos>&v5k;</pos>
        <pos>&vt;</pos>
        <gloss>to write</gloss>
        <gloss>to compose</gloss>
        <gloss>to pen</gloss>
    </sense>
    <sense>
        <gloss>to draw</gloss>
        <gloss>to paint</gloss>
    </sense>
</entry>
```

- element `k_ele` obsahuje zápis pomocí
  [kanji](https://cs.wikipedia.org/wiki/Kand%C5%BEi)
- element `r_ele` obsahuje fonetický zápis pomocí
  [kany](https://cs.wikipedia.org/wiki/Kana_(p%C3%ADsmo))
- element `pos` je *part of speech* (ve zdrojovém kódu spíše označován jako
  *tag*, kvůli zobecnění), neboli informace o gramatické roli významu. Tato informace je zapsána pomocí XML entity,
  např. `&v5k;`. Tyto entity jsou definovány na začátku XML souboru, např. `v5k` znamená *Godan verb with `ku' ending*.

### Pojednání o znacích UTF-8

Nad UTF-8 lze přemýšlet v několika úrovních:

- jednotlivé bajty, tj. reprezentace daného textu v paměti
- skalární hodnoty Unicode
- skupiny grafémů

Indexování běžného C++ stringu přistupuje k bajtům.

```cpp
#include <iostream>
#include <string>

int main() {
  std::string a = "来る";
  std::cout << a << std::endl;
  for(int i = 0; i < a.size(); ++i) {
    std::cout << "[" << i << "]: " << a[i] << std::endl;
  }
  for(auto c : a) std::cout << c << std::endl;
}
```

```
来る
[0]:  
[1]:  
[2]:  
[3]:  
[4]:  
[5]:  
 
 
 
 
 
 
```

V C++11 lze použít UTF-32, kde reprezentujeme japonské znaky "integery"

```cpp
// C++11 required
#include
<iostream>
#include
<string>

int main() {
    std::u32string a = U"来る";
    //std::cout << a << std::endl; // does not work, missing overload for <<
    for (int i = 0; i < a.size(); ++i) {
      std::cout << "[" << i << "]: " << a[i] << std::endl;
    }
    for (auto c : a) std::cout << c << std::endl;
}
```

```
[0]: 26469
[1]: 12427
26469
12427
```

Rust jako další zástupce low-level jazyka zakazuje indexování po bajtech, dovoluje pouze "substring" s pomocí range.
Vypsání substringů délky 1 ze stringu níže nefunguje:

```rust
fn main() {
  let s = "来る";
  println!("{}", s);

  // thread 'main' panicked at 'byte index 1 is not a char boundary; it is inside '来' (bytes 0..3) of `来る`', src/main.rs:5:30
  // for i in 0..s.len() {
  //   println!("[{}]: {}", i, &s[i..(i+1)]);
  // }

  for c in s.chars() {
    println!("{}", c);
  }
}
```

```
来る
来
る
```

V C# je char potenciálně více-bajtový, indexování stringu vrací UTF-8 skalární hodnoty

```csharp
var s = "来る";

Console.WriteLine(s);

for(var i = 0; i < s.Length; i++)
    Console.WriteLine($"[{i}]: {s[i]}");
foreach(char c in s)
    Console.WriteLine(c);
```

```
来る
[0]: 来
[1]: る
来
る
```

V Pythonu 3 taktéž

```python
a = "来る"
print(a)
for i in range(len(a)):
    print("[%d] %s:" % (i, a[i]))
for c in a:
    print(c)
```

```
来る
[0] 来:
[1] る:
来
る
```

Řešení: potřeby programu nezahrnují detekování "hranic skalárních hodnot", či "iterování po skalárních hodnotách". K
tomu by bylo možné použít [libutf8](https://github.com/m2osw/libutf8). Programu stačí najít hranici podle reference,
např. index, kde končí poslední výskyt は.

```cpp
std::string a = "他人を憎むことは彼女とは相入れない";
std::string needle = "は";
int needle_end = a.rfind(needle) + needle.size();
std::cout << a.substr(needle_end) << std::endl;
```

```
相入れない
```
