#include <iostream>
#include <sys/stat.h>
#include <algorithm>
#include <memory>
#include<sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <cstring>

class FileBuffer{
 public:
  char* buffer_;
  size_t countRows_ = 0;
  size_t length_;
  std::vector<char*> pointers_;

 public:
  FileBuffer(char* filename) {
    struct stat sb;
    if (stat(filename, &sb) == -1) {
      printf("ERROR: file stat failed\n");
      assert(1);
      return;
    }
    int file = open(filename, O_RDWR);
    buffer_ = static_cast<char*>(mmap(nullptr, sb.st_size + 1, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0));
    if (buffer_ == MAP_FAILED) {
      delete buffer_;
      printf("ERROR: file map failed\n");
      assert(1);
      return;
    }

    length_ = sb.st_size + 1;
    for (int i = 0; i < length_; ++i) {
      if (buffer_[i] == '\n')
        buffer_[i] = '\0';
      if (buffer_[i] == '\0')
        ++countRows_;
    }

    pointers_.assign(countRows_, nullptr);
    pointers_[0] = buffer_;
    for (int i = 0, count = 0; i < length_ - 1; ++i) {
      if (buffer_[i] == '\0')
        pointers_[++count] = buffer_ + i + 1;
    }
  }

  void writeStringsInSortedOrder() {
    for (auto str: pointers_) {
      printf("%s\n", str);
    }
    printf("\n");
  }

  void writeOriginal() {
    for (int i = 0; i < length_; ++i) {
      if (buffer_[i] == '\0')
        printf("\n");
      else
        printf("%c", buffer_[i]);
    }
    printf("\n");
  }
};

char lower(char a) {
  if (a <= 'Z' && a >= 'A')
    return a - ('A' - 'a');
  return a;
}

bool isNotLetter(char a) {
  return !((a >= 'a') && (a <= 'z')) && !((a >= 'A') && (a <= 'Z'));
}

class string_compare_normal{
 public:
  bool operator()(char* a, char* b) {
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    size_t iter_a = 0;
    size_t iter_b = 0;
    while (iter_a < len_a && iter_b < len_b) {
      if (isNotLetter(a[iter_a])) {
        iter_a++;
        continue;
      }
      if (isNotLetter(b[iter_b])) {
        iter_b++;
        continue;
      }

      if (lower(a[iter_a]) < lower(b[iter_b]))
        return true;
      else if (lower(a[iter_a]) > lower(b[iter_b]))
        return false;

      ++iter_a;
      ++iter_b;
    }
    return iter_b > 0;


  }
};

class string_compare_reversed{
 public:
  bool operator()(char* a, char* b) {
    size_t iter_a = strlen(a) - 1;
    size_t iter_b = strlen(b) - 1;
    while (iter_a >= 0 && iter_b >= 0) {
      if (isNotLetter(a[iter_a])) {
        --iter_a;
        continue;
      }
      if (isNotLetter(b[iter_b])) {
        iter_b++;
        continue;
      }

      if (lower(a[iter_a]) < lower(b[iter_b]))
        return true;
      else if (lower(a[iter_a]) > lower(b[iter_b]))
        return false;

      --iter_a;
      --iter_b;
    }
    return iter_b > 0;
  }
};


int main() {
  FileBuffer file("input.txt");

  std::sort(file.pointers_.begin(), file.pointers_.end(), string_compare_normal());
  file.writeStringsInSortedOrder();

  std::sort(file.pointers_.begin(), file.pointers_.end(), string_compare_reversed());
  file.writeStringsInSortedOrder();

  file.writeOriginal();
  return 0;
}