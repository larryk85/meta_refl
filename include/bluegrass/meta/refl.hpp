#pragma once

#include "function_traits.hpp"
#include "utility.hpp"
#include "preprocessor.hpp"

#include <array>
#include <string_view>
#include <tuple>
#include <utility>

/**
 * \file refl.hpp
 */

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

   template <typename... Bases>
   struct base_types_t {
      template <std::size_t N>
      using at_type = std::tuple_element_t<N, std::tuple<Bases...>>;
      template <typename T>
      using get_type = std::decay_t<decltype(std::get<T>(std::declval<std::tuple<Bases...>>()))>;
   };

   namespace detail {
      META_HAS_MEMBER_GENERATOR(valid, _meta_refl_valid);
      template <typename C>
      constexpr static inline bool has_member_valid_v = has_member_valid<C, void>::value;
      template <typename C>
      constexpr inline auto which_types() {
         if constexpr ( has_member_valid_v<C> ) {
            return flatten_parameters_t<&C::_meta_refl_fields>{};
         }else
            return invalid_t{};
      }

      template <class T, class=void>
      struct get_super_t {
         using type = T;
      };

      template <class T>
      struct get_super_t<T, std::void_t<typename T::super_t>> {
         using type = typename T::super_t;
      };

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

      template <typename T>
      T base_type(void(T::*)());

      template <typename T>
      constexpr inline auto get_base() -> std::enable_if_t<has_member_valid_v<T>, decltype(&T::base_type)>;

      template <typename T>
      constexpr inline auto get_base() -> std::enable_if_t<!has_member_valid_v<T>, T>;

      template <typename T>
      using base_t = decltype(get_base<T>());

      template <typename T>
      struct wrapper { using type = T; };
   } // ns bluegrass::meta::detail

   /**
    * \struct meta_object_mixin
    * Mixin to provide the type name and for_each() functions.
    */
   template <typename C>
   struct meta_object_mixin {
      using derived_t = C;
      constexpr static inline std::string_view this_name = type_name<typename derived_t::this_t>();

      template <std::size_t N, typename T, typename F>
      constexpr inline static auto for_each_impl( T&& t, F&& f ) {
         if constexpr (N+1 == derived_t::cardinality)
            return f(derived_t::template get<N>(std::forward<T>(t)));
         else {
            f(derived_t::template get<N>(std::forward<T>(t)));
            return for_each_impl<N+1>(std::forward<T>(t), std::forward<F>(f));
         }
      }

      template <typename T, typename F>
      constexpr inline static void for_each( T&& t, F&& f ) {
         if constexpr (derived_t::cardinality == 0)
            return;
         else
            return for_each_impl<0>(t, f);
      }
   };

   /**
    * \struct meta_object
    * A type used for reflection.
    * This provides accessors and reflectors for reflected types (used the #META_REFL(...) macro).
    *
    */
   template <typename C>
   struct meta_object : public meta_object_mixin<meta_object<C>> {
      using mixin_t = meta_object_mixin<meta_object<C>>;
      using mixin_t::for_each;
      using this_t = C;
      using super_t = typename detail::get_super_t<C>::type;
      using types = flatten_parameters_t<&C::_meta_refl_fields>;
      template <std::size_t N>
      using type = std::tuple_element_t<N, types>;
      constexpr static inline std::size_t cardinality = detail::cardinality<types>();
      constexpr static auto names = C::_meta_refl_field_names();

      template <std::size_t N>
      constexpr static inline auto& get(C& c) {
         using type = std::tuple_element_t<N, types>;
         return *reinterpret_cast<type*>(c.template _meta_refl_field_ptr<N>());
      }

      template <std::size_t N>
      constexpr static inline const auto& get(const C& c) {
         using type = std::tuple_element_t<N, types>;
         return *reinterpret_cast<type*>(c.template _meta_refl_field_ptr<N>());
      }

      template <typename T, typename F>
      constexpr inline static void for_each_full( T& t, F&& f ) {
         if constexpr (!std::is_same_v<super_t, C>)
            meta_object<super_t>::for_each_full(static_cast<super_t&>(t), f);
         for_each(t, f);
      }
   };

#define VALID_CONCEPT(T, ...) std::enable_if_t<__VA_ARGS__ detail::has_member_valid_v<T>, int>

   template <typename... Ts>
   struct meta_object<std::tuple<Ts...>> : meta_object_mixin<meta_object<std::tuple<Ts...>>> {
      using mixin_t = meta_object_mixin<meta_object<std::tuple<Ts...>>>;
      using this_t = std::tuple<Ts...>;;
      using types = this_t;
      using mixin_t::for_each;

      template <std::size_t N>
      using type = std::tuple_element_t<N, types>;

      constexpr static inline std::size_t cardinality = std::tuple_size_v<types>;

      template <std::size_t N>
      constexpr static inline auto& get(this_t& t) { return std::get<N>(t); }
      template <std::size_t N>
      constexpr static inline const auto& get(const this_t& t) { return std::get<N>(t); }
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

#define META_ADDRESS( ignore, FIELD ) (void*)&FIELD
#define META_DECLTYPE( ignore, FIELD ) decltype(FIELD)
#define META_PASS_STR( ignore, X ) #X

/**
 * @ingroup REFLECTION
 * @brief Macro to add relection members and accessors to be used by meta_object.\n
 * Variadic macro where each input parameter has the same form.
 * @param[in] unnamed: reference to class member.
 *
 * **Example**:
 * @code
 *  struct foo {
 *    int a = 0;
 *    float b = 0;
 *    META_REFL(a, b);
 * };
 * @endcode
 */
#define META_REFL(...)                                                      \
   public:                                                                  \
   constexpr inline auto& _meta_refl_get_this() { return *this; }           \
   void _meta_refl_valid(){}                                                \
   void _meta_refl_fields                                                   \
      ( META_FOREACH(META_DECLTYPE, "ignored", ##__VA_ARGS__) ){}           \
   inline auto _meta_refl_field_ptrs() const {                              \
      return std::array<void*, META_VA_ARGS_SIZE(__VA_ARGS__)>              \
      {META_FOREACH(META_ADDRESS, "ignored", ##__VA_ARGS__)};               \
   }                                                                        \
   template <std::size_t N>                                                 \
   inline void* _meta_refl_field_ptr() const {                              \
      return _meta_refl_field_ptrs()[N];                                    \
   }                                                                        \
   constexpr inline static auto _meta_refl_field_names() {                  \
      return std::array<std::string_view, META_VA_ARGS_SIZE(__VA_ARGS__)> { \
         META_FOREACH(META_PASS_STR, "ignored", ##__VA_ARGS__)              \
      };                                                                    \
   }
