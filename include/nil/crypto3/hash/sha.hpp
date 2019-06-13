//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Nil Foundation
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nilfoundation.org>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_HASH_SHA_HPP
#define CRYPTO3_HASH_SHA_HPP

#include <nil/crypto3/block/shacal.hpp>

#include <nil/crypto3/hash/detail/davies_meyer_compressor.hpp>
#include <nil/crypto3/hash/detail/sha_policy.hpp>
#include <nil/crypto3/hash/detail/state_adder.hpp>
#include <nil/crypto3/hash/detail/merkle_damgard_construction.hpp>
#include <nil/crypto3/hash/detail/merkle_damgard_state_preprocessor.hpp>

namespace nil {
    namespace crypto3 {
        namespace hash {

            /*!
             * @brief SHA. Initial SHA hash construction. Not considered to b a
             * cryptographically secure primitive lately.
             *
             * @ingroup hash
             */
            class sha {
                typedef detail::sha_policy policy_type;
                typedef block::shacal block_cipher_type;
            public:
                typedef merkle_damgard_construction<stream_endian::big_octet_big_bit, policy_type::digest_bits,
                                                    policy_type::iv_generator,
                                                    davies_meyer_compressor<block_cipher_type,
                                                                            detail::state_adder>> block_hash_type_;
#ifdef CRYPTO3_HASH_NO_HIDE_INTERNAL_TYPES
                typedef block_hash_type_ block_hash_type;
#else
                struct block_hash_type : block_hash_type_ {
                };
#endif
                template<typename StateAccumulator, std::size_t ValueBits>
                struct stream_processor {
                    struct params_type {
                        typedef typename stream_endian::big_octet_big_bit endian;

                        constexpr static const std::size_t value_bits = ValueBits;
                        constexpr static const std::size_t length_bits = block_hash_type::word_bits * 2;
                    };
                    typedef merkle_damgard_state_preprocessor<block_hash_type, StateAccumulator, params_type> type_;
#ifdef CRYPTO3_HASH_NO_HIDE_INTERNAL_TYPES
                    typedef type_ type;
#else
                    struct type : type_ {
                    };
#endif
                };

                constexpr static const std::size_t digest_bits = policy_type::digest_bits;
                typedef block_hash_type::digest_type digest_type;
            };

            typedef sha sha0;

        }
    }
} // namespace nil

#endif // CRYPTO3_HASH_SHA_HPP
