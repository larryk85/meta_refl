#include <bluegrass/meta/refl.hpp>

#include <iostream>
#include <string>

#include <catch2/catch.hpp>

using namespace bluegrass;
using namespace bluegrass::meta;

struct test_struct {
   int a;
   float b;
   std::string c;
   test_struct(int a, float b, std::string c)
      : a(a), b(b), c(c) {}
   static void print() {
      std::cout << "test_struct\n";
   }
   META_REFL(a, b, c);
};

struct test_struct2 : test_struct {
   using super_t = test_struct; //base_types_t<test_struct, std::true_type>;
   test_struct2(int a, float b, std::string c, int aa)
      : a(aa), test_struct(a,b,c) {}
   static void print() {
      std::cout << "test_struct2\n";
   }
   int a;
   META_REFL(a);
};

struct test_struct3 : test_struct2 {
  using super_t = test_struct2;
   test_struct3(int a, float b, std::string c, int aa, std::string s)
      : s(s), test_struct2(a,b,c, aa) {}
   static void print() {
      std::cout << "test_struct2\n";
   }
   std::string s;
   META_REFL(s);
};

void update(int& i) { i += 20; }
void update(float& f) { f += 20; }
void update(std::string& s) { s += " Hello"; }

TEST_CASE("Testing basic meta object", "[basic_meta_tests]") {
   using ts_meta = meta_object<test_struct>;

   using ts2_meta = meta_object<test_struct2>;
   using ts3_meta = meta_object<test_struct3>;

   //ts2_meta::super_t::get_type<test_struct>::print();
   ts2_meta::super_t::print();

   test_struct2 sss = { 42, 32.32f, "hello", 1001 };
   ts2_meta::for_each_full(sss, [](const auto& v) { std::cout << v << " " << std::endl; });

   test_struct3 ss3 = { 42, 32.24f, "hello", 1001, "world" };
   ts3_meta::for_each_full(ss3, [](const auto& v) { std::cout << v << " " << std::endl; });

   // !!! the require macro throws off the evaluation of
   // this and it will produce the wrong result because
   // of preprocessor voodoo
   constexpr auto name = ts_meta::this_name;
   REQUIRE( name == "test_struct" );
   REQUIRE( ts_meta::cardinality == 3 );
   constexpr auto names = ts_meta::names;
   REQUIRE( names.size() == 3 );
   REQUIRE( names[0] == "a" );
   REQUIRE( names[1] == "b" );
   REQUIRE( names[2] == "c" );

   test_struct ts = {42, 13.13, "Hello"};

   REQUIRE( std::is_same_v<ts_meta::type<0>, int> );
   REQUIRE( ts_meta::get<0>(ts) == 42 );
   ts_meta::get<0>(ts) += 3;
   REQUIRE( ts_meta::get<0>(ts) == 45 );

   REQUIRE( std::is_same_v<ts_meta::type<1>, float> );
   REQUIRE( ts_meta::get<1>(ts) == 13.13f );
   ts_meta::get<1>(ts) += 3.3;
   REQUIRE( ts_meta::get<1>(ts) == 16.43f );

   REQUIRE( std::is_same_v<ts_meta::type<2>, std::string> );
   REQUIRE( ts_meta::get<2>(ts) == "Hello" );
   ts_meta::get<2>(ts)[0] = 'f';
   REQUIRE( ts_meta::get<2>(ts) == "fello" );

   const auto& test_lam = [&](auto& f) { update(f); };

   ts_meta::for_each(ts, test_lam);

   REQUIRE( ts.a == 65 );
   REQUIRE( ts.b == 36.43f );
   REQUIRE( ts.c == "fello Hello" );
}

TEST_CASE("Testing tuple meta object", "[tuple_meta_tests]") {
   using tup_0 = std::tuple<int, float, std::string>;
   using meta_0 = meta_object<tup_0>;

   tup_0 t0 = {42, 42.42f, "4242"};
   constexpr auto name0 = meta_0::this_name;
   std::cout << "Name: " << name0 << "\n";
   if constexpr (is_windows_build)
      REQUIRE( name0 == "std::tuple<int, float, std::basic_string<char, std::char_traits<char>, std::allocator<char> > >" );
   else
      REQUIRE( name0 == "std::__1::tuple<int, float, std::__1::basic_string<char> >" );

   REQUIRE( std::is_same_v<meta_0::type<0>, int> );
   REQUIRE( std::is_same_v<meta_0::type<1>, float> );
   REQUIRE( std::is_same_v<meta_0::type<2>, std::string> );

   REQUIRE( meta_0::get<0>(t0) == 42 );
   REQUIRE( meta_0::get<1>(t0) == 42.42f );
   REQUIRE( meta_0::get<2>(t0) == "4242" );

   meta_0::get<0>(t0) = 13;
   meta_0::get<1>(t0) = 13.13f;
   meta_0::get<2>(t0) = "1313";

   REQUIRE( meta_0::get<0>(t0) == 13 );
   REQUIRE( meta_0::get<1>(t0) == 13.13f );
   REQUIRE( meta_0::get<2>(t0) == "1313" );

   int i = 30;
   const auto& test_lam = [&](auto& v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>)
         i += v.size();
      else
         i += v;
   };

   meta_0::for_each(t0, test_lam);

   REQUIRE( i == 60 );
}
