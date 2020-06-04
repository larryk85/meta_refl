#pragma once

#include "function_traits.hpp"
#include "utility.hpp"
#include "preprocessor.hpp"

#include <array>
#include <string_view>
#include <tuple>

namespace bluegrass { namespace meta {
   template <typename T>
   constexpr inline std::string_view type_name() {
      constexpr std::string_view full_name = __PRETTY_FUNCTION__;
      constexpr auto start = full_name.find("T = ");
#ifdef __clang__
      constexpr auto end = full_name.find("]");
#elif __GNUC__
      constexpr auto end = full_name.find(";");
#else
#error "Currently only supporting Clang and GCC compilers"
#endif

      return full_name.substr(start+4, end - start - 4);
   }

   template <typename Type>
   struct field_info {
      constexpr field_info( std::string_view nm ) : name(nm) {}
      std::string_view name;
      using type = Type;
   };

   // tag used to define an invalid fields for field_types
   struct invalid_fields {};

   namespace detail {
      template <typename C>
      constexpr inline auto which_field_types() {
         if constexpr ( BLUEGRASS_HAS_MEMBER_TY(C, _bluegrass_meta_refl_valid) )
            return flatten_parameters_t<&C::_bluegrass_meta_refl_fields>{};
         else
            return invalid_fields{};
      }

      template <typename T>
      constexpr inline std::size_t fields_size() {
         if constexpr (std::is_same_v<T, invalid_fields>)
            return 0;
         else
            return std::tuple_size_v<T>;
      }

      template<typename... Args>
      constexpr inline auto va_args_count_helper(Args&&...) { return sizeof...(Args); }
   } // ns bluegrass::meta::detail

   template <class C>
   struct meta_object {
      constexpr static inline std::string_view name = type_name<C>();
      using field_types = decltype(detail::which_field_types<C>());
      template <std::size_t N>
      using field_type = std::tuple_element_t<N, field_types>;
      constexpr static inline std::size_t field_count = detail::fields_size<field_types>();
      constexpr static auto field_names = C::_bluegrass_meta_refl_field_names();

      constexpr static inline auto get_name() { return name; }
      constexpr static inline auto get_field_count() { return field_count; }
      constexpr static inline auto get_field_name(std::size_t n) { return field_names[n]; }
      constexpr static inline auto get_field_names() { return field_names; }
      template <std::size_t N, typename T>
      constexpr static inline auto& get_field(T&& t) {
         static_assert(std::is_same_v<std::decay_t<T>, C>, "get_field<N, T>(T), T should be the same type as C");
         using type = std::tuple_element_t<N, field_types>;
         return *reinterpret_cast<type*>(t._bluegrass_meta_refl_field_ptrs()[N]);
      }

      template <std::size_t N, typename T, typename F>
      constexpr inline static auto for_each_field_impl( T&& t, F&& f ) {
         if constexpr (N == field_count)
            return;
         else {
            f(get_field<N>(std::forward<T>(t)));
            for_each_field_impl<N+1>(std::forward<T>(t), std::forward<F>(f));
         }
      }

      template <typename T, typename F>
      constexpr inline static void for_each_field( T&& t, F&& f ) {
         for_each_field_impl<0>(t, f);
      }
   };
}} // ns bluegrass::meta

#define BLUEGRASS_META_ADDRESS( ignore, FIELD ) &FIELD
#define BLUEGRASS_META_DECLTYPE( ignore, FIELD ) decltype(FIELD)
#define BLUEGRASS_META_PASS_STR( ignore, X ) #X

#define BLUEGRASS_META_VA_ARGS_SIZE(...)                         \
   bluegrass::meta::detail::va_args_count_helper(                \
         BLUEGRASS_META_FOREACH(                                 \
            BLUEGRASS_META_PASS_STR, "ignored", ##__VA_ARGS__))

#define BLUEGRASS_META_REFL(...)                                                      \
   constexpr void _bluegrass_meta_refl_valid();                                       \
   void _bluegrass_meta_refl_fields                                                   \
      ( BLUEGRASS_META_FOREACH(BLUEGRASS_META_DECLTYPE, "ignored", ##__VA_ARGS__) ){} \
   inline auto& _bluegrass_meta_refl_field_ptrs() {                                   \
      static std::array<void*, BLUEGRASS_META_VA_ARGS_SIZE(__VA_ARGS__)> ptrs =       \
         {BLUEGRASS_META_FOREACH(BLUEGRASS_META_ADDRESS, "ignored", ##__VA_ARGS__)};  \
      return ptrs;                                                                    \
   }                                                                                  \
   constexpr inline static auto _bluegrass_meta_refl_field_names() {                  \
      return std::array<std::string_view, BLUEGRASS_META_VA_ARGS_SIZE(__VA_ARGS__)> { \
         BLUEGRASS_META_FOREACH(BLUEGRASS_META_PASS_STR, "ignored", ##__VA_ARGS__)    \
      };                                                                              \
   }