#include <cstddef>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

template <typename T>
class Deque {
 private:
  std::vector<T*> chuncks_;
  const size_t kChunckCap = 10;
  size_t start_chunck_ = 0;
  size_t end_chunck_ = 0;
  size_t start_index_ = 0;
  size_t end_index_ = 0;
  size_t size_ = 0;

 public:
  /////////////////////////////////////////////
  //////////////CONSTRUCTORS//////////////////
  ///////////////////////////////////////////

  Deque() = default;
  //////////////////////////////
  Deque(const Deque& other)
      : start_chunck_(other.start_chunck_),
        start_index_(other.start_index_),
        end_chunck_(other.end_chunck_),
        end_index_(other.end_index_),
        size_(other.size_) {
    chuncks_.resize(other.chuncks_.size(), nullptr);
    for (size_t i = 0; i < chuncks_.size(); ++i) {
      if (other.chuncks_[i] != nullptr) {
        chuncks_[i] = static_cast<T*>(operator new[](sizeof(T) * kChunckCap));
        for (size_t j = 0; j < kChunckCap; ++j) {
          chuncks_[i][j] = other.chuncks_[i][j];
        }
      }
    }
  }
  //////////////////////////////
  Deque(size_t count) : size_(count) {
    end_chunck_ = count / kChunckCap;
    end_index_ = count % kChunckCap;
    chuncks_.resize(count / kChunckCap + 1, nullptr);
    for (auto& chunck : chuncks_) {
      chunck = static_cast<T*>(operator new[](sizeof(T) * kChunckCap));
      if_def_constructible_add(chunck, count);
    }
  }

  //////////////////////////////
  Deque(size_t count, const T& value) : size_(count) {
    chuncks_.resize((count / kChunckCap) + 1, nullptr);
    end_chunck_ = count / kChunckCap;
    end_index_ = count % kChunckCap;
    try {
      for (auto& chunck : chuncks_) {
        chunck = static_cast<T*>(operator new[](sizeof(T) * kChunckCap));
        for (size_t j = 0; j < kChunckCap; ++j) {
          if (count != 0) {
            new (&chunck[j]) T(value);
            --count;
          }
        }
      }
    } catch (...) {
      del_all();
      throw 1;
    }
  }
  /////////////////////////////////////////////////
  //////////////////DESTRUCTOR/////////////////////
  /////////////////////////////////////////////////
  ~Deque() { del_all(); }
  /////////////////////////////////////////////////
  ///////////////////OPERATORS////////////////////
  ///////////////////////////////////////////////
  void operator=(const Deque& other) {
    Deque copy(other);
    std::swap(chuncks_, copy.chuncks_);
    std::swap(start_chunck_, copy.start_chunck_);
    std::swap(end_chunck_, copy.end_chunck_);
    std::swap(start_index_, copy.start_index_);
    std::swap(end_index_, copy.end_index_);
    std::swap(size_, copy.size_);
  }
  /////////////////////////////////////////////
  T& operator[](int index) {
    if (index < 0) {
      index += size_;
    }
    return chuncks_[start_chunck_ + ((index + start_index_) / kChunckCap)]
                   [(index + start_index_) % kChunckCap];
  }
  const T& operator[](int index) const {
    if (index < 0) {
      index += size_;
    }
    return chuncks_[start_chunck_ + ((index + start_index_) / kChunckCap)]
                   [(index + start_index_) % kChunckCap];
  }

  //////////////////////////////////////////////
  //////////////////METHODS/////////////////////
  /////////////////////////////////////////////
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  /////////////////////////////////////////////
  //////////////////ATS////////////////////////
  T& at(size_t index) {
    if (index >= size_) {
      throw std::out_of_range("ERROR: OUT OF RANGE");
    }
    return chuncks_[start_chunck_ + ((index + start_index_) / kChunckCap)]
                   [(index + start_index_) % kChunckCap];
  }

  const T& at(size_t index) const {
    if (index >= size_) {
      throw std::out_of_range("ERROR: OUT OF RANGE");
    }
    return chuncks_[start_chunck_ + ((index + start_index_) / kChunckCap)]
                   [(index + start_index_) % kChunckCap];
  }
  //////////////////////////////////////////////

