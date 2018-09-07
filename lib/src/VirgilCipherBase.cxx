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

#include <virgil/crypto/VirgilCipherBase.h>

#include <virgil/crypto/VirgilByteArrayUtils.h>
#include <virgil/crypto/VirgilCryptoError.h>
#include <virgil/crypto/VirgilContentInfo.h>
#include <virgil/crypto/foundation/VirgilRandom.h>
#include <virgil/crypto/foundation/VirgilSymmetricCipher.h>
#include <virgil/crypto/foundation/VirgilAsymmetricCipher.h>
#include <virgil/crypto/foundation/VirgilPBE.h>

#include "utils.h"
#include "VirgilContentInfoFilter.h"

using virgil::crypto::VirgilByteArray;
using virgil::crypto::VirgilByteArrayUtils;
using virgil::crypto::VirgilCipherBase;
using virgil::crypto::VirgilCustomParams;
using virgil::crypto::VirgilCryptoError;
using virgil::crypto::VirgilContentInfo;
using virgil::crypto::make_error;

using virgil::crypto::foundation::VirgilRandom;
using virgil::crypto::foundation::VirgilSymmetricCipher;
using virgil::crypto::foundation::VirgilAsymmetricCipher;
using virgil::crypto::foundation::VirgilPBE;

using virgil::crypto::internal::VirgilContentInfoFilter;

namespace virgil { namespace crypto {

/**
 * @brief Handle class fields.
 */
class VirgilCipherBase::Impl {
public:
    Impl() noexcept :
            random(VirgilByteArrayUtils::stringToBytes(std::string("virgil::VirgilCipherBase"))),
            symmetricCipher(), symmetricCipherKey(), contentInfo(), contentInfoFilter(),
            recipientId(), privateKey(), pwd(), isInited(false) {}

public:
    VirgilRandom random;
    VirgilSymmetricCipher symmetricCipher;
    VirgilByteArray symmetricCipherKey;
    VirgilContentInfo contentInfo;
    VirgilContentInfoFilter contentInfoFilter;
    VirgilByteArray recipientId;
    VirgilByteArray privateKey;
    VirgilByteArray pwd;
    bool isInited;
};

}}

/**
 * @name Configuration constants.
 */
///@{
static constexpr VirgilSymmetricCipher::Padding
        kSymmetricCipher_Padding = VirgilSymmetricCipher::Padding::PKCS7;
static constexpr VirgilSymmetricCipher::Algorithm
        kSymmetricCipher_Algorithm = VirgilSymmetricCipher::Algorithm::AES_256_GCM;
///@}

VirgilCipherBase::VirgilCipherBase() : impl_(std::make_unique<Impl>()) {}

VirgilCipherBase::VirgilCipherBase(VirgilCipherBase&& rhs) noexcept = default;

VirgilCipherBase& VirgilCipherBase::operator=(VirgilCipherBase&& rhs) noexcept = default;

VirgilCipherBase::~VirgilCipherBase() noexcept = default;

void VirgilCipherBase::addKeyRecipient(const VirgilByteArray& recipientId, const VirgilByteArray& publicKey) {
    VirgilAsymmetricCipher::checkPublicKey(publicKey);
    impl_->contentInfo.addKeyRecipient(recipientId, publicKey);
}

void VirgilCipherBase::removeKeyRecipient(const VirgilByteArray& recipientId) {
    impl_->contentInfo.removeKeyRecipient(recipientId);
}

bool VirgilCipherBase::keyRecipientExists(const VirgilByteArray& recipientId) const {
    return impl_->contentInfo.hasKeyRecipient(recipientId);
}

void VirgilCipherBase::addPasswordRecipient(const VirgilByteArray& pwd) {
    impl_->contentInfo.addPasswordRecipient(pwd);
}

void VirgilCipherBase::removePasswordRecipient(const VirgilByteArray& pwd) {
    return impl_->contentInfo.removePasswordRecipient(pwd);
}

bool VirgilCipherBase::passwordRecipientExists(const VirgilByteArray& password) const {
    return impl_->contentInfo.hasPasswordRecipient(password);
}

void VirgilCipherBase::removeAllRecipients() {
    impl_->contentInfo.removeAllRecipients();
}

VirgilByteArray VirgilCipherBase::getContentInfo() const {
    return impl_->contentInfo.toAsn1();
}

void VirgilCipherBase::setContentInfo(const VirgilByteArray& contentInfo) {
    impl_->contentInfo.fromAsn1(contentInfo);
}

