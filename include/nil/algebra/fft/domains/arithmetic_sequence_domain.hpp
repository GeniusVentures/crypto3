//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef ALGEBRA_FFT_ARITHMETIC_SEQUENCE_DOMAIN_HPP
#define ALGEBRA_FFT_ARITHMETIC_SEQUENCE_DOMAIN_HPP

#include <vector>

#include <nil/algebra/fft/evaluation_domain.hpp>
#include <nil/algebra/fft/domains/basic_radix2_domain_aux.hpp>
#include <nil/algebra/fft/polynomial_arithmetic/basis_change.hpp>

#ifdef MULTICORE
#include <omp.h>
#endif

namespace nil {
    namespace algebra {
        namespace fft {

            template<typename FieldType>
            class arithmetic_sequence_domain : public evaluation_domain<FieldType> {
            public:
                bool precomputation_sentinel;
                std::vector<std::vector<std::vector<FieldType>>> subproduct_tree;
                std::vector<FieldType> arithmetic_sequence;
                FieldType arithmetic_generator;

                void do_precomputation() {
                    compute_subproduct_tree(log2(this->m), this->subproduct_tree);

                    this->arithmetic_generator = FieldType::arithmetic_generator();

                    this->arithmetic_sequence = std::vector<FieldType>(this->m);
                    for (size_t i = 0; i < this->m; i++) {
                        this->arithmetic_sequence[i] = this->arithmetic_generator * FieldType(i);
                    }

                    this->precomputation_sentinel = 1;
                }

                arithmetic_sequence_domain(const size_t m) :
                    evaluation_domain<FieldType>(m) {
                    if (m <= 1)
                        throw std::invalid_argument("arithmetic(): expected m > 1");
                    if (FieldType::arithmetic_generator() == FieldType::zero())
                        throw std::invalid_argument(
                            "arithmetic(): expected FieldType::arithmetic_generator() != FieldType::zero()");

                    precomputation_sentinel = 0;
                }

                void FFT(std::vector<FieldType> &a) {
                    if (a.size() != this->m)
                        throw std::invalid_argument("arithmetic: expected a.size() == this->m");

                    if (!this->precomputation_sentinel)
                        do_precomputation();

                    /* Monomial to Newton */
                    monomial_to_newton_basis(a, this->subproduct_tree, this->m);

                    /* Newton to Evaluation */
                    std::vector<FieldType> S(this->m); /* i! * arithmetic_generator */
                    S[0] = FieldType::one();

                    FieldType factorial = FieldType::one();
                    for (size_t i = 1; i < this->m; i++) {
                        factorial *= FieldType(i);
                        S[i] = (factorial * this->arithmetic_generator).inverse();
                    }

                    _polynomial_multiplication(a, a, S);
                    a.resize(this->m);

#ifdef MULTICORE
#pragma omp parallel for
#endif
                    for (size_t i = 0; i < this->m; i++) {
                        a[i] *= S[i].inverse();
                    }
                }
                void iFFT(std::vector<FieldType> &a) {
                    if (a.size() != this->m)
                        throw std::invalid_argument("arithmetic: expected a.size() == this->m");

                    if (!this->precomputation_sentinel)
                        do_precomputation();

                    /* Interpolation to Newton */
                    std::vector<FieldType> S(this->m); /* i! * arithmetic_generator */
                    S[0] = FieldType::one();

                    std::vector<FieldType> W(this->m);
                    W[0] = a[0] * S[0];

                    FieldType factorial = FieldType::one();
                    for (size_t i = 1; i < this->m; i++) {
                        factorial *= FieldType(i);
                        S[i] = (factorial * this->arithmetic_generator).inverse();
                        W[i] = a[i] * S[i];
                        if (i % 2 == 1)
                            S[i] = -S[i];
                    }

                    _polynomial_multiplication(a, W, S);
                    a.resize(this->m);

                    /* Newton to Monomial */
                    newton_to_monomial_basis(a, this->subproduct_tree, this->m);
                }