  //////////////////////////////////////////////
  ///////////////PUSHS////////////////////////
  void push_back(const T& val) {
    if (end_chunck_ == chuncks_.size()) {
      resize();
    }
    new (&chuncks_[end_chunck_][end_index_]) T(val);
    size_++;
    end_chunck_ = start_chunck_ + (((size_) + start_index_) / kChunckCap);
    end_index_ = (size_ + start_index_) % kChunckCap;
  }

  void push_front(const T& val) {
    if (start_chunck_ == 0 && start_index_ == 0) {
      resize();
    }
    if (start_index_ == 0) {
      --start_chunck_;
      start_index_ = kChunckCap - 1;
    } else {
      --start_index_;
    }

    new (&chuncks_[start_chunck_][start_index_]) T(val);
    size_++;
  }
  /////////////////////////////////////////////
  ///////////////POPS////////////////////////
  void pop_back() {
    if (empty()) {
      return;
    }
    if (end_index_ > 0) {
      --end_index_;
    } else {
      --end_chunck_;
      end_index_ = kChunckCap - 1;
    }
    if_def_constructible_remove(chuncks_[end_chunck_][end_index_]);
    --size_;
  }

  void pop_front() {
    if (empty()) {
      return;
    }
    if (start_index_ < kChunckCap - 1) {
      ++start_index_;
    } else {
      ++start_chunck_;
      start_index_ = 0;
    }
    if_def_constructible_remove(chuncks_[end_chunck_][end_index_]);
    --size_;
  }

  /////////////////////////////////////////////
 private:
  ////////////METHODS////////////////////
  void del_all() {
    for (size_t i = 0; i < chuncks_.size(); ++i) {
      operator delete[](chuncks_[i]);
    }
  }
  ///////////////////////////////////////
  void if_def_constructible_add(T* chunck, size_t& count) {
    if constexpr (std::is_default_constructible_v<T> &&
                  std::is_constructible_v<T>) {
      for (size_t j = 0; j < kChunckCap; ++j) {
        if (count != 0) {
          new (&chunck[j]) T();
          --count;
        }
      }
    } else {
    }
  }
  /////////////////////////////////////////
  void if_def_constructible_remove(T& val) {
    if constexpr (std::is_default_constructible_v<T> &&
                  std::is_constructible_v<T>) {
      val.~T();
    } else {
    }
  }

  ////////////////////////////////////////
  void resize() {
    if (empty()) {
      chuncks_.resize(1,
                      static_cast<T*>(operator new[](sizeof(T) * kChunckCap)));
      return;
    }
    size_t old_size = chuncks_.size();
    chuncks_.resize(old_size * 3, nullptr);
    for (size_t i = 0; i < old_size; ++i) {
      chuncks_[i + old_size] = chuncks_[i];
      chuncks_[i] = static_cast<T*>(operator new[](sizeof(T) * kChunckCap));
      chuncks_[i + 2 * old_size] =
          static_cast<T*>(operator new[](sizeof(T) * kChunckCap));
    }
    start_chunck_ += old_size;
    end_chunck_ += old_size;
  }
  ///////////////////////////////////////

