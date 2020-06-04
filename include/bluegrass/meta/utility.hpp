#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <utility>

namespace bluegrass { namespace meta {
   // helpers for std::visit
   template <class... Ts>
   struct overloaded : Ts... {
      using Ts::operator()...;
   };
   template <class... Ts>
   overloaded(Ts...)->overloaded<Ts...>;

   template <char... Str>
   struct ct_string {
      constexpr static const char value[] = {Str..., '\0'};
      constexpr static std::size_t size = sizeof...(Str)+1;
   };

   template <typename T>
   [[deprecated]]
   constexpr inline T check_type() { return T{}; } // you can this to print the type at compile type as part of the deprecation warning

}} // ns bluegrass::meta

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
   template <typename T, T... Str>
   constexpr inline bluegrass::meta::ct_string<Str...> operator""_cts() { return bluegrass::meta::ct_string<Str...>{}; }
#pragma clang diagnostic pop

