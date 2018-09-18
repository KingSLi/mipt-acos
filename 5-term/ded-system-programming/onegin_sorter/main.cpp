#include <iostream>
#include <sys/stat.h>
#include <algorithm>
#include <memory>
#include<sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <cstring>

struct TString {
    char* str_;
    size_t len_;
    TString() : str_(nullptr), len_(0) {}
    TString(char* str) : str_(str) {
        if (str_ != nullptr)
            len_ = strlen(str_);
        else
            len_ = 0;
    }
    inline size_t len() const {
        return len_;
    }
};

class FileBuffer{
public:
    char* buffer_;
    size_t countRows_ = 0;
    size_t length_ = 0;
    std::vector<TString> pointers_;
    std::vector<TString> original_;

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

        length_ = static_cast<size_t>(sb.st_size) + 1;
        for (int i = 0; i < length_; ++i) {
            if (buffer_[i] == '\n')
                buffer_[i] = '\0';
            if (buffer_[i] == '\0')
                ++countRows_;
        }

        pointers_.assign(countRows_, nullptr);
        pointers_[0] = TString(buffer_);
        for (int i = 0, count = 0; i < length_ - 1; ++i) {
            if (buffer_[i] == '\0')
                pointers_[++count] = TString(buffer_ + i + 1);
        }
        original_ = pointers_;
    }

    void writeStrings() const {
        for (auto str: pointers_) {
            printf("%s\n", str);
        }
        printf("\n");
    }

    void MakeOriginalText() {
        pointers_ = original_;
    }
};

class string_compare_normal{
public:
    bool operator()(const TString& a, const TString& b) {
        size_t len_a = a.len();
        size_t len_b = b.len();
        size_t iter_a = 0;
        size_t iter_b = 0;
        while (iter_a < len_a && iter_b < len_b) {
            if (!isalpha(a.str_[iter_a])) {
                ++iter_a;
                continue;
            }
            if (!isalpha(b.str_[iter_b])) {
                ++iter_b;
                continue;
            }

            if (tolower(a.str_[iter_a]) < tolower(b.str_[iter_b]))
                return true;
            else if (tolower(a.str_[iter_a]) > tolower(b.str_[iter_b]))
                return false;

            ++iter_a;
            ++iter_b;
        }
        return iter_b > 0;
    }
};

class string_compare_reversed{
public:
    bool operator()(const TString& a, const TString& b) {
        size_t iter_a = a.len() - 1;
        size_t iter_b = b.len() - 1;
        while (iter_a >= 0 && iter_b >= 0) {
            if (!isalpha(a.str_[iter_a])) {
                --iter_a;
                continue;
            }
            if (!isalpha(b.str_[iter_b])) {
                --iter_b;
                continue;
            }

            if (tolower(a.str_[iter_a]) < tolower(b.str_[iter_b]))
                return true;
            else if (tolower(a.str_[iter_a]) > tolower(b.str_[iter_b]))
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
    file.writeStrings();

    std::sort(file.pointers_.begin(), file.pointers_.end(), string_compare_reversed());
    file.writeStrings();

    file.MakeOriginalText();
    file.writeStrings();

    return 0;
}
