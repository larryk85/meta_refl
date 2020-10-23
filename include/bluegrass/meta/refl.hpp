#pragma once

#include "function_traits.hpp"
#include "utility.hpp"
#include "preprocessor.hpp"

#include <array>
#include <string_view>
#include <tuple>
#include <utility>

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

   // tag used to define an invalid fields for field_types
   struct invalid_t {};

   template <typename T>
   [[deprecated]]
   constexpr static inline void t1() {}
   template <typename T>
   [[deprecated]]
   constexpr static inline void t2() {}

   namespace detail {
      BLUEGRASS_META_HAS_MEMBER_GENERATOR(valid, _bluegrass_meta_refl_valid);
      template <typename C>
      constexpr static inline bool has_member_valid_v = has_member_valid<C, void>::value;
      template <typename C>
      constexpr inline auto which_types() {
         t1<C>();
         if constexpr ( has_member_valid_v<C> ) {
            t2<C>();
            return flatten_parameters_t<&C::_bluegrass_meta_refl_fields>{};
         }else
            return invalid_t{};
      }

      template <typename T>
      constexpr inline std::size_t cardinality() {
         if constexpr (std::is_same_v<T, invalid_t>)
            return 0;
         else
            return std::tuple_size_v<T>;
      }

      template<typename... Args>
      constexpr inline auto va_args_count_helper(Args&&...) { return sizeof...(Args); }

      template <typename T, std::size_t>
      struct fwd_t { using type = T; };

      template <typename T, std::size_t...Is>
      constexpr inline auto produce_tuple(std::index_sequence<Is...>)
         -> std::tuple< typename fwd_t<T, Is>::type ...>;

      template <std::size_t N, typename T>
      constexpr inline auto homogeneous_field_types()
         -> decltype(produce_tuple<T>(std::make_index_sequence<N>{}));

      template <std::size_t Max, std::size_t N, typename T>
      constexpr inline std::size_t homogeneous_field_offset() {
         static_assert(N <= Max);
         return sizeof(T) * N;
      }
   } // ns bluegrass::meta::detail

   template <typename C, typename Derived>
   struct meta_object_base {
      constexpr static inline std::string_view this_name = type_name<C>();
      constexpr static inline auto get_this_name() { return this_name; }

      using types = invalid_t;
      template <std::size_t N>
      using type  = invalid_t;
      constexpr static inline std::size_t count = 0;
      constexpr static auto names = std::array<std::string_view, 0>{};

      constexpr static inline auto get_count() { return Derived::count; }
      constexpr static inline auto get_name(std::size_t n) { return Derived::names[n]; }
      constexpr static inline auto get_names() { return Derived::names; }

      template <std::size_t N, typename T>
      constexpr static inline auto& get(T&& t) {
         return Derived::template get<N>(std::forward<T>(t));
      }

      template <std::size_t N, typename T, typename F>
      constexpr inline static auto for_each_impl( T&& t, F&& f ) {
         if constexpr (N+1 == get_count())
            return f(get<N>(std::forward<T>(t)));
         else {
            f(get<N>(std::forward<T>(t)));
            return for_each_impl<N+1>(std::forward<T>(t), std::forward<F>(f));
         }
      }
      template <typename T, typename F>
      constexpr inline static void for_each( T&& t, F&& f ) {
         if constexpr (get_count() == 0)
            return;
         else
            return for_each_impl<0>(t, f);
      }
   };

