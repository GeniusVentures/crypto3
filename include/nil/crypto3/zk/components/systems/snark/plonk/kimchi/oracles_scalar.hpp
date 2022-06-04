//---------------------------------------------------------------------------//
// Copyright (c) 2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_BLUEPRINT_PLONK_KIMCHI_ORACLES_SCALAR_COMPONENT_HPP
#define CRYPTO3_ZK_BLUEPRINT_PLONK_KIMCHI_ORACLES_SCALAR_COMPONENT_HPP

#include <nil/marshalling/algorithms/pack.hpp>

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/crypto3/zk/blueprint/plonk.hpp>
#include <nil/crypto3/zk/assignment/plonk.hpp>
#include <nil/crypto3/zk/component.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/element_powers.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/lagrange_denominators.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/public_evaluations.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/prev_chal_evals.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/combine_proof_evals.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/ft_eval.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/oracles_scalar/oracles_cip.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/verifier_index.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/transcript_fr.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/limbs.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/binding.hpp>
#include <nil/crypto3/zk/components/algebra/curves/pasta/plonk/endo_scalar.hpp>
#include <nil/crypto3/zk/components/algebra/fields/plonk/exponentiation.hpp>
#include <nil/crypto3/zk/components/algebra/fields/plonk/field_operations.hpp>