VirgilCustomParams& VirgilCipherBase::customParams() {
    return impl_->contentInfo.customParams();
}

const VirgilCustomParams& VirgilCipherBase::customParams() const {
    return impl_->contentInfo.customParams();
}

size_t VirgilCipherBase::defineContentInfoSize(const VirgilByteArray& data) {
    return VirgilContentInfo::defineSize(data);
}

VirgilByteArray VirgilCipherBase::computeShared(
        const VirgilByteArray& publicKey, const VirgilByteArray& privateKey,
        const VirgilByteArray& privateKeyPassword) {

    VirgilAsymmetricCipher publicContext;
    VirgilAsymmetricCipher privateContext;
    publicContext.setPublicKey(publicKey);
    privateContext.setPrivateKey(privateKey, privateKeyPassword);
    return VirgilAsymmetricCipher::computeShared(publicContext, privateContext);
}


VirgilByteArray VirgilCipherBase::filterAndSetupContentInfo(const VirgilByteArray& encryptedData, bool isLastChunk) {

    size_t encryptedDataSize = encryptedData.size();

    if (impl_->contentInfoFilter.isDone()) {
        return encryptedData;
    }

    if (impl_->contentInfoFilter.isWaitingData()) {
        impl_->contentInfoFilter.filterData(encryptedData);
    }

    if (isLastChunk) {
        impl_->contentInfoFilter.tellLastChunk();
    }

    if (impl_->contentInfoFilter.isContentInfoAbsent()) {
        impl_->contentInfoFilter.finish();
        accomplishInitDecryption();
        return impl_->contentInfoFilter.popEncryptedData();

    } else if (impl_->contentInfoFilter.isContentInfoFound()) {
        setContentInfo(impl_->contentInfoFilter.popContentInfo());
        impl_->contentInfoFilter.finish();
        accomplishInitDecryption();
        return impl_->contentInfoFilter.popEncryptedData();

    } else if (impl_->contentInfoFilter.isContentInfoBroken()) {
        throw make_error(VirgilCryptoError::InvalidArgument,
            "Content Info extracted from the encrypted data is broken.");

    } else if (isLastChunk) {
        accomplishInitDecryption();
    }

    //  Still waiting for data to be filtered.
    return VirgilByteArray();
}


void VirgilCipherBase::initEncryption() {

    impl_->symmetricCipher = VirgilSymmetricCipher(kSymmetricCipher_Algorithm);
    impl_->symmetricCipherKey = impl_->random.randomize(impl_->symmetricCipher.keyLength());
    auto symmetricCipherIV = impl_->random.randomize(impl_->symmetricCipher.ivSize());
    impl_->symmetricCipher.setEncryptionKey(impl_->symmetricCipherKey);
    impl_->symmetricCipher.setIV(symmetricCipherIV);

    if (impl_->symmetricCipher.isSupportPadding()) {
        impl_->symmetricCipher.setPadding(kSymmetricCipher_Padding);
    }

    impl_->symmetricCipher.reset();
    impl_->isInited = true;
}


void VirgilCipherBase::accomplishInitDecryption() {
    VirgilByteArray contentEncryptionKey;

    if (!impl_->contentInfo.isReadyForDecryption()) {
        throw make_error(VirgilCryptoError::InvalidState,
            "Content info is absent. It can be provided manually,"
            " or extracted as a part of encrypted data if it was embedded during encryption.");
    }

    if (impl_->recipientId.empty()) {
        // Password decryption.
        contentEncryptionKey = impl_->contentInfo.decryptPasswordRecipient(
                [&, this](
                        const VirgilByteArray& keyEncryptionAlgorithm,
                        const VirgilByteArray& encryptedKey) -> VirgilByteArray {
                    return doDecryptWithPassword(encryptedKey, keyEncryptionAlgorithm, impl_->pwd);
                }
        );

        if (contentEncryptionKey.empty()) {
            throw make_error(VirgilCryptoError::NotFoundPasswordRecipient);
        }

    } else {
        // Key decryption.
        contentEncryptionKey = impl_->contentInfo.decryptKeyRecipient(
                impl_->recipientId,
                [&, this](const VirgilByteArray& algorithm, const VirgilByteArray& encryptedKey) -> VirgilByteArray {
                    return doDecryptWithKey(algorithm, encryptedKey, impl_->privateKey, impl_->pwd);
                }
        );

        if (contentEncryptionKey.empty()) {
            throw make_error(VirgilCryptoError::NotFoundKeyRecipient);
        }
    }

    impl_->symmetricCipher = VirgilSymmetricCipher();
    impl_->symmetricCipher.fromAsn1(impl_->contentInfo.getContentEncryptionAlgorithm());
    impl_->symmetricCipher.setDecryptionKey(contentEncryptionKey);

    if (impl_->symmetricCipher.isSupportPadding()) {
        impl_->symmetricCipher.setPadding(kSymmetricCipher_Padding);
    }

    impl_->symmetricCipher.reset();
}


