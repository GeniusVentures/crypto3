//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nilfoundation.org>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_BASE64_CODEC_H_
#define CRYPTO3_BASE64_CODEC_H_

#include <iterator>
#include <string>

#include <nil/crypto3/codec/detail/base_policy.hpp>
#include <nil/crypto3/codec/detail/codec_modes.hpp>
#include <nil/crypto3/codec/detail/fixed_block_stream_processor.hpp>
#include <nil/crypto3/codec/detail/varlength_block_stream_processor.hpp>

#include <nil/crypto3/codec/codec_state.hpp>
#include <nil/crypto3/codec/accumulators/codec.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>

namespace nil {
    namespace crypto3 {
        namespace codec {
            namespace detail {
                template<bool>
                struct static_range;
            }

            /*!
             * @brief Base encoder finalizer functor
             * @tparam Version
             *
             * Base encoder finalizer
             *
             * @note Finalizer is implemented under assumption it gets applied to the byte storage
             */
            template<std::size_t Version, typename = detail::static_range<true>>
            struct base_encode_finalizer {
                typedef detail::base_policy<Version> policy_type;

                /*!
                 * @param input_remaining_bits Bits remaining unprocessed in block
                 */
                base_encode_finalizer(std::size_t) {

                }

                /*!
                 * @brief Base encoding padding function. Fills remaining empty bits with '='.
                 * @tparam T Input container type. Assumed to meet the requirements of Container,
                 * AllocatorAwareContainer and SequenceContainer concepts.
                 * @param t
                 */
                template<typename T>
                void operator()(T &t) {

                }
            };


            template<std::size_t Version>
            struct base_encode_finalizer<Version, detail::static_range<!(Version % 32)>> {
                typedef detail::base_policy<Version> policy_type;

                /*!
                 * @param input_remaining_bits Bits remaining unprocessed in block
                 */
                base_encode_finalizer(std::size_t input_remaining_bits = 0) : remaining_bits(input_remaining_bits) {

                }

                /*!
                 * @brief Base encoding padding function. Fills remaining empty bits with '='.
                 * @tparam T Input container type. Assumed to meet the requirements of Container,
                 * AllocatorAwareContainer and SequenceContainer concepts.
                 * @param t
                 */
                template<typename T>
                void operator()(T &t) {
                    for (typename T::iterator out = t.end() - 1; remaining_bits >=
                                                                 policy_type::padding_bits; remaining_bits -= policy_type::padding_block_bits) {
                        *out-- = '=';
                    }
                }

                std::size_t remaining_bits; ///< Bits remaining unprocessed in block
            };

            /*!
             * @brief Base decoder finalizer functor
             * @tparam Version
             *
             * Base decoder finalizer
             *
             * @note Finalizer is implemented under assumption it gets applied to the byte storage
             */
            template<std::size_t Version, typename = detail::static_range<true>>
            struct base_decode_finalizer {
                typedef detail::base_policy<Version> policy_type;

                base_decode_finalizer(std::size_t) {

                }

                /*!
                 * @brief Base decoder padding function. Fills remaining empty bits with '='.
                 * @tparam T Input container type. Assumed to meet the requirements of Container,
                 * AllocatorAwareContainer and SequenceContainer concepts.
                 * @param t
                 */
                template<typename T>
                void operator()(T &t) {

                }
            };

            template<std::size_t Version>
            struct base_decode_finalizer<Version, detail::static_range<!(Version % 32)>> {
                typedef detail::base_policy<Version> policy_type;

                /*!
                 * @param input_remaining_bits
                 */
                base_decode_finalizer(std::size_t input_remaining_bits = 0) : remaining_bits(input_remaining_bits) {

                }

                /*!
                 * @brief Base decoder padding function. Fills remaining empty bits with '='.
                 * @tparam T Input container type. Assumed to meet the requirements of Container,
                 * AllocatorAwareContainer and SequenceContainer concepts.
                 * @param t
                 */
                template<typename T>
                void operator()(T &t) {
                    int new_size = t.size();
                    for (typename T::iterator out = t.end() - 1; *out == '\0'; --out, --new_size) {
                    }
                    t.resize(new_size);
                }

                std::size_t remaining_bits;
            };

            /*!
             * @brief Base encoder
             * @tparam Version Base encoder version selector. Available values are: 32, 58, 64
             */
            template<std::size_t Version, typename = detail::static_range<true>>
            class base {
                typedef typename detail::base_policy<Version> policy_type;