#define VALID_CONCEPT(T, ...) std::enable_if_t<__VA_ARGS__ detail::has_member_valid_v<T>, int>

   template <typename C>
   struct meta_object : meta_object_base<C, meta_object<C>> {
      using this_t = meta_object<C>;
      using base_t = meta_object_base<C, this_t>;
      using base_t::this_name;
      using base_t::get_this_name;
      using base_t::for_each;
      using base_t::get_count;
      using base_t::get_name;
      using base_t::get_names;

      using types = flatten_parameters_t<&C::_bluegrass_meta_refl_fields>;
      template <std::size_t N>
      using type = std::tuple_element_t<N, types>;
      constexpr static inline std::size_t count = detail::cardinality<types>();
      constexpr static auto names = C::_bluegrass_meta_refl_field_names();

      template <std::size_t N>
      constexpr static inline auto& get(C& c) {
         using type = std::tuple_element_t<N, types>;
         return *reinterpret_cast<type*>(c.template _bluegrass_meta_refl_field_ptr<N>());
      }

      template <std::size_t N>
      constexpr static inline const auto& get(const C& c) {
         using type = std::tuple_element_t<N, types>;
         return *reinterpret_cast<type*>(c.template _bluegrass_meta_refl_field_ptr<N>());
      }
   };

   template <typename... Ts>
   struct meta_object<std::tuple<Ts...>> : meta_object_base<std::tuple<Ts...>, meta_object<std::tuple<Ts...>>> {
      using types = std::tuple<Ts...>;
      using this_t = meta_object<types>;
      using base_t = meta_object_base<types, this_t>;
      using base_t::this_name;
      using base_t::get_this_name;
      using base_t::for_each;
      using base_t::get_count;

      template <std::size_t N>
      using type = std::tuple_element_t<N, types>;

      constexpr static inline std::size_t count = std::tuple_size_v<types>;

      template <std::size_t N, typename T>
      constexpr static inline auto& get(T&& t) { return std::get<N>(t); }
   };

   namespace detail {
      template <typename... Ts>
      constexpr static inline bool is_tuple(std::tuple<Ts...>) { return true; }
      template <typename T>
      constexpr static inline bool is_tuple(T) { return false; }
      template <typename T>
      constexpr static inline auto get_meta_object()
         -> std::enable_if_t<has_member_valid_v<T> || is_tuple<T>(), meta_object<T>>;
   } // meta::refl::detail

   template <typename T>
   using meta_object_t = decltype(detail::get_meta_object<T>());

   template <typename T>
   constexpr static inline bool has_meta_object_v = detail::has_member_valid_v<T> || detail::is_tuple<T>();

}} // ns meta::refl

#undef VALID_CONCEPT

#define BLUEGRASS_META_ADDRESS( ignore, FIELD ) (void*)&FIELD
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
   inline auto _bluegrass_meta_refl_field_ptrs() const {                              \
      return std::array<void*, BLUEGRASS_META_VA_ARGS_SIZE(__VA_ARGS__)>              \
      {BLUEGRASS_META_FOREACH(BLUEGRASS_META_ADDRESS, "ignored", ##__VA_ARGS__)};     \
   }                                                                                  \
   template <std::size_t N>                                                           \
   inline void* _bluegrass_meta_refl_field_ptr() const {                              \
      return _bluegrass_meta_refl_field_ptrs()[N];                                    \
   }                                                                                  \
   constexpr inline static auto _bluegrass_meta_refl_field_names() {                  \
      return std::array<std::string_view, BLUEGRASS_META_VA_ARGS_SIZE(__VA_ARGS__)> { \
         BLUEGRASS_META_FOREACH(BLUEGRASS_META_PASS_STR, "ignored", ##__VA_ARGS__)    \
      };                                                                              \
   }

// EXPERIMENTAL macro to produce meta_object specializations for homogeneous structures
#define BLUEGRASS_HOM_META(_C, _FT, ...)                                        \
   namespace bluegrass { namespace meta {                                       \
      template <__VA_ARGS__>                                                    \
      struct meta_object<_C> : meta_object_base<_C, meta_object<_C>> {          \
         using base_t = meta_object_base<_C, meta_object<_C>>;                  \
         using base_t::this_name;                                               \
         using base_t::get_this_name;                                           \
         using base_t::for_each;                                                \
         using base_t::get_count;                                               \
         using base_t::get_name;                                                \
         using base_t::get_names;                                               \
         constexpr static inline std::size_t count = sizeof(_C) / sizeof(_FT);  \
         using types = decltype(detail::homogeneous_field_types<count, _FT>()); \
         template <std::size_t _N>                                              \
         using type = std::tuple_element_t<_N, types>;                          \
         template <std::size_t _N>                                              \
         constexpr static inline auto& get(_C& t) {                             \
            /* very gross and bad code is about to follow */                    \
            return *(reinterpret_cast<_FT*>(&t)+_N);                            \
         }                                                                      \
      };                                                                        \
   }}
