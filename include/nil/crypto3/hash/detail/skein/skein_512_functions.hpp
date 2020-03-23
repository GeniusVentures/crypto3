//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_SKEIN_512_FUNCTIONS_HPP
#define CRYPTO3_SKEIN_512_FUNCTIONS_HPP

#include <nil/crypto3/hash/detail/basic_functions.hpp>

namespace nil {
    namespace crypto3 {
        namespace hash {
            namespace detail {
                template<std::size_t DigestBits>
                struct skein_512_functions : public basic_functions<64> {

                };
            }
        }
    }
}

#endif //CRYPTO3_SKEIN_512_FUNCTIONS_HPP