                std::vector<FieldType> evaluate_all_lagrange_polynomials(const FieldType &t) {
                    /* Compute Lagrange polynomial of size m, with m+1 points (x_0, y_0), ... ,(x_m, y_m) */
                    /* Evaluate for x = t */
                    /* Return coeffs for each l_j(x) = (l / l_i[j]) * w[j] */

                    if (!this->precomputation_sentinel)
                        do_precomputation();

                    /**
                     * If t equals one of the arithmetic progression values,
                     * then output 1 at the right place, and 0 elsewhere.
                     */
                    for (size_t i = 0; i < this->m; ++i) {
                        if (this->arithmetic_sequence[i] == t)    // i.e., t equals this->arithmetic_sequence[i]
                        {
                            std::vector<FieldType> res(this->m, FieldType::zero());
                            res[i] = FieldType::one();
                            return res;
                        }
                    }

                    /**
                     * Otherwise, if t does not equal any of the arithmetic progression values,
                     * then compute each Lagrange coefficient.
                     */
                    std::vector<FieldType> l(this->m);
                    l[0] = t - this->arithmetic_sequence[0];

                    FieldType l_vanish = l[0];
                    FieldType g_vanish = FieldType::one();

                    for (size_t i = 1; i < this->m; i++) {
                        l[i] = t - this->arithmetic_sequence[i];
                        l_vanish *= l[i];
                        g_vanish *= -this->arithmetic_sequence[i];
                    }

                    std::vector<FieldType> w(this->m);
                    w[0] = g_vanish.inverse() * (this->arithmetic_generator ^ (this->m - 1));

                    l[0] = l_vanish * l[0].inverse() * w[0];
                    for (size_t i = 1; i < this->m; i++) {
                        FieldType num = this->arithmetic_sequence[i - 1] - this->arithmetic_sequence[this->m - 1];
                        w[i] = w[i - 1] * num * this->arithmetic_sequence[i].inverse();
                        l[i] = l_vanish * l[i].inverse() * w[i];
                    }

                    return l;
                }
                FieldType get_domain_element(const size_t idx) {
                    if (!this->precomputation_sentinel)
                        do_precomputation();

                    return this->arithmetic_sequence[idx];
                }
                FieldType compute_vanishing_polynomial(const FieldType &t) {
                    if (!this->precomputation_sentinel)
                        do_precomputation();

                    /* Notes: Z = prod_{i = 0 to m} (t - a[i]) */
                    FieldType Z = FieldType::one();
                    for (size_t i = 0; i < this->m; i++) {
                        Z *= (t - this->arithmetic_sequence[i]);
                    }
                    return Z;
                }
                void add_poly_Z(const FieldType &coeff, std::vector<FieldType> &H) {
                    if (H.size() != this->m + 1)
                        throw std::invalid_argument("arithmetic: expected H.size() == this->m+1");

                    if (!this->precomputation_sentinel)
                        do_precomputation();

                    std::vector<FieldType> x(2, FieldType::zero());
                    x[0] = -this->arithmetic_sequence[0];
                    x[1] = FieldType::one();

                    std::vector<FieldType> t(2, FieldType::zero());

                    for (size_t i = 1; i < this->m + 1; i++) {
                        t[0] = -this->arithmetic_sequence[i];
                        t[1] = FieldType::one();

                        _polynomial_multiplication(x, x, t);
                    }

#ifdef MULTICORE
#pragma omp parallel for
#endif
                    for (size_t i = 0; i < this->m + 1; i++) {
                        H[i] += (x[i] * coeff);
                    }
                }
                void divide_by_Z_on_coset(std::vector<FieldType> &P) {
                    const FieldType coset = this->arithmetic_generator; /* coset in arithmetic sequence? */
                    const FieldType Z_inverse_at_coset = this->compute_vanishing_polynomial(coset).inverse();
                    for (size_t i = 0; i < this->m; ++i) {
                        P[i] *= Z_inverse_at_coset;
                    }
                }
            };
        }    // namespace fft
    }        // namespace algebra
}    // namespace nil

#endif    // ALGEBRA_FFT_ARITHMETIC_SEQUENCE_DOMAIN_HPP
