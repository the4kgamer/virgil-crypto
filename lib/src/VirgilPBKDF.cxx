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

#include <virgil/crypto/foundation/VirgilPBKDF.h>

#include <tinyformat/tinyformat.h>
#include <mbedtls/oid.h>
#include <mbedtls/pkcs5.h>

#include <virgil/crypto/VirgilByteArrayUtils.h>
#include <virgil/crypto/foundation/VirgilSystemCryptoError.h>
#include <virgil/crypto/foundation/asn1/VirgilAsn1Reader.h>
#include <virgil/crypto/foundation/asn1/VirgilAsn1Writer.h>

#include "utils.h"
#include "mbedtls_context.h"

using virgil::crypto::VirgilByteArray;
using virgil::crypto::VirgilByteArrayUtils;
using virgil::crypto::VirgilCryptoException;

using virgil::crypto::foundation::VirgilPBKDF;
using virgil::crypto::foundation::VirgilHash;
using virgil::crypto::foundation::asn1::VirgilAsn1Compatible;
using virgil::crypto::foundation::asn1::VirgilAsn1Reader;
using virgil::crypto::foundation::asn1::VirgilAsn1Writer;

/**
 * @name Configuration constants
 */
///@{
static constexpr unsigned int kIterationCount_Min = 2048;
static constexpr VirgilPBKDF::Algorithm kAlgorithm_Default = VirgilPBKDF::Algorithm::PBKDF2;
static constexpr VirgilHash::Algorithm kHashAlgorithm_Default = VirgilHash::Algorithm::SHA384;
///@}

namespace virgil { namespace crypto { namespace foundation { namespace internal {

static mbedtls_md_type_t hash_to_md_type(VirgilHash::Algorithm hashAlgorithm) {
    switch (hashAlgorithm) {
        case VirgilHash::Algorithm::MD5: {
            return MBEDTLS_MD_MD5;
        }
        case VirgilHash::Algorithm::SHA1: {
            return MBEDTLS_MD_SHA1;
        }
        case VirgilHash::Algorithm::SHA224: {
            return MBEDTLS_MD_SHA224;
        }
        case VirgilHash::Algorithm::SHA256: {
            return MBEDTLS_MD_SHA256;
        }
        case VirgilHash::Algorithm::SHA384: {
            return MBEDTLS_MD_SHA384;
        }
        case VirgilHash::Algorithm::SHA512: {
            return MBEDTLS_MD_SHA512;
        }
    }
}

static VirgilHash::Algorithm md_type_to_hash(mbedtls_md_type_t md_type) {
    switch (md_type) {
        case MBEDTLS_MD_MD5: {
            return VirgilHash::Algorithm::MD5;
        }
        case MBEDTLS_MD_SHA1: {
            return VirgilHash::Algorithm::SHA1;
        }
        case MBEDTLS_MD_SHA224: {
            return VirgilHash::Algorithm::SHA224;
        }
        case MBEDTLS_MD_SHA256: {
            return VirgilHash::Algorithm::SHA256;
        }
        case MBEDTLS_MD_SHA384: {
            return VirgilHash::Algorithm::SHA384;
        }
        case MBEDTLS_MD_SHA512: {
            return VirgilHash::Algorithm::SHA512;
        }
        default: {
            throw make_error(VirgilCryptoError::UnsupportedAlgorithm);
        }
    }
}

}}}}

class VirgilPBKDF::Impl {
public:
    Impl() {}

    Impl(const VirgilByteArray& saltValue, unsigned int iterationCountValue) :
            salt(saltValue), iterationCount(iterationCountValue) {}

    VirgilByteArray salt;
    unsigned int iterationCount{ 0 };
    VirgilPBKDF::Algorithm algorithm{ kAlgorithm_Default };
    VirgilHash::Algorithm hashAlgorithm{ kHashAlgorithm_Default };
    unsigned int iterationCountMin{ kIterationCount_Min };
    bool checkRecommendations{ true };
};

VirgilPBKDF::VirgilPBKDF() : impl_(std::make_unique<Impl>()) {
}

VirgilPBKDF::VirgilPBKDF(const virgil::crypto::VirgilByteArray& salt, unsigned int iterationCount)
        : impl_(std::make_unique<Impl>(salt, iterationCount)) {
}

VirgilPBKDF::VirgilPBKDF(VirgilPBKDF&&) noexcept = default;

VirgilPBKDF& VirgilPBKDF::operator=(VirgilPBKDF&&) noexcept = default;

VirgilPBKDF::~VirgilPBKDF() noexcept = default;

VirgilByteArray VirgilPBKDF::getSalt() const {
    return impl_->salt;
}

unsigned int VirgilPBKDF::getIterationCount() const {
    return impl_->iterationCount;
}

void VirgilPBKDF::setAlgorithm(VirgilPBKDF::Algorithm alg) {
    impl_->algorithm = alg;
}

VirgilPBKDF::Algorithm VirgilPBKDF::getAlgorithm() const {
    return impl_->algorithm;
}

void VirgilPBKDF::setHashAlgorithm(VirgilHash::Algorithm hash) {
    impl_->hashAlgorithm = hash;
}