 public:
  /////////////////////////////////////////
  ////////////////////////////////////////
  ////////////ITERATOR///////////////////
  ///////////////////////////////////////
  ///////////////////////////////////////
  template <bool IsConst>
  class DequeIterator
      : public std::iterator<
            std::random_access_iterator_tag,
            typename std::conditional<IsConst, const T, T>::type> {
   private:
    Deque<T>* deque_ = nullptr;
    size_t index_ = 0;

   public:
    typedef typename std::conditional<IsConst, const T, T>::type Ttype;
    using value_type = typename std::iterator<std::random_access_iterator_tag,
                                              Ttype>::value_type;
    using pointer =
        typename std::iterator<std::random_access_iterator_tag, Ttype>::pointer;
    using difference_type =
        typename std::iterator<std::random_access_iterator_tag,
                               Ttype>::difference_type;
    using reference = typename std::iterator<std::random_access_iterator_tag,
                                             Ttype>::reference;
    /////DequeIterator constructors/////////////

    DequeIterator() = default;

    DequeIterator(Deque<T>* deque, int index) : deque_(deque), index_(index) {}

    DequeIterator(const DequeIterator<IsConst>& other)
        : deque_(other.deque_), index_(other.index_) {}

    ///////DequeIterator operators////////////////
    void operator=(const DequeIterator& other) {
      index_ = other.index_;
      deque_ = other.deque_;
    }
    /////////////////инкремент/////////////////
    DequeIterator<IsConst>& operator++() {
      ++index_;
      return *this;
    }

    DequeIterator<IsConst> operator++(int) {
      DequeIterator<IsConst> temp(*this);
      ++index_;
      return temp;
    }
    ////////////////декремент///////////////
    DequeIterator<IsConst>& operator--() {
      --index_;
      return *this;
    }

    DequeIterator<IsConst> operator--(int) {
      DequeIterator<IsConst> temp(*this);
      --index_;
      return temp;
    }
    /////////////////////////////////////////
    DequeIterator<IsConst>& operator+=(difference_type diff) {
      index_ += diff;
      return *this;
    }

    DequeIterator<IsConst>& operator-=(difference_type diff) {
      index_ -= diff;
      return *this;
    }
    //////////////////////////////////////////

    DequeIterator<IsConst> operator+(difference_type diff) const {
      DequeIterator<IsConst> temp(*this);
      temp.index_ += diff;
      return temp;
    }

    DequeIterator<IsConst> operator-(difference_type diff) const {
      DequeIterator<IsConst> temp(*this);
      temp.index_ -= diff;
      return temp;
    }
    ////////////////////////////////////////////

    difference_type operator-(const DequeIterator<IsConst>& iter) const {
      return index_ - iter.index_;
    }

    ////////////////////////////////////////////

    reference operator*() const { return (*deque_)[index_]; }
    pointer operator->() const { return &((*deque_)[index_]); }

    ////////////////////////////////////////////

    friend bool operator==(const DequeIterator<IsConst>& iter1,
                           const DequeIterator<IsConst>& iter2) {
      return iter1.deque_ == iter2.deque_ && iter1.index_ == iter2.index_;
    }

    friend bool operator>(const DequeIterator<IsConst>& iter1,
                          const DequeIterator<IsConst>& iter2) {
      return iter1.index_ > iter2.index_;
    }

    friend bool operator!=(const DequeIterator<IsConst>& iter1,
                           const DequeIterator<IsConst>& iter2) {
      return !(iter1 == iter2);
    }

    friend bool operator<(const DequeIterator<IsConst>& iter1,
                          const DequeIterator<IsConst>& iter2) {
      return (iter1 != iter2 && !(iter1 > iter2));
    }

    friend bool operator<=(const DequeIterator<IsConst>& iter1,
                           const DequeIterator<IsConst>& iter2) {
      return !(iter1 > iter2);
    }

    friend bool operator>=(const DequeIterator<IsConst>& iter1,
                           const DequeIterator<IsConst>& iter2) {
      return !(iter1 < iter2);
    }
  };

  ///////////ITERATOR USINGS////////////////////////////////////////////
  using iterator = DequeIterator<false>;
  using const_iterator = DequeIterator<true>;
  using reverse_iterator = std::reverse_iterator<DequeIterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<DequeIterator<true>>;

  ///////////////////////////////////////////////////////
  ///////////////////BEGINS & ENDS///////////////////////

  iterator begin() { return iterator(this, 0); }

  iterator end() { return iterator(this, size_); }

  const_iterator cbegin() { return const_iterator(this, 0); }

  const_iterator cend() { return const_iterator(this, size_); }

  reverse_iterator rbegin() noexcept {
    return reverse_iterator(iterator(this, size_));
  }

  reverse_iterator rend() noexcept {
    return reverse_iterator(iterator(this, 0));
  }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(const_iterator(this, size_));
  }
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(const_iterator(this, 0));
  }

  ///////////////////////////////////////////////////////
  ///////////////////INSERT & ERASE//////////////////////

  void insert(iterator place, const T& val) {
    size_t index = place - begin();
    push_back(val);
    for (size_t i = size() - 1; i > index; --i) {
      std::swap((*this)[i], (*this)[i - 1]);
    }
  }
  void erase(iterator place) {
    size_t index = place - begin();
    for (size_t i = index; i < size() - 1; ++i) {
      std::swap((*this)[i], (*this)[i + 1]);
    }
    pop_back();
  }
};