void VirgilCipherBase::initDecryptionWithPassword(const VirgilByteArray& pwd) {
    if (pwd.empty()) {
        throw make_error(VirgilCryptoError::InvalidArgument, "Can not decrypt with empty 'pwd'");
    }

    impl_->pwd = pwd;
    impl_->isInited = true;
}


void VirgilCipherBase::initDecryptionWithKey(
        const VirgilByteArray& recipientId,
        const VirgilByteArray& privateKey, const VirgilByteArray& privateKeyPassword) {

    if (recipientId.empty()) {
        throw make_error(VirgilCryptoError::InvalidArgument, "Can not decrypt with empty 'recipientId'");
    }

    if (privateKey.empty()) {
        throw make_error(VirgilCryptoError::InvalidArgument, "Can not decrypt with empty 'privateKey'");
    }

    impl_->recipientId = recipientId;
    impl_->privateKey = privateKey;
    impl_->pwd = privateKeyPassword;
    impl_->isInited = true;
}


void VirgilCipherBase::buildContentInfo() {
    const auto& symmetricCipherKey = impl_->symmetricCipherKey;
    auto& random = impl_->random;

    impl_->contentInfo.encryptKeyRecipients(
            [&symmetricCipherKey](const VirgilByteArray& publicKey) -> VirgilContentInfo::EncryptionResult {
                VirgilAsymmetricCipher asymmetricCipher;
                asymmetricCipher.setPublicKey(publicKey);
                return { asymmetricCipher.toAsn1(), asymmetricCipher.encrypt(symmetricCipherKey) };
            }
    );

    impl_->contentInfo.encryptPasswordRecipients(
            [&symmetricCipherKey, &random](const VirgilByteArray& password) -> VirgilContentInfo::EncryptionResult {
                const VirgilByteArray salt = random.randomize(16);
                const size_t iterationCount = random.randomize(3072, 8192);

                VirgilPBE pbe(VirgilPBE::Algorithm::PKCS5, salt, iterationCount);

                return { pbe.toAsn1(), pbe.encrypt(symmetricCipherKey, password) };
            }
    );

    impl_->contentInfo.setContentEncryptionAlgorithm(impl_->symmetricCipher.toAsn1());
}

void VirgilCipherBase::clear() {
    impl_->isInited = false;
    impl_->symmetricCipher.clear();
    impl_->recipientId.clear();
    impl_->contentInfoFilter.reset();

    VirgilByteArrayUtils::zeroize(impl_->symmetricCipherKey);
    VirgilByteArrayUtils::zeroize(impl_->privateKey);
    VirgilByteArrayUtils::zeroize(impl_->pwd);

    impl_->symmetricCipherKey.clear();
    impl_->privateKey.clear();
    impl_->pwd.clear();
}


bool VirgilCipherBase::isInited() const {
    return impl_->isInited;
}


bool VirgilCipherBase::isReadyForEncryption() const {
    return impl_->symmetricCipher.isInited() && impl_->symmetricCipher.isEncryptionMode();
}


bool VirgilCipherBase::isReadyForDecryption() const {
    return impl_->symmetricCipher.isInited() && impl_->symmetricCipher.isDecryptionMode();
}


VirgilSymmetricCipher& VirgilCipherBase::getSymmetricCipher() {
    return impl_->symmetricCipher;
}


VirgilByteArray VirgilCipherBase::doDecryptWithKey(
        const VirgilByteArray& algorithm, const VirgilByteArray& encryptedKey,
        const VirgilByteArray& privateKey, const VirgilByteArray& privateKeyPassword) const {

    VirgilAsymmetricCipher asymmetricCipher;
    asymmetricCipher.setPrivateKey(privateKey, privateKeyPassword);
    return asymmetricCipher.decrypt(encryptedKey);
}


VirgilByteArray VirgilCipherBase::doDecryptWithPassword(
        const VirgilByteArray& encryptedKey, const VirgilByteArray& encryptionAlgorithm,
        const VirgilByteArray& password) const {

    VirgilPBE pbe;
    pbe.fromAsn1(encryptionAlgorithm);
    return pbe.decrypt(encryptedKey, password);
}
