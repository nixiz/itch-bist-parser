#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <execution>

using namespace std;

// fix mesaj tag'leri iÁin kullan˝lacak parser s˝n˝flar˝n˝n deklerasyonlar˝
// tek bir dosya iÁerisinde ˆrnek uyguland˝˝ iÁin bu s˝n˝flar˝n yukar˝da 
// deklare edilmesi gerekmektedir. d¸zg¸n bir hiyerar˛de bu s˝n˝flar kodlan˝rsa
// buna gerek kalmayacakt˝r.
struct date_time_t {};
struct legacy_any_tag_parser;
struct tag_1_parser;
struct tag_2_parser;
// ... 
// ... 
// ... 
struct tag_52_parser;

// b¸t¸n fix mesajlar˝n˝n parse edildikten sonra yap˝laca˝ i˛ler iÁin
// temel visitor pattern'i uygulanm˝˛ bu s˝n˝f kullan˝labilir.
// paket iÁerisinde gelen b¸t¸n mesajlar burada i˛lendikten sonra 
// work_with_message() gibi bir metod ¸zerinden ne yap˝lmak isteniyorsa yap˝labilir.
class FixMessageProcessor {
public:
  void process_fix_tag(const legacy_any_tag_parser& parser) {}
  void process_fix_tag(const tag_1_parser& parser) {}
  void process_fix_tag(const tag_2_parser& parser) {}
  // ... 
  // ... 
  // ... 
  void process_fix_tag(const tag_52_parser& parser);
public:
  void work_with_message() { }
};

// eski tip parser burada kullan˝lmaya devam edilebilir. bu sayede 
// ˆzelle˛tirilmemi˛ bir fix tag mesaj˝ iÁin gelen mesaj˝n i˛lenmesi devam edecektir
struct legacy_any_tag_parser {
  bool parse(string_view message) { 
    std::cout << "parsing message: " << message << "\n";
    return false; 
  }
  void apply(FixMessageProcessor& fmp) {
    fmp.process_fix_tag(*this);
  }
};

// istenilen veya dˆn¸˛t¸r¸lecek t¸rlere gˆre ˆzelle˛tirilmi˛ parser s˝n˝flar˝:
struct tag_1_parser {
  bool parse(string_view message) { return false; }
  void apply(FixMessageProcessor& fmp) {
    fmp.process_fix_tag(*this);
  }
};
// ... 
// ... 
// ... 
struct tag_52_parser {
  bool parse(string_view message) { return true; }
  void apply(FixMessageProcessor& fmp) {
    fmp.process_fix_tag(*this);
  }
  date_time_t date_time;
};

void FixMessageProcessor::process_fix_tag(const tag_52_parser& parser) {
  date_time_t time = parser.date_time;
}

// tag numaras˝na gˆre ˆzelle˛tirilmemi˛ parser s˝n˝flar˝ varsay˝lan olarak
// legacy, yava˛ parser s˝n˝f˝n˝ kullanarak devam edebilirler. 
template <int tag>
struct tag_traits {
  using type = void;
  using parser = legacy_any_tag_parser;
};

template <>
struct tag_traits<1> {
  // her tag iÁin belirlenmi˛ bir dˆn¸˛¸m t¸r¸ olduu varsay˝larak 
  // 1 tag'i iÁin dˆn¸˛ yap˝lacak tipin int olmas˝ gerekiyorsa int olarak 
  // belirtilecektir.
  using type = int;
  using parser = tag_1_parser;
};

template <>
struct tag_traits<52> {
  // ayn˝ ˛ekilde 52 tag'i iÁin date time s˝n˝f˝ kullan˝lacakt˝r
  using type = date_time_t;
  using parser = tag_52_parser;
};

// fix mesaj˝ndan Á˝kar˝lacak tag deerine gˆre ˆzelle˛tirilmi˛ parser s˝n˝flar˝n
// Áar˝lmas˝n˝ salayan s˝n˝f.
template <int tag = -1>
struct fix_tag_parser {
  using tag_type = typename tag_traits<tag>::type;
  using tag_parser = typename tag_traits<tag>::parser;