            public:
                typedef base_encode_finalizer<Version> encoding_finalizer_type;
                typedef base_decode_finalizer<Version> decoding_finalizer_type;

                typedef typename detail::isomorphic_encoding_mode<base<Version>> stream_encoder_type;
                typedef typename detail::isomorphic_decoding_mode<base<Version>> stream_decoder_type;

                constexpr static const std::size_t encoded_value_bits = policy_type::encoded_value_bits;
                typedef typename policy_type::encoded_value_type encoded_value_type;

                constexpr static const std::size_t decoded_value_bits = policy_type::decoded_value_bits;
                typedef typename policy_type::decoded_value_type decoded_value_type;

                constexpr static const std::size_t encoded_block_values = policy_type::encoded_block_values;
                constexpr static const std::size_t encoded_block_bits = policy_type::encoded_block_bits;
                typedef typename policy_type::encoded_block_type encoded_block_type;

                constexpr static const std::size_t decoded_block_values = policy_type::decoded_block_values;
                constexpr static const std::size_t decoded_block_bits = policy_type::decoded_block_bits;
                typedef typename policy_type::decoded_block_type decoded_block_type;

                inline static encoded_block_type encode(const decoded_block_type &plaintext) {
                    return policy_type::encode_block(plaintext);
                }

                inline static decoded_block_type decode(const encoded_block_type &encoded) {
                    return policy_type::decode_block(encoded);
                }

                template<typename ProcessingMode, typename StateAccumulator, std::size_t ValueBits>
                struct stream_processor {
                    struct params_type {
                        typedef typename stream_endian::little_octet_big_bit endian_type;

                        constexpr static const std::size_t value_bits = ValueBits;
                        constexpr static const std::size_t length_bits = ProcessingMode::input_block_bits;
                    };

                    typedef varlength_block_stream_processor<ProcessingMode, StateAccumulator, params_type> type;
                };
            };

            template<std::size_t Version>
            class base<Version, detail::static_range<!(Version % 32)>> {
                typedef typename detail::base_policy<Version> policy_type;

            public:
                typedef base_encode_finalizer<Version> encoding_finalizer_type;
                typedef base_decode_finalizer<Version> decoding_finalizer_type;

                typedef typename detail::isomorphic_encoding_mode<base<Version>> stream_encoder_type;
                typedef typename detail::isomorphic_decoding_mode<base<Version>> stream_decoder_type;

                constexpr static const std::size_t encoded_value_bits = policy_type::encoded_value_bits;
                typedef typename policy_type::encoded_value_type encoded_value_type;

                constexpr static const std::size_t decoded_value_bits = policy_type::decoded_value_bits;
                typedef typename policy_type::decoded_value_type decoded_value_type;

                constexpr static const std::size_t encoded_block_values = policy_type::encoded_block_values;
                constexpr static const std::size_t encoded_block_bits = policy_type::encoded_block_bits;
                typedef typename policy_type::encoded_block_type encoded_block_type;

                constexpr static const std::size_t decoded_block_values = policy_type::decoded_block_values;
                constexpr static const std::size_t decoded_block_bits = policy_type::decoded_block_bits;
                typedef typename policy_type::decoded_block_type decoded_block_type;

                inline static encoded_block_type encode(const decoded_block_type &plaintext) {
                    return policy_type::encode_block(plaintext);
                }

                inline static decoded_block_type decode(const encoded_block_type &encoded) {
                    return policy_type::decode_block(encoded);
                }

                template<typename ProcessingMode, typename StateAccumulator, std::size_t ValueBits>
                struct stream_processor {
                    struct params_type {
                        typedef typename stream_endian::little_octet_big_bit endian_type;

                        constexpr static const std::size_t value_bits = ValueBits;
                        constexpr static const std::size_t length_bits = ProcessingMode::input_block_bits;
                    };

                    typedef fixed_block_stream_processor<ProcessingMode, StateAccumulator, params_type> type;
                };
            };

            /*!
             * @brief Type alias for base<32>
             */
            typedef base<32> base32;

            /*!
             * @brief Type alias for base<58>
             */
            typedef base<58> base58;

            /*!
             * @brief Type alias for base<64>
             */
            typedef base<64> base64;
        }
    }
}

#endif
