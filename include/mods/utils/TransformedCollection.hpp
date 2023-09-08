
#ifndef MODS_UTILS_TRANSFORMEDCOLLECTION_HPP
#define MODS_UTILS_TRANSFORMEDCOLLECTION_HPP

#include <type_traits>

namespace mods
{
   namespace utils
     {
        template<typename C, typename F>
          class TransformedCollection
          {
           private:
             class Iterator;
             
           public:
             using value_type = typename std::remove_reference<typename std::result_of<F(typename C::reference)>::type>::type;
             using reference = value_type&;
             using const_reference = const value_type&;
             using pointer = value_type*;
             using const_pointer = const value_type*;
             using iterator = Iterator;
             using const_iterator= Iterator;
             using reverse_iterator= std::reverse_iterator<iterator>;
             using const_reverse_iterator = std::reverse_iterator<const_iterator>;
             using difference_type = std::ptrdiff_t;
             using size_type = std::size_t;
             
             TransformedCollection(C& collection, F transform)
               : _collection(collection),
               _transform(std::move(transform))
                 {
                 }
             
             TransformedCollection() = delete;
             TransformedCollection(const TransformedCollection&) = delete;
             TransformedCollection(TransformedCollection&&) = delete;
             auto operator=(const TransformedCollection&) -> TransformedCollection& = delete;
             auto operator=(TransformedCollection&&) -> TransformedCollection& = delete;
             ~TransformedCollection() = default;
             
             auto begin() noexcept -> iterator
               {
                  return iterator(_collection.begin(), _transform);
               }
             
             auto end() noexcept -> iterator
               {
                  return iterator(_collection.end(), _transform);
               }
             
           private:
             C& _collection;
             F _transform;
             
             class Iterator
               {
                public:
                  using iterator_category = std::random_access_iterator_tag;
                  using difference_type = std::ptrdiff_t;
                  using value_type = TransformedCollection::value_type;
                  using pointer = value_type*;
                  using reference = value_type&;
                  
                  Iterator(const typename C::iterator& internalIt, F transform)
                    : _internalIt(internalIt),
                    _transform(std::move(transform))
                      {
                      }
                  
                  Iterator() = delete;
                  Iterator(const Iterator&) = default;
                  Iterator(Iterator&&) noexcept = default;
                  auto operator=(const Iterator&) -> Iterator& = delete;
                  auto operator=(Iterator&&) -> Iterator& = delete;
                  ~Iterator() = default;
                  
                  auto operator-(const Iterator& it) const -> difference_type
                    {
                       return _internalIt - it._internalIt;
                    }
                  
                  auto operator*() const -> reference
                    {
                       return _transform(*_internalIt);
                    }
                  
                  auto operator++() -> Iterator&
                    {
                       ++_internalIt;
                       return *this;
                    }
                  
                  auto operator!=(const Iterator& it)
                    {
                       return _internalIt != it._internalIt;
                    }
                  
                private:
                  typename C::iterator _internalIt;
                  F _transform;
               };
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_TRANSFORMEDCOLLECTION_HPP