  static bool parse(string_view message) {
    // gereksinime gˆre parser s˝n˝f˝ burada yarat˝labilir veya 
    // yarat˝lmadan statik metoduna yˆnlendirilebilir.
    // ayn˝ ˛ekilde statik instance kullan˝ld˝˝nda parser s˝n˝f˝n˝ 
    // bir sonraki i˛leme haz˝rlamak da gerekebilir. 
    // 'parser.clean()' veya 'parser.prepare()' gibi..
    return parser.parse(message);
  }

  static void apply(FixMessageProcessor& fmp) {
    parser.apply(fmp);
  }

private:
  static inline tag_parser parser;
};

// stackoverflow'dan buldum kullan˝labilir: https://stackoverflow.com/a/58048821
std::vector<std::string_view> split(const std::string_view str, const char delim = ',')
{
  std::vector<std::string_view> result;

  int indexCommaToLeftOfColumn = 0;
  int indexCommaToRightOfColumn = -1;

  for (int i = 0; i < static_cast<int>(str.size()); i++)
  {
    if (str[i] == delim)
    {
      indexCommaToLeftOfColumn = indexCommaToRightOfColumn;
      indexCommaToRightOfColumn = i;
      int index = indexCommaToLeftOfColumn + 1;
      int length = indexCommaToRightOfColumn - index;
      std::string_view column(str.data() + index, length);
      result.push_back(std::move(column));
    }
  }
  const std::string_view finalColumn(str.data() + indexCommaToRightOfColumn + 1, str.size() - indexCommaToRightOfColumn - 1);
  result.push_back(finalColumn);
  return result;
}

// ayn˝ ˛ekilde tek dosya ¸zerinde yazd˝˝m iÁin deklare etmem gerekiyor
void on_fix_tag_received(FixMessageProcessor& fmp, string_view message);

// tcp'den veya bir yerden gelen fix mesaj˝n˝n al˝nd˝˝ metod:
void on_fix_message_received(
  const char* message, int len)
{
  // her gelen mesaj iÁin fix taglerini toplayarak i˛lemek iÁin haz˝r hale getiren bir s˝n˝f˝m˝z olsun
  FixMessageProcessor processor;

  // cheksum hesapla ve mesaj˝n doruluunu kontrol et:
  bool is_message_valid = true;
  if (!is_message_valid) return;

  // gelen mesajlar˝ tag'ler halinde gruplayarak paralel veya seri bir ˛ekilde
  // bir dˆng¸de i˛le:
  auto fix_tags = split(message, '|');
  for_each(begin(fix_tags), end(fix_tags),
           [&processor](string_view tag) {
             // ve her tag parÁas˝n˝ iÁin parser metodumuzu Áa˝ral˝m
             on_fix_tag_received(processor, tag);
           });
  // b¸t¸n mesajlar pars edildikten sonra gerekli olan i˛lemleri yapal˝m.
  processor.work_with_message();
}

void on_fix_tag_received(
  FixMessageProcessor& fmp,
  string_view message)
{
  auto message_parts = split(message, '=');
  int tag = std::stoi(message_parts.at(0).data());
  // burada gelen mesaj˝n tag numaras˝na gˆre, ona ˆzel parser metodlar˝n˝ Áa˝r˝yoruz
  switch (tag)
  {
  case 1:
  {
    bool res = fix_tag_parser<1>::parse(message_parts.at(1));
    if (res) {
      fix_tag_parser<1>::apply(fmp);
    }
  }
  break;
  case 2:
  {
    bool res = fix_tag_parser<2>::parse(message_parts.at(1));
    if (res) {
      fix_tag_parser<2>::apply(fmp);
    }
  }
  break;
  case 52:
  {
    bool res = fix_tag_parser<52>::parse(message_parts.at(1));
    if (res) {
      fix_tag_parser<52>::apply(fmp);
    }
  }
  break;
  default:
  {
    bool res = fix_tag_parser<>::parse(message);
    if (res) {
      fix_tag_parser<>::apply(fmp);
    }
  }
  break;
  }
}

int main(int argc, const char* argv[]) {
  const char sample_msg[] = "8=FIX.4.2|9=196|35=X|49=A|56=B|34=12|52=20100318-03:21:11.364|262=A|268=2|279=0|269=0|278=BID|55=EUR/USD|270=1.37215|15=EUR|271=2500000|346=1|279=0|269=1|278=OFFER|55=EUR/USD|270=1.37224|15=EUR|271=2503200|346=1|10=171";
  on_fix_message_received(sample_msg, sizeof(sample_msg));
}