#include <nil/crypto3/zk/snark/systems/plonk/pickles/constants.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace components {

                template<typename ArithmetizationType, typename CurveType, typename KimchiParamsType, 
                    typename KimchiCommitmentParamsType, std::size_t... WireIndexes>
                class oracles_scalar;

                template<typename ArithmetizationParams, typename CurveType, typename KimchiParamsType,  
                         typename KimchiCommitmentParamsType, std::size_t W0, std::size_t W1,
                         std::size_t W2, std::size_t W3, std::size_t W4, std::size_t W5, std::size_t W6, std::size_t W7,
                         std::size_t W8, std::size_t W9, std::size_t W10, std::size_t W11, std::size_t W12,
                         std::size_t W13, std::size_t W14>
                class oracles_scalar<
                    snark::plonk_constraint_system<typename CurveType::scalar_field_type, ArithmetizationParams>,
                    CurveType, KimchiParamsType, KimchiCommitmentParamsType, 
                    W0, W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14> {

                    using BlueprintFieldType = typename CurveType::scalar_field_type;

                    typedef snark::plonk_constraint_system<BlueprintFieldType, ArithmetizationParams>
                        ArithmetizationType;

                    using var = snark::plonk_variable<BlueprintFieldType>;

                    constexpr static const std::size_t selector_seed = 0x0f08;

                    using endo_scalar_component =
                        zk::components::endo_scalar<ArithmetizationType, CurveType, W0, W1, W2, W3, W4, W5, W6, W7, W8,
                                                    W9, W10, W11, W12, W13, W14>;
                    using from_limbs = zk::components::from_limbs<ArithmetizationType, CurveType, W0, W1, W2>;

                    using exponentiation_component =
                        zk::components::exponentiation<ArithmetizationType, 60, W0, W1, W2, W3, W4, W5, W6, W7, W8, W9,
                                                       W10, W11, W12, W13, W14>;
                    using mul_component = zk::components::multiplication<ArithmetizationType, W0, W1, W2>;
                    
                    using alpha_powers_component = zk::components::element_powers<ArithmetizationType, KimchiParamsType::alpha_powers_n, 0, 1, 2, 3,
                                                                          4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14>;

                    using pi_powers_component = zk::components::element_powers<ArithmetizationType, KimchiParamsType::public_input_size, 0, 1, 2, 3,
                                                                          4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14>;

                    using lagrange_denominators_component =
                            zk::components::lagrange_denominators<ArithmetizationType, KimchiParamsType::public_input_size, W0, W1, W2, W3, W4,
                                                                    W5, W6, W7, W8, W9, W10, W11, W12, W13, W14>;

                    using public_eval_component =
                            zk::components::public_evaluations<ArithmetizationType, 
                                                        KimchiParamsType::public_input_size, W0, W1, W2, W3,
                                                        W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14>;

                    using prev_chal_evals_component =
                            zk::components::prev_chal_evals<ArithmetizationType, 
                                                        KimchiCommitmentParamsType, W0, W1, W2, W3,
                                                        W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14>;
                                        
                    using combined_proof_evals_component =
                            zk::components::combine_proof_evals<ArithmetizationType, 
                                                        KimchiParamsType, W0, W1, W2, W3,
                                                        W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14>;

                    using ft_eval_component =
                            zk::components::ft_eval<ArithmetizationType, CurveType,
                                                        KimchiParamsType, W0, W1, W2, W3,
                                                        W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14>; 
                                        
                    using cip_component = zk::components::oracles_cip<ArithmetizationType, KimchiCommitmentParamsType,
                                                        KimchiParamsType, W0, W1, W2, W3,
                                                        W4, W5, W6, W7, W8, W9, W10, W11, W12, W13, W14>; 
                    
                    using transcript_type = kimchi_transcript<ArithmetizationType, CurveType, KimchiParamsType,
                                        W0, W1, W2, W3, W4, W5, W6, W7, W8, W9, W10,
                                        W11, W12, W13, W14>;

                

                    using proof_binding = typename zk::components::binding<ArithmetizationType,
                        BlueprintFieldType, KimchiCommitmentParamsType>;

                    constexpr static const std::size_t eval_points_amount = 2;

                    constexpr static std::size_t rows() {
                        std::size_t row = 0;

                        // alpha
                        row += endo_scalar_component::rows_amount;
                        // zeta
                        row += endo_scalar_component::rows_amount;

                        //transcript.init_assignment(assignment, row);
                        row += transcript_type::init_rows;
                        row += transcript_type::absorb_rows;

                        // zeta_pow_n
                        row += exponentiation_component::rows_amount;

                        // zeta_omega
                        row += mul_component::rows_amount;

                        // zeta_omega_pow_n
                        row += exponentiation_component::rows_amount;

                        // alpha_powers
                        row += alpha_powers_component::rows_amount;

                        // omega_powers
                        row += pi_powers_component::rows_amount;

                        // lagrange_denominators
                        row += lagrange_denominators_component::rows_amount;

                        // TODO: check on empty public_input
                        // public_eval
                        row += public_eval_component::rows_amount;
                        
                        row += transcript_type::absorb_evaluations_rows;
                        row += transcript_type::absorb_evaluations_rows;

                        row += transcript_type::absorb_rows;

                        // v_challenge
                        row += transcript_type::challenge_rows;

                        row += endo_scalar_component::rows_amount;

                        // u_challenge
                        row += transcript_type::challenge_rows;
                        
                        row += endo_scalar_component::rows_amount;

                        // powers_of_eval_points_for_chunks
                        row += 2 * exponentiation_component::rows_amount;

                        // prev_challenges_evals
                        row += prev_chal_evals_component::rows_amount;

                        // combined_evals
                        for (std::size_t i = 0; i < eval_points_amount; i++) {
                            row += combined_proof_evals_component::rows_amount;
                        }

                        // ft_eval0
                        row += ft_eval_component::rows_amount;

                        //cip
                        row += cip_component::rows_amount;

                        return row;
                    }

                public:
                    constexpr static const std::size_t rows_amount = rows();
                    constexpr static const std::size_t gates_amount = 0;

                    struct params_type {
                        

                        kimchi_verifier_index_scalar<CurveType> &verifier_index;
                        kimchi_proof_scalar<BlueprintFieldType, KimchiParamsType,
                            KimchiCommitmentParamsType::eval_rounds> &proof;
                        typename proof_binding::fq_sponge_output &fq_output;

                        params_type(kimchi_verifier_index_scalar<CurveType> &_verifier_index,
                            kimchi_proof_scalar<BlueprintFieldType, KimchiParamsType,
                                KimchiCommitmentParamsType::eval_rounds> &_proof,
                            typename proof_binding::fq_sponge_output &_fq_output) : 
                                                        verifier_index(_verifier_index),
                                                        proof(_proof),
                                                        fq_output(_fq_output) {}
                    };

                    struct result_type {
                        struct random_oracles {
                            var alpha;
                            var zeta;
                            var v;
                            var u;
                            var v_chal;
                            var u_chal;
                        };

                        transcript_type transcript;
                        random_oracles oracles;
                        std::array<var, KimchiParamsType::alpha_powers_n> alpha_powers;
                        std::array<var, eval_points_amount> p_eval;
                        std::array<var, eval_points_amount> powers_of_eval_points_for_chunks;
                        std::array<var, KimchiCommitmentParamsType::eval_rounds> prev_challenges;
                        var zeta_pow_n;
                        var ft_eval0;
                        std::array<kimchi_proof_evaluations<BlueprintFieldType, KimchiParamsType>,
                            eval_points_amount> combined_evals;
                        var cip;
                        std::array<var, eval_points_amount> eval_points;
                    };

                    static result_type
                        generate_circuit(blueprint<ArithmetizationType> &bp,
                                         blueprint_public_assignment_table<ArithmetizationType> &assignment,
                                         const params_type &params,
                                         const std::size_t start_row_index) {
                        auto selector_iterator = assignment.find_selector(selector_seed);
                        std::size_t first_selector_index;

                        if (selector_iterator == assignment.selectors_end()) {
                            first_selector_index = assignment.allocate_selector(selector_seed, gates_amount);
                            generate_gates(bp, assignment, params, first_selector_index);
                        } else {
                            first_selector_index = selector_iterator->second;
                        }

                        generate_assignments_constant(bp, assignment, params, start_row_index);

                        std::size_t row = start_row_index;

                        var beta = params.fq_output.beta;
                        var gamma = params.fq_output.gamma;
                        var joint_combiner = params.fq_output.joint_combiner;

                        typename BlueprintFieldType::value_type endo_factor =
                            0x12CCCA834ACDBA712CAAD5DC57AAB1B01D1F8BD237AD31491DAD5EBDFDFE4AB9_cppui255;
                        std::size_t endo_num_bits = 128;
                        // alpha = phi(alpha_challenge)
                        var alpha = endo_scalar_component::generate_circuit(
                            bp, assignment, {params.fq_output.alpha, endo_factor, endo_num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;
                        // zeta = phi(zeta_challenge)
                        var zeta = endo_scalar_component::generate_circuit(
                            bp, assignment, {params.fq_output.zeta, endo_factor, endo_num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;

                        var zero = var(0, start_row_index + 4, false, var::column_type::constant);
                        var one = var(0, start_row_index + 5, false, var::column_type::constant);
                        var domain_size = var(0, start_row_index + 6, false, var::column_type::constant);
                        var max_poly_size = var(0, start_row_index + 7, false, var::column_type::constant);

                        // fr_transcript.absorb(fq_digest)
                        transcript_type transcript;
                        transcript.init_circuit(bp, assignment, zero, row);
                        row += transcript_type::init_rows;
                        transcript.absorb_circuit(bp, assignment, params.fq_output.fq_digest, row);
                        row += transcript_type::absorb_rows;

                        // zeta_pow_n = zeta**n
                        var zeta_pow_n = exponentiation_component::generate_circuit(
                                             bp, assignment,
                                             {zeta, domain_size, zero, one}, row)
                                             .output;
                        row += exponentiation_component::rows_amount;

                        var zeta_omega = zk::components::generate_circuit<mul_component>(bp, assignment,
                           {zeta, params.verifier_index.omega}, row).output;
                        row += mul_component::rows_amount;

                        var zeta_omega_pow_n = 
                            exponentiation_component::generate_circuit(bp, assignment, 
                            {zeta_omega, domain_size, zero, one}, row).output;
                        row += exponentiation_component::rows_amount;

                        std::array<var, KimchiParamsType::alpha_powers_n> alpha_powers =
                            alpha_powers_component::generate_circuit(bp, assignment, 
                            {alpha, one}, row).output;
                        row += alpha_powers_component::rows_amount;

                        std::array<var, KimchiParamsType::public_input_size> omega_powers =
                            pi_powers_component::generate_circuit(bp, assignment, 
                            {params.verifier_index.omega, one}, row).output;
                        row += pi_powers_component::rows_amount;

                        std::array<var, eval_points_amount * KimchiParamsType::public_input_size> lagrange_denominators =
                                            lagrange_denominators_component::generate_circuit(bp, assignment,
                                                {zeta, zeta_omega, omega_powers, one}, row).output;
                        row += lagrange_denominators_component::rows_amount;

                        // TODO: check on empty public_input
                        std::array<var, KimchiParamsType::public_input_size> pi = params.proof.public_input;
                        std::array<var, eval_points_amount> public_eval = public_eval_component::generate_circuit(bp,
                            assignment, {zeta_pow_n, zeta_omega_pow_n, 
                                        pi,
                                        lagrange_denominators, 
                                        omega_powers,
                                        domain_size, one, zero}, row).output;
                        row += public_eval_component::rows_amount;

                        transcript.absorb_evaluations_circuit(
                             bp, assignment, public_eval[0], params.proof.proof_evals[0], row);
                        row += transcript_type::absorb_evaluations_rows;
                        transcript.absorb_evaluations_circuit(
                            bp, assignment, public_eval[1], params.proof.proof_evals[1], row);
                        row += transcript_type::absorb_evaluations_rows;

                        transcript.absorb_circuit(bp, assignment, params.proof.ft_eval, row);
                        row += transcript_type::absorb_rows;

                        var v_challenge = transcript.challenge_circuit(bp, assignment, row);
                        row += transcript_type::challenge_rows;

                        var v = endo_scalar_component::generate_circuit(
                            bp, assignment, {v_challenge, endo_factor, endo_num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;

                        var u_challenge = transcript.challenge_circuit(bp, assignment, row);
                        row += transcript_type::challenge_rows;
                        var u = endo_scalar_component::generate_circuit(
                            bp, assignment, {u_challenge, endo_factor, endo_num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;


                        std::array<var, eval_points_amount> powers_of_eval_points_for_chunks;
                        powers_of_eval_points_for_chunks[0] = exponentiation_component::generate_circuit(
                                             bp, assignment,
                                             {zeta, max_poly_size, zero, one}, row)
                                             .output;
                        row += exponentiation_component::rows_amount;
                        powers_of_eval_points_for_chunks[1] = exponentiation_component::generate_circuit(
                                             bp, assignment,
                                             {zeta_omega, max_poly_size, zero, one}, row)
                                             .output;
                        row += exponentiation_component::rows_amount;

                        std::array<var, KimchiCommitmentParamsType::eval_rounds> prev_challenges =
                            params.proof.prev_challenges;
                        std::array<std::array<var, KimchiCommitmentParamsType::res_size>, eval_points_amount> 
                            prev_challenges_evals = 
                            prev_chal_evals_component::generate_circuit(bp, assignment,
                                {prev_challenges, 
                                {{zeta, zeta_omega}},
                                powers_of_eval_points_for_chunks, 
                                one, zero}, row).output;
                        row += prev_chal_evals_component::rows_amount;

                        std::array<kimchi_proof_evaluations<BlueprintFieldType, KimchiParamsType>,
                            eval_points_amount> combined_evals;
                        for (std::size_t i = 0; i < eval_points_amount; i++) {
                            combined_evals[i] = combined_proof_evals_component::generate_circuit(
                                bp, assignment, {params.proof.proof_evals[i], 
                                powers_of_eval_points_for_chunks[i]}, row).output;
                            row += combined_proof_evals_component::rows_amount;
                        }

                        std::array<kimchi_proof_evaluations<BlueprintFieldType, KimchiParamsType>,
                            eval_points_amount> p_evals = params.proof.proof_evals;
                        var ft_eval0 = ft_eval_component::generate_circuit(
                            bp,
                            assignment, 
                            {params.verifier_index,
                            zeta_pow_n,
                            alpha_powers,
                            combined_evals,
                            gamma,
                            beta,
                            p_evals,
                            zeta,
                            joint_combiner},
                            row
                        ).output;
                        row += ft_eval_component::rows_amount;

                        //cip
                        var cip = cip_component::generate_circuit(bp,
                            assignment,
                            {ft_eval0,
                            params.proof.ft_eval,
                            prev_challenges_evals,
                            public_eval,
                            params.proof.proof_evals},
                            row).output;
                        row += cip_component::rows_amount;

                        generate_copy_constraints(bp, assignment, params, start_row_index);
                        
                        typename result_type::random_oracles random_oracles = {
                            alpha,
                            zeta,
                            v,
                            u,
                            v_challenge,
                            u_challenge
                        };

                        return {
                            transcript,
                            random_oracles,
                            alpha_powers,
                            public_eval,
                            powers_of_eval_points_for_chunks,
                            prev_challenges,
                            zeta_pow_n,
                            ft_eval0,
                            combined_evals,
                            cip, 
                            {zeta, zeta_omega}
                        };
                    }

                    static result_type generate_assignments(blueprint_assignment_table<ArithmetizationType> &assignment,
                                                            const params_type &params,
                                                            std::size_t start_row_index) {

                        std::size_t row = start_row_index;

                        typename BlueprintFieldType::value_type endo_factor =
                            0x12CCCA834ACDBA712CAAD5DC57AAB1B01D1F8BD237AD31491DAD5EBDFDFE4AB9_cppui255;
                        std::size_t num_bits = 128;

                        var fq_digest = params.fq_output.fq_digest;
                        var beta = params.fq_output.beta;
                        var gamma = params.fq_output.gamma;
                        var joint_combiner = params.fq_output.joint_combiner;

                        var alpha = endo_scalar_component::generate_assignments(assignment,
                            {params.fq_output.alpha, endo_factor, num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;
                        std::cout << "alpha: " << assignment.var_value(alpha).data << std::endl;

                        var zeta = endo_scalar_component::generate_assignments(assignment,
                            {params.fq_output.zeta, endo_factor, num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;
                        std::cout << "zeta: " << assignment.var_value(zeta).data << std::endl;
                        std::cout << "params.fq_output.zeta: " << assignment.var_value(params.fq_output.zeta).data << std::endl;

                        var zero = var(0, start_row_index + 4, false, var::column_type::constant);
                        var one = var(0, start_row_index + 5, false, var::column_type::constant);
                        var domain_size = var(0, start_row_index + 6, false, var::column_type::constant);
                        var max_poly_size = var(0, start_row_index + 7, false, var::column_type::constant);

                        transcript_type transcript;
                        transcript.init_assignment(assignment, zero, row);
                        row += transcript_type::init_rows;
                        transcript.absorb_assignment(assignment, fq_digest, row);
                        row += transcript_type::absorb_rows;

                        var n = domain_size;
                        var zeta_pow_n = exponentiation_component::generate_assignments(
                            assignment, {zeta, n, zero, one}, row).output;
                        row += exponentiation_component::rows_amount;

                        var zeta_omega = mul_component::generate_assignments(assignment, {zeta, 
                            params.verifier_index.omega}, row).output;
                        row += mul_component::rows_amount;

                        var zeta_omega_pow_n = exponentiation_component::generate_assignments(
                            assignment, {zeta_omega, n, zero, one}, row).output;
                        row += exponentiation_component::rows_amount;

                        std::array<var, KimchiParamsType::alpha_powers_n> alpha_powers = alpha_powers_component::generate_assignments(
                            assignment, {alpha, one}, row).output;
                        row += alpha_powers_component::rows_amount;

                        std::array<var, KimchiParamsType::public_input_size> omega_powers =
                            pi_powers_component::generate_assignments(assignment, 
                            {params.verifier_index.omega, one}, row).output;
                        row += pi_powers_component::rows_amount;

                        std::array<var, eval_points_amount * KimchiParamsType::public_input_size> lagrange_denominators = 
                                    lagrange_denominators_component::generate_assignments(assignment,
                                    {zeta, zeta_omega, omega_powers, one}, row).output;
                        row += lagrange_denominators_component::rows_amount;

                        // TODO: check on empty public_input
                        std::array<var, KimchiParamsType::public_input_size> pi = params.proof.public_input;
                        std::array<var, eval_points_amount> public_eval = public_eval_component::generate_assignments(
                            assignment, {zeta_pow_n, zeta_omega_pow_n, 
                                        pi,
                                        lagrange_denominators, 
                                        omega_powers,
                                        n, one, zero}, row).output;
                        row += public_eval_component::rows_amount;
                        
                        transcript.absorb_evaluations_assignment(
                            assignment, public_eval[0], params.proof.proof_evals[0], row);
                        row += transcript_type::absorb_evaluations_rows;
                        transcript.absorb_evaluations_assignment(
                             assignment, public_eval[1], params.proof.proof_evals[1], row);
                        row += transcript_type::absorb_evaluations_rows;

                        transcript.absorb_assignment(assignment, params.proof.ft_eval, row);
                        row += transcript_type::absorb_rows;

                        var v_challenge = transcript.challenge_assignment(assignment, row);
                        row += transcript_type::challenge_rows;
                        var v = endo_scalar_component::generate_assignments(assignment,
                            {v_challenge, endo_factor, num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;

                        var u_challenge = transcript.challenge_assignment(assignment, row);
                        row += transcript_type::challenge_rows;
                        var u = endo_scalar_component::generate_assignments(assignment,
                            {u_challenge, endo_factor, num_bits}, row).output;
                        row += endo_scalar_component::rows_amount;

                        std::array<var, eval_points_amount> powers_of_eval_points_for_chunks = {
                            exponentiation_component::generate_assignments(
                                assignment, {zeta, max_poly_size, zero, one}, row).output,
                            exponentiation_component::generate_assignments(
                                assignment, {zeta_omega, max_poly_size, zero, one}, 
                                row + exponentiation_component::rows_amount).output
                        };
                        row += 2 * exponentiation_component::rows_amount;

                        std::array<var, KimchiCommitmentParamsType::eval_rounds> prev_challenges =
                            params.proof.prev_challenges;
                        std::array<std::array<var, KimchiCommitmentParamsType::res_size>, eval_points_amount> 
                            prev_challenges_evals = 
                            prev_chal_evals_component::generate_assignments(assignment,
                                {prev_challenges, 
                                {{zeta, zeta_omega}},
                                powers_of_eval_points_for_chunks, 
                                one, zero}, row).output;
                        row += prev_chal_evals_component::rows_amount;

                        std::array<kimchi_proof_evaluations<BlueprintFieldType, KimchiParamsType>,
                            eval_points_amount> combined_evals;
                        for (std::size_t i = 0; i < eval_points_amount; i++) {
                            combined_evals[i] = combined_proof_evals_component::generate_assignments(
                                assignment, {params.proof.proof_evals[i], 
                                powers_of_eval_points_for_chunks[i]}, row).output;
                            row += combined_proof_evals_component::rows_amount;
                        }

                        std::array<kimchi_proof_evaluations<BlueprintFieldType, KimchiParamsType>,
                            eval_points_amount> p_evals = params.proof.proof_evals;
                        var ft_eval0 = ft_eval_component::generate_assignments(
                            assignment, 
                            {params.verifier_index,
                            zeta_pow_n,
                            alpha_powers,
                            combined_evals,
                            gamma,
                            beta,
                            p_evals,
                            zeta,
                            joint_combiner},
                            row
                        ).output;
                        row += ft_eval_component::rows_amount;

                        //cip
                        var cip = cip_component::generate_assignments(
                            assignment,
                            {ft_eval0,
                            params.proof.ft_eval,
                            prev_challenges_evals,
                            public_eval,
                            params.proof.proof_evals},
                            row).output;
                        row += cip_component::rows_amount;

                        typename result_type::random_oracles random_oracles = {
                            alpha,
                            zeta,
                            v,
                            u,
                            v_challenge,
                            u_challenge
                        };

                        return {
                            transcript,
                            random_oracles,
                            alpha_powers,
                            public_eval,
                            powers_of_eval_points_for_chunks,
                            prev_challenges,
                            zeta_pow_n,
                            ft_eval0,
                            combined_evals,
                            cip,
                            {zeta, zeta_omega}
                        };
                    }

                private:
                    static void generate_gates(blueprint<ArithmetizationType> &bp,
                                               blueprint_public_assignment_table<ArithmetizationType> &assignment,
                                               const params_type &params,
                                               std::size_t component_start_row = 0) {
                    }

                    static void
                        generate_copy_constraints(blueprint<ArithmetizationType> &bp,
                                                  blueprint_public_assignment_table<ArithmetizationType> &assignment,
                                                  const params_type &params,
                                                  std::size_t component_start_row = 0) {
                        
                    }

                    static void
                        generate_assignments_constant(blueprint<ArithmetizationType> &bp,
                                                  blueprint_public_assignment_table<ArithmetizationType> &assignment,
                                                  const params_type &params,
                                                  std::size_t component_start_row) {
                            std::size_t row = component_start_row + 4;
                            assignment.constant(0)[row] = 0;
                            row++;
                            assignment.constant(0)[row] = 1;
                            row++;

                            assignment.constant(0)[row] = params.verifier_index.domain_size;
                            row++;
                            assignment.constant(0)[row] = KimchiCommitmentParamsType::max_poly_size;
                    }
                };
            }    // namespace components
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_BLUEPRINT_PLONK_KIMCHI_ORACLES_SCALAR_COMPONENT_HPP