VirgilHash::Algorithm VirgilPBKDF::getHashAlgorithm() const {
    return impl_->hashAlgorithm;
}

void VirgilPBKDF::enableRecommendationsCheck() {
    impl_->checkRecommendations = true;
}

void VirgilPBKDF::disableRecommendationsCheck() {
    impl_->checkRecommendations = false;
}


VirgilByteArray VirgilPBKDF::derive(const virgil::crypto::VirgilByteArray& pwd, size_t outSize) {
    checkRecommendations(pwd);

    if (outSize > std::numeric_limits<unsigned int>::max()) {
        throw make_error(VirgilCryptoError::InvalidArgument, "Size of the output sequence is too big");
    }

    internal::mbedtls_context <mbedtls_md_context_t> hmac_ctx;
    hmac_ctx.setup(internal::hash_to_md_type(impl_->hashAlgorithm), 1);

    const unsigned int adjustedOutSize =
            (outSize > 0) ? static_cast<unsigned int>(outSize) : mbedtls_md_get_size(hmac_ctx.get()->md_info);

    VirgilByteArray result(adjustedOutSize);

    switch (impl_->algorithm) {
        case Algorithm::PBKDF2:
            system_crypto_handler(
                    mbedtls_pkcs5_pbkdf2_hmac(hmac_ctx.get(), pwd.data(), pwd.size(), impl_->salt.data(),
                            impl_->salt.size(), impl_->iterationCount, adjustedOutSize, result.data()),
                    [](int) { std::throw_with_nested(make_error(VirgilCryptoError::InvalidArgument)); }
            );
            break;
    }
    return result;
}

void VirgilPBKDF::checkRecommendations(const VirgilByteArray& pwd) const {
    if (!impl_->checkRecommendations) {
        return;
    }
    if (pwd.empty()) {
        throw make_error(VirgilCryptoError::NotSecure, "Empty password is not secure.");
    }
    if (impl_->salt.empty()) {
        throw make_error(VirgilCryptoError::NotSecure, "Empty salt is not secure.");
    }
    if (impl_->iterationCount < impl_->iterationCountMin) {
        throw make_error(VirgilCryptoError::NotSecure,
                tfm::format("Iteration count %s is not secure, minimum recommended value is %s.",
                        impl_->iterationCount, impl_->iterationCountMin));
    }
}

size_t VirgilPBKDF::asn1Write(VirgilAsn1Writer& asn1Writer, size_t childWrittenBytes) const {
    if (impl_->algorithm != Algorithm::PBKDF2) {
        throw make_error(VirgilCryptoError::UnsupportedAlgorithm);
    }

    size_t len = 0;
    const char* oid = 0;
    size_t oidLen;

    // Write prf
    system_crypto_handler(
            mbedtls_oid_get_oid_by_md(internal::hash_to_md_type(impl_->hashAlgorithm), &oid, &oidLen),
            [](int) { std::throw_with_nested(make_error(VirgilCryptoError::UnsupportedAlgorithm)); }
    );

    len += asn1Writer.writeOID(std::string(oid, oidLen));
    len += asn1Writer.writeSequence(len);

    len += asn1Writer.writeInteger(static_cast<int>(impl_->iterationCount));
    len += asn1Writer.writeOctetString(impl_->salt);
    len += asn1Writer.writeSequence(len);
    len += asn1Writer.writeOID(std::string(MBEDTLS_OID_PKCS5_PBKDF2, MBEDTLS_OID_SIZE(MBEDTLS_OID_PKCS5_PBKDF2)));
    len += asn1Writer.writeSequence(len);

    return len + childWrittenBytes;
}

void VirgilPBKDF::asn1Read(VirgilAsn1Reader& asn1Reader) {
    // Read key derivation function algorithm identifier
    asn1Reader.readSequence();
    VirgilByteArray oid = VirgilByteArrayUtils::stringToBytes(asn1Reader.readOID());
    mbedtls_asn1_buf oid_buf;
    oid_buf.p = oid.data();
    oid_buf.len = oid.size();

    if (MBEDTLS_OID_CMP(MBEDTLS_OID_PKCS5_PBKDF2, &oid_buf) != 0) {
        throw make_error(VirgilCryptoError::UnsupportedAlgorithm);
    }

    // Read PBKDF2-Params
    asn1Reader.readSequence();
    impl_->salt = asn1Reader.readOctetString();
    impl_->iterationCount = static_cast<unsigned int>(asn1Reader.readInteger());

    // Read PRF
    asn1Reader.readSequence();
    oid = VirgilByteArrayUtils::stringToBytes(asn1Reader.readOID());
    oid_buf.p = oid.data();
    oid_buf.len = oid.size();

    mbedtls_md_type_t md_type = MBEDTLS_MD_NONE;
    system_crypto_handler(
            mbedtls_oid_get_md_alg(&oid_buf, &md_type),
            [](int) { std::throw_with_nested(make_error(VirgilCryptoError::UnsupportedAlgorithm)); }
    );

    impl_->algorithm = Algorithm::PBKDF2;
    impl_->hashAlgorithm = internal::md_type_to_hash(md_type);
}
