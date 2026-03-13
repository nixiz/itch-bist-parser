#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <execution>

using namespace std;

// fix mesaj tag'leri için kullanılacak parser sınıflarının deklerasyonları
// tek bir dosya içerisinde örnek uygulandığı için bu sınıfların yukarıda 
// deklare edilmesi gerekmektedir. düzgün bir hiyerarşide bu sınıflar kodlanırsa
// buna gerek kalmayacaktır.
struct date_time_t {};
struct legacy_any_tag_parser;
struct tag_1_parser;
struct tag_2_parser;
// ... 
// ... 
// ... 
struct tag_52_parser;

// bütün fix mesajlarının parse edildikten sonra yapılacağı işler için
// temel visitor pattern'i uygulanmış bu sınıf kullanılabilir.
// paket içerisinde gelen bütün mesajlar burada işlendiikten sonra 
// work_with_message() gibi bir metod üzerinden ne yapılmak isteniyorsa yapılabilir.
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

// eski tip parser burada kullanılmaya devam edilebilir. bu sayede 
// özelleştirilmemiş bir fix tag mesajı için gelen mesajın işlenmesi devam edecektir
struct legacy_any_tag_parser {
  bool parse(string_view message) { 
    std::cout << "parsing message: " << message << "\n";
    return false; 
  }
  void apply(FixMessageProcessor& fmp) {
    fmp.process_fix_tag(*this);
  }
};

// istenilen veya dönüştürülecek tiplere göre özelleştirilmiş parser sınıfları oluşturulabilir. bu sınıflar parse ve apply metodlarını kendilerine göre özelleştirebilirler.:
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

// tag numarasına göre özelleştirilmemiş parser sınıflar varsayılan olarak
// legacy, yavaş parser sınıfını kullanarak devam edebilirler. 
template <int tag>
struct tag_traits {
  using type = void;
  using parser = legacy_any_tag_parser;
};

template <>
struct tag_traits<1> {
  // her tag için belirlenmiş bir dönüşüm türü olduğu varsaylarak 
  // 1 tag'i için dönüş yapacak tipin int olması gerekiyorsa int olarak 
  // belirtilecektir.
  using type = int;
  using parser = tag_1_parser;
};

template <>
struct tag_traits<52> {
  // aynı şekilde 52 tag'i için date time sınıfı kullanılacaktır
  using type = date_time_t;
  using parser = tag_52_parser;
};

// fix mesajından çıkarılacak tag değerine göre özelleştirilmiş parser sınıflarının
// çağrılmasını sağlayan sınıf.
template <int tag = -1>
struct fix_tag_parser {
  using tag_type = typename tag_traits<tag>::type;
  using tag_parser = typename tag_traits<tag>::parser;

  static bool parse(string_view message) {
    // gereksinime göre parser sınıfı burada yaratılabilir veya 
    // yaratılmadan statik metoduna yönlendirilebilir.
    // aynı şekilde statik instance kullanıldığında parser sınıfını 
    // bir sonraki işleme hazırlamak da gerekebilir. 
    // 'parser.clean()' veya 'parser.prepare()' gibi..
    return parser.parse(message);
  }

  static void apply(FixMessageProcessor& fmp) {
    parser.apply(fmp);
  }

private:
  static inline tag_parser parser;
};

// stackoverflow'dan buldum kullanılabilir: https://stackoverflow.com/a/58048821
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

// aynı şekilde tek dosya üzerinde yazdığım için declare etmem gerekiyor
void on_fix_tag_received(FixMessageProcessor& fmp, string_view message);

// tcp'den veya bir yerden gelen fix mesajının alındığı metod:
void on_fix_message_received(
  const char* message, int len)
{
  // her gelen mesaj için fix taglerini toplayarak işlemek için hazır hale getiren bir sınıfımız olsun
  FixMessageProcessor processor;

  // cheksum hesapla ve mesajın doğruluğunu kontrol et:
  bool is_message_valid = true;
  if (!is_message_valid) return;

  // gelen mesajlar tag'ler halinde gruplayarak paralel veya seri bir şekilde
  // bir döngüde işle:
  auto fix_tags = split(message, '|');
  for_each(begin(fix_tags), end(fix_tags),
           [&processor](string_view tag) {
             // ve her tag parçası için parser metodumuzu çağıralım
             on_fix_tag_received(processor, tag);
           });
  // bütün mesajlar pars edildikten sonra gerekli olan işlemleri yapalım.
  processor.work_with_message();
}

void on_fix_tag_received(
  FixMessageProcessor& fmp,
  string_view message)
{
  auto message_parts = split(message, '=');
  int tag = std::stoi(message_parts.at(0).data());
  // burada gelen mesajın tag numarasına göre, ona özel parser metodlarını çağırıyoruz
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
