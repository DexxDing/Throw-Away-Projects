#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <bitset>
#include <limits>

// ----------------------
// TODO: Overall refactor
// ----------------------

size_t popcount(size_t n) {
    std::bitset<std::numeric_limits<size_t>::digits> bits(n);
    return bits.count();
}


template<typename T, size_t dim>
struct nDimVector {
    using type = std::vector<typename nDimVector<T, dim - 1>::type>;

    static auto Generate(const std::vector<size_t>& args) {
        T head = args.front();
        std::vector<size_t> tail(args.begin() + 1, args.end());
        using vector = std::vector<typename nDimVector<T, dim - 1>::type>;
        return vector(head, nDimVector<T, dim - 1>::Generate(tail));
    }
};
template<typename T>
struct nDimVector<T, 0> {
    using type = T;

    static auto Generate(const std::vector<size_t>& args) {
        return 0;
    }
};

template<typename T, size_t dim>
using nDimVector_t = typename nDimVector<T, dim>::type;



template<typename Type, size_t Dimension>
struct FT3D {
    nDimVector_t<Type, Dimension> fenwickArray;

    explicit FT3D(const std::vector<size_t>& sizes) :
             fenwickArray(nDimVector<Type, Dimension>::Generate(sizes)) {}


    template<typename T, size_t dim>
    struct I {
        static void increment(const std::vector<size_t>& start,
                  const Type val, nDimVector_t<Type, dim>* vec) {
            for (size_t i = start[dim - 1]; i < vec->size(); i = i | (i + 1)) {
                I<T, dim - 1>::increment(start, val, &vec->at(i));
            }
        }
    };
    template<typename T>
    struct I<T, 0>{
        static void increment(const std::vector<size_t> start,
                              const T val, nDimVector_t<T, 0>* vec) {
            *vec += val;
        }
    };

    void increment(const std::vector<size_t>& start, const Type val) {
        return I<Type, Dimension>::increment(start, val, &fenwickArray);
    }


    template<typename T, size_t dim>
    struct query_h {
        static T Q(const std::vector<size_t>& start,
                   const nDimVector_t<T, dim>& vec) {
            Type res = 0;
            for (size_t i = start[dim - 1]; i < vec.size(); i = (i&(i+1))-1) {
                res += query_h<T, dim - 1>::Q(start, vec.at(i));
            }
            return res;
        }
    };
    template<typename T>
    struct query_h<T, 0> {
        static T Q(const std::vector<size_t>& start,
              const nDimVector_t<T, 0>& vec) {
            return vec;
        }
    };

    Type query(const std::vector<size_t>& start) {
        return query_h<Type, Dimension>::Q(start, fenwickArray);
    }


    Type get(const std::vector<Type>& from, const std::vector<Type>& to) {
        Type res = 0;

        std::vector<size_t> elems(Dimension);
        for (size_t i = 0; i < (1 << Dimension); ++i) {
            for (size_t j = 0; j < Dimension; ++j) {
                if (i & (1 << j)) {
                    elems[j] = from[j] - 1;
                } else {
                    elems[j] = to[j];
                }
            }

            if (popcount(i) & 1) {
                res -= query(elems);
            } else {
                res += query(elems);
            }
        }
        return res;
    }
};


template <typename T, size_t dim>
void solve() {
    std::ios_base::sync_with_stdio(false);
    size_t size, count;
    std::cin >> size >> count;
    FT3D<T, dim> fen(std::vector<size_t>(dim, size));

    std::string res = "";

    std::vector<size_t> pos(dim);
    std::vector<T> posfrom(dim);
    std::vector<T> posto(dim);

    for (size_t i = 0; i < count; ++i) {
        size_t buf;
        std::cin >> buf;

        if (buf == 1) {
            T val;
            for (auto& i : pos)
                std::cin >> i;
            std::cin >> val;

            fen.increment(pos, val);
        } else {
            for (auto& i : posfrom)
                std::cin >> i;
            for (auto& i : posto)
                std::cin >> i;

            res += std::to_string(fen.get(posfrom, posto)) + '\n';
        }
    }
    std::cout << res;
}


int main()
{
    solve<int, 3>();

    return 0;
}
