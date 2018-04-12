/**
 * Copyright (C) 2015-2018 Virgil Security Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 */

#ifndef VIRGIL_PYTHIA_GET_PASSWORD_UPDATE_TOKEN_RESULT_H
#define VIRGIL_PYTHIA_GET_PASSWORD_UPDATE_TOKEN_RESULT_H

#include "../VirgilByteArray.h"

namespace virgil {
namespace crypto {
namespace pythia {

/**
 * @brief Handles result of the method VirgilPythia::getPasswordUpdateToken().
 * @ingroup pythia
 */
class VirgilPythiaGetPasswordUpdateTokenResult {
public:
    /**
     * @brief Encapsulate given data.
     *
     * @param passwordUpdateToken - BN value that allows to update all deblinded passwords (one by one)
     *        after server issued new pythia_secret or pythia_scope_secret.
     * @param updatedTransformationPublicKey - G1 public key corresponding to the new
     *        transformation_private_key after issuing password_update_token.
     */
    explicit VirgilPythiaGetPasswordUpdateTokenResult(
            VirgilByteArray passwordUpdateToken, VirgilByteArray updatedTransformationPublicKey)
            : passwordUpdateToken_(std::move(passwordUpdateToken)),
              updatedTransformationPublicKey_(std::move(updatedTransformationPublicKey)) {
    }

    /**
     * @return BN value that allows to update all deblinded passwords (one by one)
     *         after server issued new pythia_secret or pythia_scope_secret.
     */
    const VirgilByteArray& passwordUpdateToken() const {
        return passwordUpdateToken_;
    }

    /**
     * @return G1 public key corresponding to the new
     *         transformation_private_key after issuing password_update_token.
     */
    const VirgilByteArray& updatedTransformationPublicKey() const {
        return updatedTransformationPublicKey_;
    }

private:
    const VirgilByteArray passwordUpdateToken_;
    const VirgilByteArray updatedTransformationPublicKey_;
};

} // namespace pythia
} // namespace crypto
} // namespace virgil

#endif /* VIRGIL_PYTHIA_GET_PASSWORD_UPDATE_TOKEN_RESULT_H */