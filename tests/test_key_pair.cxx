/**
 * Copyright (C) 2015 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
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
 */

/**
 * @file test_key_pair.cxx
 * @brief Covers class VirgilKeyPair
 */

#include "catch.hpp"

#include <string>

#include <virgil/crypto/VirgilByteArray.h>
#include <virgil/crypto/VirgilByteArrayUtils.h>
#include <virgil/crypto/VirgilKeyPair.h>

using virgil::crypto::VirgilByteArray;
using virgil::crypto::VirgilByteArrayUtils;
using virgil::crypto::VirgilKeyPair;

static const char * const kPrivateKey =
    "-----BEGIN ENCRYPTED PRIVATE KEY-----\n"
    "MIIBKTA0BgoqhkiG9w0BDAEDMCYEIJ2CZ9XD79se4sWO8zaB8ooKkf1IR/cymmox\n"
    "NH0pe2zCAgIgAASB8HPqZNMejdzjsPeLJrLj1SXdES8FOUgWDbIhFLm/6G3leCNi\n"
    "/7scgIOwook/f5qEL3ydHobXcYrr5Ltlr5o5BsSBELBAJKoUKcWmu8Aub03v/wIe\n"
    "TNsVhxA/4mn5kgs6BwJp59oODv0YqpRAFsMQsXJaXjePVWpKLsDAooT8Wa0s5cfP\n"
    "tURNzUUQG7COakN4PF01MXgHYEsvc/ygXI/QUHIBPwBVV7bx3lIV1xDy5WCNgBfd\n"
    "EEd8luTaIzd15Y7ahooAA9K1WDPEhtq0gl8jG5vSbZ+BCaMNd43+Gksno4c9oBkZ\n"
    "sMaFiu8OBbyVfjhr9g==\n"
    "-----END ENCRYPTED PRIVATE KEY-----\n";

static const char * const kPrivateKeyPwd = "strong_pwd";

TEST_CASE("Reset Private Key password", "[key-pair]") {
    VirgilByteArray oldPwd = VirgilByteArrayUtils::stringToBytes(kPrivateKeyPwd);
    VirgilByteArray newPwd = VirgilByteArrayUtils::stringToBytes("new password");
    VirgilByteArray emptyPwd;

    VirgilByteArray initialPrivateKey = VirgilByteArrayUtils::stringToBytes(kPrivateKey);
    VirgilByteArray newPrivateKey = VirgilKeyPair::resetPrivateKeyPassword(initialPrivateKey, oldPwd, newPwd);
    VirgilByteArray oldPrivateKey = VirgilKeyPair::resetPrivateKeyPassword(newPrivateKey, newPwd, oldPwd);

    VirgilByteArray newPrivateKeyNoPassword =
            VirgilKeyPair::resetPrivateKeyPassword(newPrivateKey, newPwd, emptyPwd);

    VirgilByteArray oldPrivateKeyNoPassword =
            VirgilKeyPair::resetPrivateKeyPassword(oldPrivateKey, oldPwd, emptyPwd);

    REQUIRE(VirgilByteArrayUtils::bytesToString(newPrivateKeyNoPassword) ==
            VirgilByteArrayUtils::bytesToString(oldPrivateKeyNoPassword));
}
